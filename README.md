### Abstract
    simple OPCUA-Server using OPEN62541
    this OPCUA-Server received 3 different sensor information from arduino using serial USB connection.

### Requirement
 - raspberryPi 2 or 3 ( https://www.raspberrypi.org/downloads/raspbian/ )
 - Arduino with 3 sensor ( Analog Ambient light sensor, Analog Sound sensor, DHT11 sensor)

### Using open62541
A general introduction to OPC UA and the open62541 documentation can be found at http://open62541.org/doc/current.
Past releases of the library can be downloaded at https://github.com/open62541/open62541/releases.
To use the latest improvements, download a nightly build of the *single-file distribution* (the entire library merged into a single source and header file) from http://open62541.org/releases. Nightly builds of MSVC binaries of the library are available [here](https://ci.appveyor.com/project/open62541/open62541/build/artifacts).

** Building the Library 
reference ( https://open62541.org/doc/0.2/building.html )
 - sudo apt-get install liburcu-dev # for multithreading
 - sudo apt-get install check # for unit tests
 - sudo apt-get install python-sphinx graphviz # for documentation generation
 - sudo apt-get install python-sphinx-rtd-theme # documentation style

 - cd OPCUA_Sensor_Server-master
 - mkdir build
 - cd build
 - cmake .. 
 - make
**select additional features
 - ccmake .. (UA_ENABLE_AMALGAMATION : ON --> create open62541.c) 
 - make

### Using wiringPi
wiringPi library supported rasberryPi serial transport, pi-thread function .. etc

** wiringPi build & install
 - sudo apt-get update
 - sudo apt-get upgrade
 - sudo apt-get install git-core
 - git clone git://git.drogon.net/wiringPi

 - cd wiringPi ( move wiringPi directory )
 - sudo ./build ( build/ install )

 - gpio -v ( check version )

### Server Implementation
** server source code -> opcua-sensor-server/opcua_server_src/opcua_server_3sensor.c

1) Connect Arduino and raspberryPi with serial USB cable
 - include Arduino code ( Arduino/ 3sensor_check )
 - include dht11 library same folder 

2) opcua server run
 - gcc -std=c99 open62541.c opcua_server_3sensor.c -o opcua_server -lwiringPi
 - ./opcua_server


test