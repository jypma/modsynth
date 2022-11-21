# Design

- Screen
- 1 Encoder
- 4 bipolar CV inputs with Attenuverters (which set a fixed offset when unplugged) (to A5..A8)
- 2 Gate inputs with trigger button
- 2 CV outputs (from MPC4822, SPI)
- 2 Gate outputs (some I/O port)
- 6-pin serial connector, but not exposed.
- MIDI connector, is there space? perhaps two versions?

# UI

- We only have space for 1 encoder.
- Rotate: select variable for current preset
- Click: Adjust variable, click again to confirm.

# Hardware
- Try a dual DIP/SOIC footprint for the ATMega328
- Run ATMega and MCP4822 off 5V, it'll be easier
- We might need a 5V precision reference for the ATMega's ADC (MCP4822 has internal reference)

# Software
- For high-detail modules (envelopes, LFO) try to update the MCP from a timer interrupt, so we can update the screen simultaneously? Perhaps only interrupt-driven WHILE we update the screen.
  - Otherwise, only update the screen after manual input, or on a low derivative...


# Potential modules

## ADSR

- 1 Gate input
- 4 CV inputs (can be virtual) for A,D,S,R
- 1 CV output
- optional: 1 CV inverted output
- optional: 1 extra Gate input, 1-2 extra CV output (second envelope)

Hints [here](https://github.com/baritonomarchetto/Programmable-Envelope-Generator/blob/main/ProgEnvGen_V2.ino)

## AR

- 1 Trigger input
- 2 CV inputs (can be virtual) for A,D
- 1 CV output
- optional: 1 CV inverted output
- optional: 1 extra Gate input, 1-2 extra CV output (second envelope)

## LFO

- 1 CV input (can be virtual) for freq
- optional: 1 CV input for FM
- 1 virtual input: Select wave shape
- 1 CV output

Hints [here](https://github.com/robertgallup/arduino-DualLFO). Need to check quality of the waveform using the MCP4822.

## Pulse

- 1 CV input (can be virtual) for freq
- optional: 1 CV input for FM
- 1 CV input (can be virtual): Duty cycle
- 1 Gate output (or CV)
- optional: extra outputs with dividers

## Clock divider

1 Gate input
N CV input + Gate output pairs, input sets divisor

## Quantizer

Need to check if the incoming ADC is precise enough.

## Sequencer

- encoder + 2 CV input for programming/recording. Modes for recording/setting CH1, CH2 or both
- Gate input advances the sequencer (consider internal clock as well)
- Add more later without adding UI elements

## MIDI In

- 2 CV outputs (note/bend and modulation)
- 1 Gate output
- 1 Clock output (selectable division)
- Selectable MIDI channel (one of the CV inputs)
- Hints [here](https://note.com/solder_state/n/n17e028497eba) (incorporates pitch bend into the CV)
- Original [midi2cv](https://github.com/elkayem/midi2cv)
- Provide an internal connector to chain two modules, having two MIDI channels.

# Simulation

## Output stage (TL074, -12..+12V) [here](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWEBmATNA7AhAOBjVIAWfbEfc8ycgUwFowwAoIyAThADZUiVlOQGTjj4CIMAXTbQ2jMO2Q4wONpBxERMSKmTimAJS44RyYkZGoMqKDdbhr1R9AQHzKYW9aPb1Bg5swLoYIYNbICNZEaCgRNhB21v6BTADuXDwx1iFhsZBMAE5cCRlCFhnUofB5aaWigh6o5anpvGjU3LyNvNUtIF1F1P15hR19GRFl3SBEGPDNRGwC-RNj3UwA5tOLKGYLAsj8Aa574ES8J9vedkjeyQDy9SaHOHB1jgVbSxknYGc2pnNCid+idTFNCHMAPbgZBFWw4ThLJziFCo6iw5BMIA)
Try with 1% resistor and digital calibration, since it's far from the rails.


## Input stage (alt.2, MCP6004 protected, 0..5V)
- Not very useful, since pot controls offset, and needs to be in the middle for a bidirectional signal to make it through. Or is it? CV in would almost always be a
- Does the pot react the wrong way?
[here](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWEBmAHAJmgdgGzoRmACzICcpkORICkNNdCApgLRhgBQRkpIe1yZDhC5UKIeCjRhdGJHTIw8ZSvkh0HAEp9UY5JGo5d6rOijnu4M7KkItOvUYchuNupbbXzMO9oRgzZHR3IJRg8whLajdoOwB5ESdBYVQ4cRkoDgAnPnQBcP9A8LowVPhsvkt0PMSxaujwFQ4Ad1r1GtF0zNaqOnrc6n7IFoH2wxqg2RH+cBw6GaGOMCxqIiIxdjM1jdIzCFp4KXID1WUITSYAZwBLS4AXAEMAOwBjJntt8ACXdZBU7xcjABPgqCF2s3cVAhESaOTBZn6RFIwiGjWUH2RYxcmPwGXcdBYSBidgA5qNkAgzDMiGA8SN4ViFjVhq0GbT5jV2Zkcp9gmJeaYYeiyQKtrSTF5hgB7FyVCyoHDCJRSCB0ZAoczq5AcIA)
Build with 0.1% 100k and 180k resistors. Digital calibration after that.


## Input stage v2 (MCP6004, more accurate offset)
- Offset lands nicely in a middle range (maximum offset need not be calibrated)
- Calibrate 0V (offset to zero), and 4V (send precisely 4V on input)
[here](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWEBmAHAJmgdgGzoRmACzICcpkORICkNNdCApgLRhgBQRkpIOqqENzqpk6IXHBRoOKNMjpkYeCtUKQ6DgCUQWBOOT9d+lIbl1u4cXRvQE24+JLi9TquYkg2183YcJcFFRqIlJZNGo6CEsbX3sAeRBRJ0lUd2FzDgAnPgEg6gDw4LkwVBVs3UhqdDzkjTyo1Q4AdySxesE6jMgWyuq8-kEawR7WwY7cwQioXvH0dGo5ho4wLAKwcVXZBA2NBFkIZRV5SCQtJgBnAEsLgBcAQwA7AGMmf132cR3xMo9LJFiMHsORw+w0CxoNXBkXATR031M2xwdDEgM8LABcQ4AHNJkIwLJxkRfjZZmD5os8hSZjkEegsF9dsgzCjkPAKgjPjQPoEosgsOyAB4oKHoHhCUjIDS7ai7AAqAHsAA4AHQuAApIGqFQALAC2AEo1Tc1QAbBXNS63bUAMxtFyYtw4CqE4FkfNOI1gCH5WDA-P9oXQODC7tgcAgKJQcilyA4QA)

## Input stage v3 (offset now fixed, needs -9V)
[here](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWEBmAHAJmgdgGzoRmACzICcpkORICkNNdCApgLRhgBQRkpIOqqENzqpk6IXHBRoOKNMjpkYeCtUKQ6DgCUQWBOOT9d+lIbl1u4cXRvQE24+JLi9TquYkg2183YcJcFFRqIlJZNGo6CEsbX3sAeRBRJ0lUd2FzDgAnPgEg6gDw4LkwVBVs3UhqdDzkjTyo1Q4AdySxesE6jMgWyuq8-kEawR7WwY7cwQioXvH0dGo5ho4wLAKwcVXZBA2NBFkIZRV5SCQtJgBnAEsLgBcAQwA7AGMmf132cR3xMo8Yjxg9hyOH2GgWNBqYMi4DK8He4nQWC+ODoYliFjoLF46L8AHNJkIwLJxkRfjZZqD5os8lSZjlvuBdgytiVaHCAB4oSHoHhCUjIDS7ai7AAqAHsAA4AHQuAApIDKxQALAC2AEoZTcZQAbMXNS63RUAMyNFyYtw4YqE4FkUWQp0EWGgYDCyQCCAopH0Apg8AgqJQcgFyA4QA)

## Input stage v4 (offset now in the right direction, needs 8V)
[here](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWEBmAHAJmgdgGzoRmACzICcpkORICkNNdCApgLRhgBQRkpIOqqENzqpk6IXHBRoOKNMjpkYeCtUKQ6DgCUQWBOOT9d+lIbl1u4cXRvQE24+JLi9TquYkg2183YcIcOjRqIlJZYLkIS1kfGHsAeRBRJ0lUd2FzDgAnPgEUVGoEXHzqOnQsFWzdSGp0POSNPLplSoB3JLFGwQaMyA52itq8-kE6wT72ka7cwQiJmY10aimxqA4wLEKwcQ3ZBG2NAKkW+HlIJC0mAGcASyuAFwBDADsAYyZ-A-ZxffFUGzklhY4189hyOCO6CWNDqi1KGgq8E+4nKP0CKHQAIsWNBHAA5gsiGBZFMiP9MpNIdCVtC+gAPDGjHhCUjIDQHagHAAqAHsAA4AHSuAApIEKeQALAC2AEohXchQAbHmta73cUAMw1VyY936NC+B1+4GKfQJRVkGQtQk6Nn1ASCBRoxTmHB5QnAsmayHO3TOkAgQRQcjZyA4QA)

## -9V Reference
[here](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWcMBMcUHYMGZIA4UA2ATmIxAUgoqoQFMBaMMAKABYwqMEUQ28qAqoXzgQAPwAqAGTbYIkADoBnNlRQqwjMBuUwcYEsWbdieBAjzYV6lSm26GhaJSvME2HnjyF5NqJoqTi748mAeXj5+etAGRiYIZhZWmmoq4gAuADZyWgAmDACOAHYA+nR5AcrYKAJVKGpUSsoYaE0q2NjQnT29fYQgumDQ9ky6+iicGJBgbISNPpb+uijQbA5BMPCQ2ISW2AJgeMzEKMaQhP7NYJuw8Lv7h8dgp+eXMa1TM3MLe3ipzUyOXkFSKZVBAEFhrxrnhoEcEcckYjBnYRtsMRjsBsYoQMARzIQGp0PGdlio2GtMdStGMbLAwFgMKcdpw8ZZiAh3u1lDcYjMmSz5BduHhOdzoHiCVziZ4asQAelsrlQSVygUIasYSohFU1KjeTj9DMTqLJm4GuTlKMdEFnPwsL5sCzDAg2Aq9FU+cEHRgnS69u76dNmC8zUd5JbeWllECVQU1RU9fBkwbabaPjtcJMGiQ5N4PbZrTiGDBOMYsG7LPhJh4MFdAspS-DIBXuGxqwRwth65nOpAc3NiPmxYrY8qQQnwZVmnVdPQmNYPmgiWBsB25PjCP9PUMS-ouWRSPJ3RgOy8G7zbtwSMziCeyOfC7EV5N126ez4d7NARP8mD1QYCEYCQWci3TRxjQaF4aiIORsyXItsTpfl4CsGZIEoZ1zE5cwEEvJh6W2dDOCwpJcIsa8Bw4e8UDg-sUCXEQlWBf9CgAB3VKo500fdYjgKxTDMQhVw4K1kIzZwsAINg2BwMUEGZMTPWaQjlCk-EGjkg5OSU71jVCISfFEvlmPHVjVU4pMwJ5XVmn1PcUP0TgXh4U4UDdfAcCtG1HAwFxcA3QkdKOfCVMbBh-I8SAgq5ELzGDFzOTOOjPLwbzo1-Cypy465KG4g0FwzA9PA4TwzkIWZnULNNtG9OEPNk4hIHbHRvDky96pGN13Rat02rwDqPgiMq3MquRSDHONJwAyFQM0bZU0cjMmFgOjGVkjBwm8HQBx8nFVncr4BLXDgbUQr00SOlq0NOnRtCXVaB0mM85O2440CGGNprY6cDR+0E8lyha4H+v9Aa45a+SiwLcBIt1iDxfamH0ltjESH8tJiyZuRYAAlCg-RAeZaCJvEYSgPgqBR6BiEppoXBYAAnYnRCJNhie8YmGkpzg4BYAB3QmBhJ4XuY5yBBbF9nWbwcWoCl-EOcUgYlflyWWdaWgdBALX1fAPn+aF7c5ZlvWZcloXzZ5tWLcVngKB1vWEB1jXdbQR3eD12S5fUaZ4EVj2fd1wa+H4BWAGMQB8Dng5j-XhmpJPaXIYg7gEGovma2pyC2GZFdDmX47toX4+DtXg8lgBzaP5jDuXvCoSuFYAeVrjmi7qC2FYAe3AbBwAGKg10wuW05OzweAHAdvCHu4Zkpjm1xAAfc91leWCAA)

## -8V Reference
[here](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWcMBMcUHYMGZIA4UA2ATmIxAUgoqoQFMBaMMAKABYwqMEUQ28qAqoXzgQAPwAqAGTbYIkADoBnNlRQqwjMBuUwcafHOJhCeDDoQr1KlNt0ME0BGwyQOxbNjZm5ha1CaKo7Oru5gnt6+2P560AZweMam5paaairiAC4ANnJaACYMAI4AdgD6dAWBytgoAjUoalRKyhiGNV7QXj29fYQgumDQdky6+iicxJBEHlOQsTbKKNBs9sEw8JAxhDpEWD5smAiLNWAbsPA7e4QHhEfci-GTkNOzEfOxYBnK2XnyVRKFUBAEFhrxWmA8NAobC8HC4YNbCMtqjUdh1nFbgRPHCvJMMMQAro2Ks0eStGNrLAwBhCJg8MQEGBmWRiPsApDqZBafSMIzmazCRysfyULj4Xh8bSicpvq0-vlAWVKkUQSsISohDU1Ei5Zj9DyEPCLGw5GbvhhibZMSEeIR5PCdMRduKrXozsEnPbHcxxa6yNS3CyTd9zRxXOkFbklUUVVUdfBE3rKTog9tXN4XAhsDhsCdrctbTBOHTTOyzUdvrtOUFlAwS5AyyQmmbJj5znE3N4cGxs7mvAW5T9FQC48Dqq0Grp6ExsEG0Pc-e57ngknhC6mHPoW6Y0HVc+yiLW5Rc6ey99sUIeUMeu4uODoV-x11HMjGx0DVQwQTAkFOli3M8ZmNfhsGIeEEHaF1CwxKk4i2KUXG8eUBH5RlOxaYJMMuRJ83NVCmzXExgJQUCpQglloNiER33+QoSgAB1VGpp00Yt4h5SZiDkSAEBOHMYlgzEnAEHMZgWDgvAscYaiYFRRL43AHzAaS03rQ0dAiXj+MIQSaOjejlWYhMAKw5RtVaXUhgNeIdDYF0+Mo0hKFOXRRnUhhCGcKDSDwfjpjXM1Ti5etvP4wkIIC-Akh8IN7MchBnKZBY31+D8GPjSdNEoVi9VndSd2vfM4FcY0cyZTdtE7BgMBGfyHI7PTpgWWTQtq+qXB43Zmrgekg3pQdSu4fzwKsYdDNjL9QX-TQtmTGz4KYGlXjwel3DpRrMELDz3OgfzSCzZhw22bR5w9UKVgOzw+2O7w1Dg1TsJWiD1tce4eO2ia6KmrK9VHBiChYyEk10AHASBhNFuYFRwt89o3XuXy2B26qLlLJKKL0lxbhMDc9BYAAlCg6RAe5aFJ24ISgPgqCYYZiBplpnBYAAnMnRHpNgybXMmmhpzg4BYAB3EmBnJsW+e5yARclrmObwKWoFl-luaggZVaVmX2faWgdBAXWteQSghdFsxFflw35Zl0Wrf5zXrZVngKH1w3mQhNmDbQF3eENs1FfUQxTa9qh-YNpI+H4ZWAGMQDW7mw-jo3hnJVPKXIaENpxelVP4EQFEuVhbYj+Wk8d0Wk7DzWw5lgBzOP7kjxW11DqOWhYAB5BvudLhpreVgB7cBsHAAYqFUvjm9gVxs8mfMkagXCFD4YeQBH8guDXlggA)
