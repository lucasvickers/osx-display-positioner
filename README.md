# osx-display-positioner
Command line tool for adjusting display positions in OS X.

### Componenets
#### displaypositioner
#### displaymonitor


### Motivation
Working on an install that involves 3 mac pros each driving 6 screens.  We noticed that the machine periodically loses the screen position settings.  The displaypositioner app was written to identify the issue and fix the screen positions if needed.

In our specific example, it appears the low level bug prevented the errant screen from being placed in the correct location.  We were somewhat lucky enough that a system restart seemed to fix the screen positions again.  After this finding, the displaywatcher python tool was created.  This tool uses the displaypositioner app to see if the monitors have lost position.  If they are, it will restart the machine up to 3 times and send out email notifications alerting the admins of the issue.


### displaypositioner

#### Overview
The need for this project originated out of an install where the mac pro kept forgetting and remembering a specified display layout.
This tool allows you to save the current display settings to config file, then later compare the display settings to that config file, and if you like apply changes to restore your old display layout.

#### Compiling:
Written as an easy to compile C program, but xcode project included anyway.
To compile via gcc:
```
gcc -w -o displaypositioner main.c -framework IOKit -framework ApplicationServices
```

#### Usage:
The settings are stored in:
 ```~/Library/Application\ Support/display.positioner.config```
```
usage: DisplayPositioner -l (list display information)
       DisplayPositioner -a (apply settings from file)
       DisplayPositioner -c (compare current configuration to config file, apply changes if needed)
       DisplayPositioner -t (test current configruations to config file, make no changes)
       DisplayPositioner -p (programatic test, return str of "true" if displays match config or "false" otherwise)
       DisplayPositioner -s (save current configuration to config file)
```

#####-l : List current display settings
```
#  Display_ID   Display_ID   Resolution   Display_Origin   _____Display_Bounds_____    Rotation   Details
5  0x1a493445   441005125    1920x1200        0     0          0     0  1920  1200     0          [main]
4  0x1a493444   441005124    1920x1200    -1920     0      -1920     0     0  1200     0          
```
#####-a : Apply the display settings from file to the current displays if the DisplayIDs match.
```
441005125 - setting display to origin 	0 	0
441005124 - setting display to origin 	-1920 	0
``` 
#####-c : Compare the display settings to the config file.  If DisplayIDs match but origins do not, update display settings.
Note: it is intelligent when it decides to update display settings, but it will update all displays.
```
Monitor values differ, updating.
441005125 - setting display to origin 	0 	0
441005124 - setting display to origin 	-1920 	0
```
#####-t : Identical to Compare, but won't actually make changes.
```
Monitor values differ, would update.
Display_ID  Config_Origin  Display_Origin
441005124   -1920     0    -1920  -579
```
#####-p : Identical to test, but returns a simple true or false (string) if the monitors match the config settings
```
true
```
#####-s : Saves current display settings to the config file.
```
Stored 2 display settings to config.
```


### displaywatcher
This is a python tool that will query the displaypositioner to see if the display is out of sync.  If it is, it will retempt a reboot upto X times.  It can notify via email on restarts/errors, etc.

More documentation pending, but a short description
- enable local root ssh login (used for restart command)
- copy config.yaml.example to config.yaml
- update the config.yaml file, that means the following fields
  - # main binary path (set it to the path of the binary)
  - # location of the reboot file (set where you want to log)
  - all email fields

The delay is to give the OS enough time to initialize the displays.
Be sure to save the your display configurations using "displaypositioner -s"
