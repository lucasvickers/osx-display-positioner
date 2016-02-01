/*
 Copyright (c) 2015, Lucas Vickers

 This code is intended to be used with the Cinder C++ library, http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

// compile with:
// gcc -w -o displaypositioner main.c -framework IOKit -framework ApplicationServices

#include <stdio.h>
#include <getopt.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreServices/CoreServices.h>

#define PROGNAME "DisplayPositioner"
#define MAX_DISPLAYS 16

// make my life easier
int32_t SYSTEM_VALUES[MAX_DISPLAYS][3];
uint32_t STORED_SYSTEM_VALUES;

int32_t CONFIG_VALUES[MAX_DISPLAYS][3];
uint32_t STORED_CONFIG_VALUES;

enum {
    kIOFBSetTransform = 0x00000400,
};


////// HELPER FUNCTIONS

// gets the Application Support path for current user
void
getConfigPath( char* str, uint strSize )
{
    FSRef ref;
    OSType folderType = kApplicationSupportFolderType;
    
    FSFindFolder( kUserDomain, folderType, kCreateFolder, &ref );
    FSRefMakePath( &ref, (UInt8*)str, strSize );
}


// opens file "~/Application Support/display.positioner.config"
// in a given mode returns the FILE pointer
FILE*
getConfigFP( const char* mode )
{
    char datafolder[300];
    getConfigPath( datafolder, sizeof( datafolder ) );
    
    char path[350];
    snprintf( path, sizeof( path ), "%s/display.positioner.config", datafolder );
    
    FILE *fp;
    fp = fopen( path, mode );
    if( fp == NULL ) {
        printf( "Unable to open config file in mode %s\n", mode );
        printf( "File was: %s\n", path );
        exit( 1 );
    }
    return fp;
}


// reads display values into memory
void
pullDisplaySettings()
{
    CGDisplayErr      dErr;
    CGDisplayCount    displayCount;
    CGDisplayCount    maxDisplays = MAX_DISPLAYS;
    CGDirectDisplayID onlineDisplays[MAX_DISPLAYS];
    
    dErr = CGGetOnlineDisplayList( maxDisplays, onlineDisplays, &displayCount );
    if( dErr ) {
        fprintf( stderr, "Error getting display list." );
        exit( 1 );
    }

    for( int i=0; i<displayCount; ++i ) {
        
        SYSTEM_VALUES[i][0] = onlineDisplays[i];
        SYSTEM_VALUES[i][1] = CGRectGetMinX ( CGDisplayBounds ( onlineDisplays[i] ) );
        SYSTEM_VALUES[i][2] = CGRectGetMinY ( CGDisplayBounds ( onlineDisplays[i] ) );
    }
    STORED_SYSTEM_VALUES = displayCount;
}


// read config value into memory
void
loadConfigSettings()
{
    FILE *fp = getConfigFP( "r" );
    char str[200];
    char *token;
    int line = 0;

    while( fgets( str , sizeof( str ) , fp ) != NULL ) {
        
        if( line > MAX_DISPLAYS ) {
            fprintf( stderr, "Error parsing config file, too many monitors." );
            exit(1);
        }
        
        token = strtok(str, ",");
        int index = 0;
        while( token != NULL ) {
            // a little sanity check, not much
            if( index > 2 ) {
                fprintf( stderr, "Error parsing config file, too many values." );
                exit(1);
            }
            
            CONFIG_VALUES[line][index] = atoi(token);
            
            ++index;
            token = strtok(NULL, ",");
        }
        
        ++line;
    }
    
    STORED_CONFIG_VALUES = line;

    fclose( fp );
}


// writes config values to file
void
saveConfigSettings()
{
    FILE *fp = getConfigFP( "w" );
    for( int i=0; i<STORED_CONFIG_VALUES; ++i ) {
        fprintf( fp, "%d, %d, %d\n",
                 CONFIG_VALUES[i][0], CONFIG_VALUES[i][1], CONFIG_VALUES[i][2] );
    }
    
    printf("Stored %d display settings to config.\n", STORED_CONFIG_VALUES);
    
    fclose( fp );
}


// applies config values to system
// does no checks, just applies settings to matching IDs
void
applyDisplaySettings()
{
    uint32_t displayCount;
    CGGetActiveDisplayList( 0, NULL, &displayCount );
    
    CGDirectDisplayID activeDisplays[displayCount];
    CGGetActiveDisplayList( displayCount, activeDisplays, &displayCount );
    
    for( int i=0; i<displayCount; ++i ) {
        
        for( int j=0; j<STORED_CONFIG_VALUES; ++j ) {
            
            if( activeDisplays[i] == CONFIG_VALUES[j][0] ) {
                
                printf( "%d - setting display to origin \t%d \t%d\n",
                        CONFIG_VALUES[j][0], CONFIG_VALUES[j][1], CONFIG_VALUES[j][2] );
                
                // magic
                CGDisplayConfigRef config;
                CGBeginDisplayConfiguration( &config );
                CGConfigureDisplayOrigin( config, activeDisplays[i], CONFIG_VALUES[j][1], CONFIG_VALUES[j][2] );
                CGCompleteDisplayConfiguration( config, kCGConfigurePermanently );
                
                break;
            }
        }
    }
}


// compares config values and screen values
// returns true if their ids match
bool
compareDisplaySettingIDs()
{
    
    if( STORED_CONFIG_VALUES != STORED_SYSTEM_VALUES ) {
        return false;
    }
    
    int matchCount = 0;
    for( int i=0; i<STORED_CONFIG_VALUES; ++i ) {
        for( int j=0; j<STORED_SYSTEM_VALUES; ++j ) {
            if( CONFIG_VALUES[i][0] == SYSTEM_VALUES[j][0] ) {
                ++matchCount;
                break;
            }
        }
    }
    
    if( matchCount != STORED_SYSTEM_VALUES ) {
        return false;
    }
    
    return true;
}


// compares config values and screen values
// returns true if their ids and values match
bool
compareDisplaySettingValues()
{
    if( ! compareDisplaySettingIDs() ) {
        return false;
    }

    int matchCount = 0;
    for( int i=0; i<STORED_CONFIG_VALUES; ++i ) {
        for( int j=0; j<STORED_SYSTEM_VALUES; ++j ) {
            // could just do a memcmp...
            if( CONFIG_VALUES[i][0] == SYSTEM_VALUES[j][0] &&
                CONFIG_VALUES[i][1] == SYSTEM_VALUES[j][1] &&
                CONFIG_VALUES[i][2] == SYSTEM_VALUES[j][2] )
            {
                ++matchCount;
                break;
            }
        }
    }
    
    return matchCount == STORED_SYSTEM_VALUES;
}


/////// END POINT FUNCTIONS

// list all display information
void
listDisplays( void )
{
    
    CGDisplayErr      dErr;
    CGDisplayCount    displayCount, i;
    CGDirectDisplayID mainDisplay;
    CGDisplayCount    maxDisplays = MAX_DISPLAYS;
    CGDirectDisplayID onlineDisplays[MAX_DISPLAYS];
    
    CGEventRef ourEvent = CGEventCreate( NULL );
    CGPoint ourLoc = CGEventGetLocation( ourEvent );
    CFRelease( ourEvent );
    
    mainDisplay = CGMainDisplayID();
    
    dErr = CGGetOnlineDisplayList( maxDisplays, onlineDisplays, &displayCount );
    if( dErr != kCGErrorSuccess ) {
        fprintf( stderr, "CGGetOnlineDisplayList: error %d.\n", dErr );
        exit( 1 );
    }
    
    printf("#  Display_ID   Display_ID   Resolution   Display_Origin   _____Display_Bounds_____    Rotation   Details\n");
    for ( i=0; i<displayCount; i++ ) {
        CGDirectDisplayID dID = onlineDisplays[i];
        printf("%-2d 0x%-10x %d    %4lux%-4lu    %5.0f %5.0f      %5.0f %5.0f %5.0f %5.0f   %3.0f          %s%s%s",
               // id
               CGDisplayUnitNumber (dID),
               dID,
               dID,
               // res
               CGDisplayPixelsWide(dID), CGDisplayPixelsHigh(dID),
               // origin
               CGRectGetMinX (CGDisplayBounds (dID)),
               CGRectGetMinY (CGDisplayBounds (dID)),
               // bounds (related to origin)
               CGRectGetMinX (CGDisplayBounds (dID)),
               CGRectGetMinY (CGDisplayBounds (dID)),
               CGRectGetMaxX (CGDisplayBounds (dID)),
               CGRectGetMaxY (CGDisplayBounds (dID)),
               // rotation
               CGDisplayRotation (dID),
               
               (CGDisplayIsActive (dID)) ? "" : "[inactive]",
               (dID == mainDisplay) ? "[main]" : "",
               (CGDisplayIsBuiltin (dID)) ? "[internal]\n" : "\n");
    }
    
    printf( "Mouse Cursor Position:  ( %5.0f , %5.0f )\n",
            (float)ourLoc.x, (float)ourLoc.y );
    
    exit( 0 );
}


// update monitor positions to match config file positions
void
updatePositions( void )
{
    // load from the config file and save it in temp
    loadConfigSettings();
    
    // pull our current settings
    pullDisplaySettings();
    
    if( ! compareDisplaySettingIDs() ) {
        fprintf( stderr, "Config file has %d monitors, system has %d, can't continue.\n",
                 STORED_CONFIG_VALUES, STORED_SYSTEM_VALUES );
        exit( 1 );
    }
    
    applyDisplaySettings();
    printf( "Display settings have been applied.\n" );
    
    exit( 0 );
}


// compare config file settings to monitor settings, update if needed
void
compareUpdatePositions()
{

    loadConfigSettings();
    pullDisplaySettings();
    
    if( ! compareDisplaySettingIDs() ) {
        fprintf( stderr, "Config file has %d monitors, system has %d, not updating.\n",
                STORED_CONFIG_VALUES, STORED_SYSTEM_VALUES );
        exit( 0 );
    }
    
    if( compareDisplaySettingValues() ) {
        printf( "Monitor values match, not updating.\n" );
        exit( 0 );
    }
    
    printf( "Monitor values differ, updating.\n" );
    applyDisplaySettings();
    
    exit( 0 );
}


// compare config file settings to monitor settings, do not update
void
testComparePositions(bool humanReadable)
{
    loadConfigSettings();
    pullDisplaySettings();
    
    if( ! compareDisplaySettingIDs() ) {
        if( humanReadable ) {
            printf( "Config file has %d monitors, system has %d, would not update.\n",
                    STORED_CONFIG_VALUES, STORED_SYSTEM_VALUES );
        } else {
            printf( "false" );
        }
        exit( 0 );
    }
    
    if( compareDisplaySettingValues() ) {
        if( humanReadable ) {
            printf( "Monitor values match, would not update.\n" );
        } else {
            printf( "true" );
        }
        exit( 0 );
    }
    
    if( humanReadable ) {
        printf( "Monitor values differ, would update.\n" );
        
        printf( "Display_ID  Config_Origin  Display_Origin\n");
        
        for( int i=0; i<STORED_CONFIG_VALUES; ++i ) {
            for( int j=0; j<STORED_SYSTEM_VALUES; ++j ) {
                // could just do a memcmp...
                if( CONFIG_VALUES[i][0] == SYSTEM_VALUES[j][0] ) {
                    
                    if( CONFIG_VALUES[i][1] != SYSTEM_VALUES[j][1]
                        || CONFIG_VALUES[i][2] != SYSTEM_VALUES[j][2] )
                    {
                        printf( "%d   %5d %5d    %5d %5d\n",
                               CONFIG_VALUES[i][0], CONFIG_VALUES[i][1], CONFIG_VALUES[i][2],
                               SYSTEM_VALUES[i][1], SYSTEM_VALUES[i][2] );
                    }

                    break;
                }
            }
        }
    } else {
        printf( "false" );
    }
    
    exit( 0 );
}


// save current monitor settings to config file
void
savePositions()
{
    pullDisplaySettings();
    
    STORED_CONFIG_VALUES = STORED_SYSTEM_VALUES;
    memcpy( CONFIG_VALUES, SYSTEM_VALUES, sizeof( CONFIG_VALUES ) );
    
    saveConfigSettings();
    
    exit( 0 );
}


///// MAIN FUNCTIONS

void
usage( void )
{
    fprintf( stderr,
             "usage: %s -l (list display information)\n"
             "       %s -a (apply settings from file)\n"
             "       %s -c (compare current configuration to config file, apply changes if needed)\n"
             "       %s -t (test current configruations to config file, make no changes)\n"
             "       %s -p (programatic test, return str of \"true\" if displays match config or \"false\" otherwise)\n"
             "       %s -s (save current configuration to config file)\n",
             PROGNAME, PROGNAME, PROGNAME, PROGNAME, PROGNAME, PROGNAME );
    exit( 1 );
}

int
main( int argc, char **argv )
{
    // don't run if the data is bad somehow
    STORED_CONFIG_VALUES = 0;
    STORED_SYSTEM_VALUES = 0;
    
    int  i;
    while ( ( i = getopt(argc, argv, "lactsp" ) ) != -1 ) {
        switch( i ) {
            case 'l':
                listDisplays();
                break;
            case 'a':
                updatePositions();
                break;
            case 'c':
                compareUpdatePositions();
                break;
            case 't':
                testComparePositions(true);
                break;
            case 'p':
                testComparePositions(false);
            case 's':
                savePositions();
                break;
            default:
                break;
        }
    }
    
    usage();
    
    exit( 0 );
}

