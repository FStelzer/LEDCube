# LEDCube project - like all the other LED Cubes but using PCBs and therefore much simpler to solder.

Kit contains 4 different parts:
 - Mainboard (largest one) -> goes on the bottom, components facing down
 - 4x Columns - go vertical up from the mainboard
 - 8x Rows A - 4x LEDs on each - connect to column boards
 - 8x Rows B - 4x LEDs on each - connecto to column boards

See images/ folder first to see what the result should look like and sort your parts and try to fit them together without solder first!

## Soldering

Check if all column boards fit through the slots in the mainboard. Sometimes they need a little bit of scraping/sanding on the edges to fit easily.
Similar for the rows connecting to the columns, check if they all fit. Usually those can simply be pushed in.

Then solder 4 identical rows to each column board. The columns have an arrow printed on it. This is lower end next to the mainboard. Make sure all the LED on the rows point upwards when soldering them to the columns.
When adding a row to a column add a single solder joint first, then heat up and adjust so everything is straight. Only then add all the remaining joints.
Note that every row has up to 8 joints to the column boards (top and bottom side). Some rows only allow for 6. This is expected and due to routing issues.
Simply solder every possible connection and leave the ones without corresponding pads.
This should give you four grids with two different versions. One Version slightly shifted to the right and one to the left.

Same procedure then soldering the columns to the main board. Start with the inner 2 columns.
Like the rows, again only one joint first, then reheat & align, then the other joints.
The column boards at the bottom have 4 Joints on Top and also 4 on the Bottom. Start with the ones on top for easier alignment.
The rows have a small white arrow printed on them. This arrows needs to point towards the printed white line on the mainboard (see pictures).
Make sure to have each column the same distance through the mainboard or the end result will not be level.

## Software setup
 - Install VSCode
 - Clone this repo and open the folder with vscode (or directly clone it with vscode)
 - VSCode will prompt you to install the PlatformIO Extension. Do so and also then install the required board support and libraries (platformio should handle this). You'll need the ststm32 platform, the official stm32duino core and the CircularBuffer library but everything should be installed automatically.
 - You can then build & upload new code while the board is simply connected via usb (it will provide a serial port which can also be used for debug output)

 In case you brick the board (invalid code, exception, crash) the upload utility will wait a moment for the board to appear. You can then press the "Reset" button, the bootloader will provide the serial interface and the upload should work. If you take too long it will time out. Simply try again, pressing the reset faster when upload is waiting for the port.

 ## Adding animations
Adding a new animation only needs a new function defined in include/animations.h and then added to the "animationList" array initializer. The order of this array also controls how the animations are cycled through with the mode button.
animations.cpp has a few sample animations you can copy & modify. The most important functions you wanna use are:
Cube->setVoxel(x, y, z, intensity); (x,y,z: 0-3; intensity: 0-4095)
Cube->update(microseconds); Updates the actualy drawing buffer with the new values and then waits the specified time in microseconds
Cube->drawLine/drawCube();
Cube->setAll(intensity);

or the Voxel class which supports move & moveTo functions to move a single Voxel with relative coordinates around the cube.
The animateBeats() function shows how to access the built in microphone.

The animation function will basically be called in and endless loop. But has to finish one call before the cube can react to a press on the "mode" button so there are two ways to improve responsiveness of the button.
1. Use static variables and only animate a short portion of your whole animation within one call (pulseInnerOuter does it this way)
2. Use the "YIELD" Macro (defined in animations.h) within your code to provide a quicker exit in case the button was pressed.