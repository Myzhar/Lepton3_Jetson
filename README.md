# Lepton3_Jetson

Library and examples to connect the FLIR Lepton3 thermal camera to Nvidia Jetson embedded boards

<img src="images/Lepton3.jpg" width="250" height="250">

## Prerequisites

* [Flir Lepton 3 module](https://www.flir.it/products/lepton/?model=500-0276-01) (can work also with Lepton 3.5)
* [Breakout Board v1.4 by Getlab](https://groupgets.com/manufacturers/getlab/products/flir-lepton-breakout-board-v1-4)
* [An Nvidia Jetson board](https://www.nvidia.com/en-us/autonomous-machines/jetson-store/) (tested on Jetson Nano with Jetpack 3.3)
* OpenCV library to compile the examples
* CMake > 2.8.9

## Software Installation

Clone this repository

`$ git clone https://github.com/Myzhar/Lepton3_Jetson.git`

Install build requirements

`$ sudo apt install build-essential cmake g++ libopencv-dev`
    
Compile

```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ cd ..
```

## Camera connection

<img src="images/Connections.png" width="250" height="250">

Detailed instruction about how to connect the Lepton3 module to a Nvidia Jetson Nano are available on [Myzhar website](https://www.myzhar.com/blog/?p=4500)

<img src="images/JetsonNanoGPIO.jpg" width="250" height="250">

## Run the Demo
Two examples are provided to illustrate how to use the `lepton3_grabber` static library available in the folder `build/grabber_lib`.

### OpenCV Demo
With this sample you can see how to use OpenCV to display the thermal stream and control the camera behaviors

```
$ cd build/opencv_demo
$ ./opencv_demo
```

Keyboard commands:
* `c` -> RGB mode (24bit RGB color images)
* `r` -> Radiometry mode (16 bit gray image containing 14 bit linear thermal values )
* `h` -> High gain mode (-10°C to 140°C with 5°C of accuracy)
* `l` -> Low gain mode ( -10°C to 400°C with 10°C of accuracy)
* `a` -> Auto gain mode
* `f` -> Perform FFC normalization
* `F` -> Perform FFC radiometry normalization

### Fever control demo
With this demo you can see how to estimate temperatures from 16 bit gray images to evaluate the temperature of a person and get alarms in case of fever. 
I created this demo as a security sample application to be used in the COVID19 period.

```
$ cd build/check_fever_app
$ ./check_fever_app
```

Using keyboard `u`/`d` you can increase/decrease the estimated temperature su simulate person fever.


    

