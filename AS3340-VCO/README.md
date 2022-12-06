# TODO
- Need a pull-up from the CV IN pin to +12V of ~240K, to get the right frequency range for some (?) AS3340 chips. <- Done, bug in prototyping board?
- Need a 1M resistor between the output of the LIN CV pot and the actual chip pin 13. It's in the datasheet, too.
- Double-check the pin 1 vs. pin 3 for all pots
- Hard sync: reverse the diode (still pull to ground) to sync on rising edge
