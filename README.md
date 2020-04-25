# Azure Sphere H-Bridge
This implements an H-Bridge motor interafce for the Azure Sphere using GPIO and a L293D Push Pull 4 Channel Driver. It is a port of a previous [djaus2/DNETCoreGPIO](https://github.com/djaus2/DNETCoreGPIO) repository, the 6th option, **"H-Bridge Motor using L293D"**. That project uses the [dotnet/io](https://github.com/dotnet/iot) repository functionality to control a motor using GPIO via .Net Core on a Rapsberry Pi (IoT-Core and Raspian). There is also an extension of that project, Azure IoT SDK Quickstart Telemetry project available that is part of the [djaus2/az-iothub-ps](https://github.com/djaus2/az-iothub-ps/tree/master/PS/qs-apps/quickstarts/telemetry/control-a-motor) repository where the motor is controlled remotely via an IoT Hub. This port cuurrently doesn't include the IoT Hub Telemetry aspect _(coming later)_ but does continuously control the motor from a loop in the app's main function.

## Motor Control Pins
See circuit diagram in Circuits folder, left part.   
Pins (L293D pins in brackets):
- Enable  GPIO0 (E1) AzSphere Pin 4
- Reverse GPIO3 (I1) AzSphere Pin 8
- Forward GPIO2 (I2) AzSphere Pin 10
  
See settings in **app_manifest.json**  
The GPIO pins are some what arbitrary so you can change them there.  
That needs to match the settings in **hbridge.h**
