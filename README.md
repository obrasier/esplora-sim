# Esplora Simulator #

This is a simulator to run students Arduino Esplora code and save the state. It enables the running of Arduino sketches, and can receive input messages for all the peripheral hardware on the Esplora.

### How do I get it? ###

Clone from github and compile:
```bash
$ git clone https://github.com/obrasier/esplora-sim
$ cd esplora-sim
$ make
```

This will create a build directory with the executable in `build/esplora_sim`. 

The sketch should be placed in `src/sketch/sketch.ino`, and run `make` to compile and build.

It is possible to see the output in the microbit simulator running `run_gui.sh` after you have compiled the program.
