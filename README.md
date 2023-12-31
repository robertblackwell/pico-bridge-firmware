This project is the C/C++ code for a robot hardware interface running on a Pi Pico.

The 'robot' at this point is really only a custom strip-board prototype and a couple of Pololu 25D motors attached.

The pico app is based on the Raspberry Pi Pico SDK. 

# Serial Communication

Tested UART communication by connecting 2 pi picos using uart0 at baud rate of 115200. Data throughput of approx 10,000 chars/sec.

Tested 
    -   uart0 on tx=0/rx=1
    -   uart0 on tx=16/rx=17
    -   uart1 on tx=8/rx=9

## Debugging
https://www.digikey.com.au/en/maker/projects/raspberry-pi-pico-and-rp2040-cc-part-2-debugging-with-vs-code/470abc7efb07432b82c95f6f67f184c0

https://www.youtube.com/watch?v=5rYwKmTVt4M&t=12s

https://github.com/majbthrd/pico-debug/blob/master/howto/openocd.md


#### Connections

```
Pico Probe USB                      --> dev machine
Pico Probe connector labelled debug --> 3 debug pins on the Target Pico
```
#### Control/Dar=ta Flow

On the dev machine run the following entities

<pre>
    IDE --> GDB --> OpenOCD 
</pre>

The openocd processes connects to the PicoProbe via USB

The command for the openocd server is:

```
openocd -s ${HOME}/pico/openocd/tcl/ -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000"

```

The value `${HOME}/pico/openocd/tcl/` should point into the location of your openocd installation.



## Refernces

Good website for rp2040 pwm explanation
https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=2

https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=1

A program for loading code into a pico over a serial line
https://github.com/raspberrypi/picotool/blob/master/main.cpp

Loading programs to a pico using SWD using a Raspberry PI
https://www.electronicshub.org/programming-raspberry-pi-pico-with-swd/

https://www.jeffgeerling.com/blog/2023/testing-raspberry-pis-new-debug-probe

https://blog.smittytone.net/2021/02/05/how-to-debug-a-raspberry-pi-pico-with-a-mac-swd/

https://www.digikey.com.au/en/maker/projects/raspberry-pi-pico-and-rp2040-cc-part-2-debugging-with-vs-code/470abc7efb07432b82c95f6f67f184c0


Good ROS2 project to emulate
https://github.com/linorobot/linorobot2

Starting point for understanding ROS2
https://docs.ros.org/en/foxy/Tutorials.html

Pi Pico OS
https://github.com/garyexplains/piccolo_os_v1/tree/main

Pico std Uart and Usb different output
https://www.youtube.com/watch?v=2EKnANSZQKI

Reference manual for rp2040 Arm Cortex M0 processor
https://developer.arm.com/documentation/ddi0484/c/Programmers-Model

Microros on pi pico
https://ubuntu.com/blog/getting-started-with-micro-ros-on-raspberry-pi-pico

https://github.com/micro-ROS/micro-ROS-Agent

https://github.com/micro-ROS/micro_ros_arduino

https://www.youtube.com/watch?v=aD3Lf-9cb0A

https://github.com/ros-drivers/rosserial

https://github.com/yoneken/rosserial_stm32

build ros subscriber and publisher ros2
https://docs.ros.org/en/iron/Tutorials/Beginner-CLI-Tools/Configuring-ROS2-Environment.html

//xrce-dds use manual
https://usermanual.wiki/m/5285fe9aa500a3e8a817f094ccd4f2366c36faacdbc7fe7bc9af937338b95954.pdf#section.5.3

http://wiki.ros.org/rosserial#Protocol
http://wiki.ros.org/rosserial_python
https://github.com/ros-drivers/rosserial/blob/noetic-devel/rosserial_python/src/rosserial_python/SerialClient.pyros2 rclcpp github

MQTT - ROS
https://github.com/ika-rwth-aachen/mqtt_client/tree/main
https://github.com/robofoundry/ros2_mqtt


MQTT serial

https://www.metacodes.pro/funcodes/using_tty2mqtt_to_bridge_between_serial_communication_and_mqtt/
https://courses.ideate.cmu.edu/16-376/s2021/ref/text/code/mqtt-serial-bridge.html

https://github.com/eclipse/paho.mqtt.embedded-c/tree/master
https://eclipse.dev/paho/index.php?page=clients/c/embedded/index.php
This is a multi-part project
https://www.rs-online.com/designspark/mqtt-part-1-developing-iiot-networks

https://pypi.org/project/paho-mqtt/


Demo ROS2 serial - Josh Newans
This is the repo for the python code on the Raspberry PI that listens to the serial port
and publishes into the ROS graph
https://github.com/joshnewans/serial_motor_demo

MicroRos for raspberry pi pico
https://github.com/micro-ROS/micro_ros_raspberrypi_pico_sdk


micro-ros docker image builds
https://hub.docker.com/r/microros/micro-ros-agent

Lets start thinking about doing all the code n a raspberry pi (NOT PICO)
https://github.com/lbrombach
https://docs.donkeycar.com/parts/odometry/
