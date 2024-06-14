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
    Connect the red lead of a DVM set to volt range to the wiper of BIAS control on panel.
    Adjust BIAS until you are as close as possible to 0 volts on the wiper.
    Disconnect the red lead.
    Connect the red lead of a DVM set to millivolt range to the output of the VCA.
    Adjust trimmer R1/R2 (100k) so that you have 0 volts on the output of the VCA.
    Disconnect the meter.
    Connect a 100 hz +/-5 volt square wave output from a signal generator to one of the CV inputs of the VCA.
    Observe the output of the VCA with an oscilloscope (adjust the scope as needed).
    The control voltage should appear on the output but at a much lower amplitude.
    Adjust trimmer R3/R4 (2k) to minimize the control voltage feedthrough.
       -> There's a balance here: If you do minimize the feedthrough, the VCA's max output level will be significantly attenuated (with a +5V signal). I leave it such that I still have ~80% of incoming oscillator signals left.
    If you have no oscilloscope you can listen to the output and adjust for least CV feedthrough.

# TODO

- Values are missing on I/O and main board
- IC value can't be read on main board
- No mounting holes in main and I/O board (there's space in the bottom for manual drilling)
- Holes for switches on I/O board aren't big enough. Drill is possible, but use lots of solder for channel2, since traces connect at the top (and drilling ruins the through-hole connection).
- SMD footprints are missing pin 1 circle

# Simulation

https://www.falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWcMBMcUHYMGZIA4UA2ATmIxAUgoqoQFMBaMMAKACUQU89xiVOUAFl78qENJ2hIUUqFCktBkYiAwJ+2dSDyCem-hBiE5MSCmxh4V62c4sA7qq3ZsxjBJfHI7bXBDYJPD8uHioqJXBREwUOIKpsHV9wsC85CKYosIUAJyd+FARjHR4C1LQ4BzzOQqrPKEri6qLdfy1vXLV+QUJhPACQbuEqcvaqwZBCdwGeuUsrH0mumcWB-rCBsUz5BBYAc21+8b6u4lTvR07-Qh5L8e8OSwiUIWQnmvX0sC2YHYfIGsulgBEg+mxM20UyhApAMfGQ3BE4HkxiYP0gYE0CAQGLwCAwfSQpnMhhYuRh4AwzR4YEpchGDRapW0LRpXlJ0LhzAM+GpX1m1kq5K5HIMtPO8OpnJ54D54vJaB45NwQz2yCmyrVGlcJh8xD5Gr1+SmoMi4J+7KVkGEhv8Vv58zJcI1lnVdvCGHFlgRYE5KBSiNGX39PoMfuMITkuHg7KD4e9Yf8iWG8weCYSwljA39JoyZoUSm5L0INWY2uLxkMAB0AM6QGtKGsMYg1uu1luyVuthvV5ttvs-KA1sCNjDQQRkbCCbFW5SQDDdduUSnMQgoUgF1eCIdwGsAEzoADMAIYAVwANgAXQfV4fVhho5cpNfEDdCRdzzTXSDF7iFUjb1t92Pc8r1bW9OykD9CiCH9cRIXs5j3Q9T0va9bwYPBYCEPA9TIQpNHRcCxzwSk8B6BBiASK1nitACkOA1CwMbTD0UEfFXDxMAKMIDF23HIEKO-a5sDUXE6OrICUNAxsiN0UiwEEDEvkIXAFMbGA5MEIgeK+BJrhwQhxMkkDryYFt1OI+dtK5PSSNcIzkJM1szJvQhYHHDAXzgMAdC4Yg8AsuTrgUpSiFUrcbx3CTHMYmS+L1f58DxfA+BXQLiDnU5IHTNi+gIQzIsAmLpLvcD0sywhsp0XKEiIByGJKly3JIsw8BpXQLFwMiEHSoEko9HC-RSeqpLQ8za0srSuKCIRThwHrCvo0amLvDSSMmFTKIQLTBEU+c+JIqaEBm8dJk0EanKHRtiGgGlMBwWrNCwXQDspSlsC2na9oixDooasb+1gdEehXdFSACLgKJQC7Ypvcb7yBu6CBIx50UxOrFr+qTKi5WhdplCQI09L5hnjf0iZYESxBJih8dxiYXkMeoOAMhnhHLNnZltWg8x2DoyxeY5ObEAV2oOfhN3F1QJHEeBthANg6GrABLasLyPAA7ABjOh2SF9wqCFtrUi47x9iN-0jZaMIGR4FZGkl8UHcF-pHZxsx2cFlo3f2Swi29D3pfWYmyxLDE3DaGNw6qIEI6iNiU1eMRKqTyJQjSMFg5yGVtUBcx+Eud1o19-OY9L-EhnqXJSzjnOqTpGwWDF2PmWpOd+BtCA5jlgdFZVtXNZ1nwW8aEfswz00s9+ZAak7-5jBfSvwioXMp-dqZuGp7VN-qRwa+0II69b+olE2ZYSwCO2SwB1FoFxP0uB6BJKHD98PZ5EIfUECwYZK9DmGgPoVwxB1BnQVO2HipxLCkGOFgdQv9wA4yIOzMOoc2QPEvhMCIGIuATHwOCbB3wFB70wSsBS+CViBiUHbKY5CbhtAGB6e44BqEx1YRXAhK9ZTRD5iwihtDKDs2NMgCoAB5cAgiJgCLLMI7w4jGhMh3kyG2Ys6gKWEHUIQYgaA9yBkgAAwgANUqPoREEYQz1H2JoiImjZHskUhWfGDj-DahFvMRwainHBnxqMZxmiER1DccXTgATtQRlcOncUfjtTOLuPY-04wmR3BEdGMWWiszhgiNiAwKT5BoGwCAAAQsrI81YfDpOcek9MnDJ48PKVkvk6TtpLw2CAVedSLgeBif6QJLBxF+09hmQOeIzh9MiMGX0wYU42wAPZyBmAUr4a5hhSA+t-Y6kM4CVRUKYbydJVDkG8HMlwWDZjPBwvIeAhggasREh6VwXogx+iRFqfwLA5k8VOcvfy1AYDiEQXM2kMxNjLMuclOcc4PTmBIJ5LwNyICLJYEAA
