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

Output stage (TL074, -12..+12V) [here](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWEBmATNA7AhAOBjVIAWfbEfc8ycgUwFowwAoIyAThADZUiVlOQGTjj4CIMAXTbQ2jMO2Q4wONpBxERMSKmTimAJS44RyYkZGoMqKDdbhr1R9AQHzKYW9aPb1Bg5swLoYIYNbICNZEaCgRNhB21v6BTADuXDwx1iFhsZBMAE5cCRlCFhnUofB5aaWigh6o5anpvGjU3LyNvNUtIF1F1P15hR19GRFl3SBEGPDNRGwC-RNj3UwA5tOLKGYLAsj8Aa574ES8J9vedkjeyQDy9SaHOHB1jgVbSxknYGc2pnNCid+idTFNCHMAPbgZBFWw4ThLJziFCo6iw5BMIA)
Try with 1% resistor and digital calibration, since it's far from the rails.


Input stage (alt.2, MCP6004 protected, 0..5V)
[here](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWEBmAHAJmgdgGzoRmACzICcpkORICkNNdCApgLRhgBQRkpIe1yZDhC5UKIeCjRhdGJHTIw8ZSvkh0HAEp9UY5JGo5d6rOijnu4M7KkItOvUYchuNupbbXzMO9oRgzZHR3IJRg8whLajdoOwB5ESdBYVQ4cRkoDgAnPnQBcP9A8LowVPhsvkt0PMSxaujwFQ4Ad1r1GtF0zNaqOnrc6n7IFoH2wxqg2RH+cBw6GaGOMCxqIiIxdjM1jdIzCFp4KXID1WUITSYAZwBLS4AXAEMAOwBjJntt8ACXdZBU7xcjABPgqCF2s3cVAhESaOTBZn6RFIwiGjWUH2RYxcmPwGXcdBYSBidgA5qNkAgzDMiGA8SN4ViFjVhq0GbT5jV2Zkcp9gmJeaYYeiyQKtrSTF5hgB7FyVCyoHDCJRSCB0ZAoczq5AcIA)
Build with 0.1% 100k and 180k resistors. Digital calibration after that.
