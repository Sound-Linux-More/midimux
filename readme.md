# midimux

## what is it?
midimux is a 2to1 multiplexer/router for the ALSA MIDI sequencer

## why does it exist?
because csound can work with only 1 midi port at a time, so you can't use a midi
controller and a midi softsynth at the same time. I needed something minimal to
funnel multiple ports to multiple channels on a single port.

## how does it work?
midimux, by default, creates 3 ports: MUX, DEV00 and DEV01.

- what is received on DEV00 (any channel) is sent to MUX, channel 0
- what is received on DEV01 (any channel) is sent to MUX, channel 1
- what is sent to MUX, channel 0, is sento DEV00, channel 0
- what is sent to MUX, channel 1, is sento DEV01, channel 0
- what is sent to MUX, any other channel, is discarded
- non-channel midi messages are passed unchanged

## can I use multiplex more than 2 ports?

sure, just execute *midimux N* where N is the number of ports you need.

## how do I use it?

- clone the repo
- make
- ./midimux
- use *aconnect -l* to find client and port number of the devices to multiplex
  (say $CONTROLLERCLIENT, $CONTROLLERPORT, $SYNTHCLIENT, $SYNTHPORT)
- connect midimux to your device and synth bidirectionally with:
  - aconnect midimux:1 $CONTROLLERCLIENT:$CONTROLLERPORT
  - aconnect $CONTROLLERCLIENT:$CONTROLLERPORT midimux:1
  - aconnect midimux:2 $SYNTHCLIENT:$SYNTHPORT
  - aconnect $SYNTHCLIENT:$SYNTHPORT midimux:2

now you have a port on midimux:0 with your controller and synth multiplexed.
Enjoy!

## how do I stop it?

good old *CTRL-C*

## what license is it under?

GPL3

## anything else?

not really.

## goodbye?

goodbye.
