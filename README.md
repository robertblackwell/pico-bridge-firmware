# Pico Bridge Firmware

This project contains a number of firmware packages for a RaspberryPi Pico micro controller.

Each firmware package is a subdirectory of the project `src` directory, and they each draw to a varying extent on 
common code in the `src/common` subdirectory.

All packages a build using the __Pico c/c++ SDK__.

The packages trace my experience and experiments coming to grips with using a Pi Pico micro controller
and most of them are simple experiments of tests.

The latest step in that development is the package called __bridge__.

In addition to implementing some simple control functions for a differential drive wheeled robot
it also provides the micro controller end of a __ROS2-serial_bridge__.

See [https://github.com/robertblackwell/ros2-serial-bridge.git](https://github.com/robertblackwell/ros2-serial-bridge.git) for details
of the other end of the link.

# Building the firware

The firmware packages are built in the standard __CMake__ manner.

```bash

rm -rf build
mkdir build 
cd build
cmake ..
make

```

This will require the __Pico c/c++ SDK__ on your system and the 
that special `PICO_....` environment variables are set. See the SDK documentation for details.

# Loading the firward into the Pico

Again this is standard-operating-procedure.

Find the relevant `uf2` file in `build/src/<package-name>/<package>.uf2`.

Put the Pico ito mass storage mode (reset, disconnect, re-connect).

Copy the desired package to the Pico on y linux machine that looks like (from within the build dirrectory).

```bash
cp src/bridge/bridge.uf2  /media/user-name/RPI-RP2/
```

The Pico will reboot and start running the firmware.

# Tests

There are a number of directories with names of the form `src/test_??????`. These are not unit tests
in the usual sense. They are simply firmware programs that when loaded into a Pico
exercise various functions and interact with a human through (typically) the serial monitor of an
Arduino IDE.

The directories __tests_local_c_only__ is historical and will be deleted soon.

The directory __tests_on_hosts__ provides tests ro experiments for some of the code in `src/common` and those tests run 
in a standard Linux environment. Notice it has a standalone `CmakeLists.txt` file.
