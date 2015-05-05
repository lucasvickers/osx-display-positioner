//
//  main.c
//  displaypositioner
//
//  Created by Lucas Vickers on 5/5/15.
//  Copyright (c) 2015 Lucas Vickers. All rights reserved.
//

#include <stdio.h>
#include <getopt.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreServices/CoreServices.h>

#define PROGNAME "DisplayPositioner"
#define MAX_DISPLAYS 16

// make my life easier
uint32_t SYSTEM_DISPLAY_VALUES[MAX_DISPLAYS][3];
uint32_t STORED_SYSTEM_DISPLAY_VALUES;

uint32_t CONFIG_DISPLAY_VALUES[MAX_DISPLAYS][3];
uint32_t STORED_CONFIG_DISPLAY_VALUES;

enum {
    kIOFBSetTransform = 0x00000400,
};


////// HELPER FUNCTIONS

void
getConfigPath( char* str, uint strSize )
{
    FSRef ref;
    OSType folderType = kApplicationSupportFolderType;
    
    FSFindFolder( kUserDomain, folderType, kCreateFolder, &ref );
    FSRefMakePath( &ref, (UInt8*)str, strSize );
}


FILE*
getConfigFP( const char* mode )
{
    char datafolder[300];
    getConfigPath( datafolder, sizeof( datafolder ) );
    printf( "%s\n", datafolder );
    
    char path[350];
    snprintf( path, sizeof( path ), "%s/display.positioner.config", datafolder );
    
    FILE *fp;
    fp = fopen( path, "w" );
    if( fp == NULL ) {
        printf( "Unable to open config file in mode %s\n", mode );
        printf( "File was: %s\n", path );
        exit( 0 );
    }
    return fp;
}

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
        
        DISPLAY_VALUES[i][0] = onlineDisplays[i];
        DISPLAY_VALUES[i][1] = CGRectGetMinX ( CGDisplayBounds ( onlineDisplays[i] ) );
        DISPLAY_VALUES[i][2] = CGRectGetMinY ( CGDisplayBounds ( onlineDisplays[i] ) );
    }
    STORED_VALUES = displayCount;
}

void
loadConfigSettings()
{
    
}

void
saveConfigSettings()
{
    
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
    
    // TODO test if I can remove this event stuff
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
    
    // if we have a logic match
    //   apply
    
    uint32_t displayCount;
    CGGetActiveDisplayList( 0, NULL, &displayCount );
    
    CGDirectDisplayID activeDisplays[displayCount];
    CGGetActiveDisplayList( displayCount, activeDisplays, &displayCount );
    
    
    if( displayCount != STORED_VALUES ) {
        fprintf( stderr, "There are %d active monitors and %d stored values, "
                         "they do not match, exiting.\n",
                         displayCount, STORED_VALUES );
        exit( 1 );
    }
    
    int matchCount = 0;
    for( int i=0; i<displayCount; ++i ) {
        for( int j=0; j<STORED_VALUES; ++j ) {
            if( activeDisplays[i] == DISPLAY_VALUES[j][0] ) {
                ++matchCount;
                break;
            }
        }
    }
    
    if( matchCount != displayCount ) {
        fprintf( stderr, "Couldn't match all stored IDs to active IDs, exiting.\n" );
        exit( 1 );
    }
    
    for( int i=0; i<displayCount; ++i ) {

        for( int j=0; j<STORED_VALUES; ++j ) {
            
            if( activeDisplays[i] == DISPLAY_VALUES[j][0] ) {
                
                printf( "%d - setting display to origin \t%d \t%d\n",
                        DISPLAY_VALUES[j][0], DISPLAY_VALUES[j][1], DISPLAY_VALUES[j][2] );
                
                // magic
                CGDisplayConfigRef config;
                CGBeginDisplayConfiguration( &config );
                CGConfigureDisplayOrigin( config, activeDisplays[i], DISPLAY_VALUES[j][1], DISPLAY_VALUES[j][2] );
                CGCompleteDisplayConfiguration( config, kCGConfigurePermanently );
            }
        }
    }
    
    exit( 0 );
}


// compare config file settings to monitor settings, update if needed
void
compareUpdatePositions()
{
    // load from the config file and save it in temp
    loadConfigSettings();
    
    // pull our current settings
    pullDisplaySettings();
    
    // compare and update all if needed
    // if compare shows a need
    //  and we have a logic match
    //   apply
    
    
    exit( 0 );
}

// compare config file settings to monitor settings, do not update
void
testComparePositions()
{
    loadConfigSettings();
    pullDisplaySettings();
    
    // compare and report
    
    exit( 0 );
}

// save current monitor settings to config file
void
savePositions()
{
    pullDisplaySettings();
    // copy display to config values
    saveConfigSettings();
    
    exit( 0 );
}


///// MAIN FUNCTIONS

void
usage( void )
{
    fprintf( stderr,
             "usage: %s -l (list display information)\n"
             "       %s -r (run and apply changes from file)\n"
             "       %s -c (compare current configuration to config file, apply changes if needed)\n"
             "       %s -t (test current configruations to config file, make no changes)\n"
             "       %s -s (save current configuration to config file)\n",
             PROGNAME, PROGNAME, PROGNAME, PROGNAME, PROGNAME );
    exit( 1 );
}

int
main( int argc, char **argv )
{
    // don't run if the data is bad somehow
    STORED_VALUES = 0;
    
    int  i;
    while ( ( i = getopt(argc, argv, "lrcst" ) ) != -1 ) {
        switch( i ) {
            case 'l':
                listDisplays();
                break;
            case 'r':
                updatePositions();
                break;
            case 'c':
                compareUpdatePositions();
                break;
            case 't':
                testComparePositions();
                break;
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

