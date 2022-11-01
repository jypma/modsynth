# Ray Wilson Dual VCA

Source: [RayWilson-Dual-VCA](https://github.com/gerb-ster/RayWilson-Dual-VCA/) on Github

The following modifications were made by me:

- Converted schematics and board to Kicad
- Added component values to silk screen
- Added a PCB faceplate in Kicad
- Split up into separate main and io board

## Main board

### Schematic

![schematic](mainboard/export/Schematic/mainboard-schematic.svg)

### PCB

![top](mainboard/export/PCB/2D_render/jlcpcb_green_enig/mainboard-top.jpg)

![bottom](mainboard/export/PCB/2D_render/jlcpcb_green_enig/mainboard-bottom.jpg)

## IO board

### Schematic

![schematic](ioboard/export/Schematic/ioboard-schematic.svg)

### PCB

![top](ioboard/export/PCB/2D_render/jlcpcb_green_enig/ioboard-top.jpg)

![bottom](ioboard/export/PCB/2D_render/jlcpcb_green_enig/ioboard-bottom.jpg)

## Face plate

![faceplate](faceplate/export/PCB/2D_render/jlcpcb_green_enig/faceplate-top.jpg)

# Calibration

See also the original instructions [here](http://musicfromouterspace.com/index.php?MAINTAB=SYNTHDIY&VPW=1910&VPH=716)

TODO: Look up the right R designations here

    Disconnect any inputs or CV from the VCA.
    Set the Log/Linear switch to Linear.
    Connect the black lead of a DVM set to volt range to ground.
    Connect the red lead of a DVM set to volt range to the wiper of R10 (R32 for page 2).
    Adjust R10 (R32 for page 2) until you are as close as possible to 0 volts on the wiper.
    Disconnect the red lead.
    Connect the red lead of a DVM set to millivolt range to the output of the VCA.
    Adjust trimmer R45 (R47 for page 2) so that you have 0 volts on the output of the VCA.
    Disconnect the meter.
    Connect a 100 hz +/-5 volt square wave output from a signal generator to one of the CV inputs of the VCA.
    Observe the output of the VCA with an oscilloscope (adjust the scope as needed).
    The control voltage should appear on the output but at a much lower amplitude.
    Adjust trimmer R18 (R41 for page 2) to minimize the control voltage feedthrough.
    If you have no oscilloscope you can listen to the output and adjust for least CV feedthrough.
