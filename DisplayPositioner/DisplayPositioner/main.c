//
//  main.c
//  DisplayPositioner
//
//  Created by Lucas Vickers on 5/5/15.
//  Copyright (c) 2015 Lucas Vickers. All rights reserved.
//



// fb-rotate.c
//
// Compile with:
// gcc -w -o fb-rotate fb-rotate.c -framework IOKit -framework ApplicationServices

#include <stdio.h>
#include <getopt.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <ApplicationServices/ApplicationServices.h>

#define PROGNAME "DisplayPositioner"
#define MAX_DISPLAYS 16

// kIOFBSetTransform comes from <IOKit/graphics/IOGraphicsTypesPrivate.h>
// in the source for the IOGraphics family


enum {
    kIOFBSetTransform = 0x00000400,
};

void
usage(void)
{
    fprintf(stderr,
            "usage: %s -l\n"
            "       %s -r\n",
            PROGNAME, PROGNAME);
    exit(1);
}

void
listDisplays(void)
{
    CGDisplayErr      dErr;
    CGDisplayCount    displayCount, i;
    CGDirectDisplayID mainDisplay;
    CGDisplayCount    maxDisplays = MAX_DISPLAYS;
    CGDirectDisplayID onlineDisplays[MAX_DISPLAYS];
    
    CGEventRef ourEvent = CGEventCreate(NULL);
    CGPoint ourLoc = CGEventGetLocation(ourEvent);
    
    CFRelease(ourEvent);
    
    mainDisplay = CGMainDisplayID();
    
    dErr = CGGetOnlineDisplayList(maxDisplays, onlineDisplays, &displayCount);
    if (dErr != kCGErrorSuccess) {
        fprintf(stderr, "CGGetOnlineDisplayList: error %d.\n", dErr);
        exit(1);
    }
    
    printf("#  Display_ID   Display_ID   Resolution   Display_Origin   _____Display_Bounds_____    Rotation   Details\n");
    for (i = 0; i < displayCount; i++) {
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
    
    printf("Mouse Cursor Position:  ( %5.0f , %5.0f )\n",
           (float)ourLoc.x, (float)ourLoc.y);
    
    exit(0);
}

void runChange(void)
{
    int numMonitors = 2;
    int monitors[2][3] = {
        { 441005125, 0, 0 },
        { 441005124, -1920, 0}
    };

    uint32_t displayCount;
    CGGetActiveDisplayList(0, NULL, &displayCount);
    
    CGDirectDisplayID activeDisplays[displayCount];
    CGGetActiveDisplayList(displayCount, activeDisplays, &displayCount);
    
    for( int i=0; i<displayCount; ++i ) {
        
        printf("%d", activeDisplays[i]);
        
        bool found = false;
        for( int j=0; j<numMonitors; ++j ) {
  
            if( activeDisplays[i] == monitors[j][0] ) {
                
                printf(" - setting display to origin \t%d \t%d\n",
                       monitors[j][1], monitors[j][2]);
                
                // magic
                CGDisplayConfigRef config;
                CGBeginDisplayConfiguration(&config);
                CGConfigureDisplayOrigin( config, activeDisplays[i], monitors[j][1], monitors[j][2] );
                CGCompleteDisplayConfiguration(config, kCGConfigurePermanently);
                
                found = true;
            }
        }
        if( ! found ) {
            printf(" - skipping\n");
        }
    }
    
    exit(0);
}


CGDirectDisplayID
cgIDfromU32(uint32_t preId)
{
    CGDisplayErr      dErr;
    CGDisplayCount    displayCount, i;
    CGDisplayCount    maxDisplays = MAX_DISPLAYS;
    CGDirectDisplayID onlineDisplays[MAX_DISPLAYS];
    CGDirectDisplayID postId = preId;
    
    dErr = CGGetOnlineDisplayList(maxDisplays, onlineDisplays, &displayCount);
    if (dErr != kCGErrorSuccess) {
        fprintf(stderr, "CGGetOnlineDisplayList: error %d.\n", dErr);
        exit(1);
    }
    for (i = 0; i < displayCount; i++) {
        CGDirectDisplayID dID = onlineDisplays[i];
        if ((dID == preId) || (dID == postId) ||
            (onlineDisplays[i] == preId) || (onlineDisplays[i] == postId)) {
            return dID;
        }
    }
    fprintf(stderr, " Could not find a matching id in onlineDisplays!\n");
    exit(1);
}


int
main(int argc, char **argv)
{
    int  i;
    
    while ((i = getopt(argc, argv, ":lr")) != -1) {
        switch (i) {
            case 'l':
                listDisplays();
                break;
            case 'r':
                runChange();
                break;
            default:
                break;
        }
    }
    
    usage();
    
    exit(0);
}