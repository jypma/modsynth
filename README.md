# Eurorack modules collection

This repository contains a collection of eurorack module open hardware designs. It is a combination of designs found elsewhere (with references where relevant), and designs by me.

All schematics and boards are in Kicad format, and schematics, board and gerber exports are automated using KiBot and docker (see [export.sh](export.sh)).

## Modules

The following modules are available:

- A [Dual 4-channel mixer](DualMixer) module designed by me (based on basic opamp mixer essentials)
- [Ornament & Crime](OrnamentCrime), a well-known open hardware design
- A [Quad Attenuverter](QuadAttenuverter) with offset, based on an internet design
- Ray Wilson's  [Dual-VCA](RayWilson-Dual-VCA) as originally designed by Gerbrand Sterrenburg
- [Roland System 100 ADSR](SCM-140-ADSR) as originally designed by Gerbrand Sterrenburg
- Thomas Henry's [555 VCO](TH-555-VCO) as originally designed by Gerbrand Sterrenburg
- The YUSynth [Steiner VCF](Steiner-VCF) as originally designed by Gerbrand Sterrenburg
- Music Thing Modular's [Turing Machine](TuringMachine), a well-known open hardware design

## Utilities

- A 16-bin [DIP to SOIC](adapter-soic16) adapter board (to mount SOIC chips on DIP sockets)
- A [Power distribution board](power-board) for eurorack modules

## Combined bill of material and shopping list

I'm maintaining a merged [bill of material](bom.org) for all modules, with component lists in stock at time of writing.

The only exception is the Teensy 3.0 / 3.1 / 3.2 required for _Ornament & Crime_. Those are impossible to find at the moment (I happen to have some, hence it's on the list).

# Generating BOM, gerbers, and pictures

The script `export.sh` in the root of the project will generate or update the artifacts in the  `export/` directory, such as BOMs, gerber files, packages, and various documentation pictures. Most of these files are not version-controlled (but those linked from the markdown documentation are).

Whenever changing a schematic, the export script will update the relevant pictures and artifacts. Re-exporting the same schematic will result in the same picture, so `git` plays nice with this.

## Link for simulation of triangle to sine converter

[here](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWK0AckDMYwE4As3sA2SQgdgCYFsQFIaa6EBTAWiwCgAlEXOEciiHTpC-QXXQ8+SOrOgJ2YUrh7pyY9QjDqB6iGHh8YudJFIhOAewCuAOwAm7AO5S6u1TvHsATjXIoNPwD0BHU3cjh2AHMgoVDY9BQVWXYAFxpiQMJyFXc6CBZcaFJabWylCu1JGARCBDUwFFI8BF5cclEDOnsmADMAQ2sAG1TnDLdBUIC8nxBs3Jy5xbBcZPBDLiWVbQD5kCIoQ95wMMOasb2drfBVqAvF8kW9ylFIWcJj-wDSSFyUAIk2DeShUHwkIhAPxUwk66yM8gsTAAzgBLJGpfq2ADGTAuxxhc0+-zuLjB-GJz2JbxcUKEENpXzuvjI6kSoMEqwB-A2pMEbLmfIid3SaAm6lIhGm4nAIEKxUIYFCuHq2S0cFe8ioCAo+GU2CUyk6fB6A2GoxpkvJ31+VqZIBQYFEphUDqdlvCkRirqElu9vFkJPtjqENu9MxcYcEosCb18Eumi3j4Ey+Q23CTVyTBwDx20Z2QCgtAUzlsdKSL-ETlped141BQB0ZXWmxIgMFEbDAqBQrQ+P3IitwxHMMEg5Ewtcg1CwNudJz4c7b0A7Bmg6FIphyiuIpgII9g47bmzwuVq+1ujzWdFzpzkCm4WHi7kfOmOOfyt4Lx9uajoJ7in7HGwn7nA+iqshCL4+ly150MB+bnL4DadBK+zYHoqGpvAYzIeAqG4eGaEYUakDFphYzNnhJEtgCOGNlGaAxjhjHuP8bhvsx7F0M8izUpC1aLEkCbJDhuDCfaYkhiJEaSXOuFzrG9qMf6Sngq8cKRBGjEEvJNpvF6yl8GxHhnLMBgPK2pE0YcXSaesxJNgYfy0S45nOXC1mKVgQpKJ0ET5Jh4C0HZ3kBZ0TlUYGM7bKh0W2nx2h8L584SHp0T2cEPlWQBpmudlv7gBFBVvAA8ilUknLJel3NwLI8JJdViQGsGynmzXyGMTX8GeXWXoGvWLFMlbSbEfVDTMlgZDwRwoIQRrIG29DNUIcyHE67BAA)
