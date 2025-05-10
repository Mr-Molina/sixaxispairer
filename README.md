# PlayStation Controller Pairer

A tool for viewing and setting the bluetooth address PlayStation controllers are paired with.

## Supported Controllers

* PlayStation 3 SixAxis controller (0x0268)
* PlayStation Move Motion controller (0x042f)
* Sony DualShock 4 [CUH-ZCT2x] (0x09cc)

## Dependencies

HID API (https://github.com/libusb/hidapi)

## Supported Platforms

* Windows
* Mac
* Linux

## Building

```
mkdir build
cd build
cmake ..
make
```

## Usage

```
./sixaxispairer         - Show current controller MAC address
./sixaxispairer [mac]   - Set controller MAC address (format: AABBCCDDEEFF or AA:BB:CC:DD:EE:FF)
./sixaxispairer -l      - List all connected Sony USB devices
./sixaxispairer -a      - List all connected USB devices (not just Sony)
./sixaxispairer -d      - Dump all available information from connected controller
./sixaxispairer -h      - Show help message
```

## Code Structure

The codebase is organized into the following modules:

* **mac_utils**: MAC address handling utilities
* **controller_info**: Controller detection and information
* **controller_connection**: Controller connection and communication
* **ui**: User interface and command handling
* **main**: Program entry point and command processing

## Notes for DualShock 4 Controllers

DualShock 4 controllers present multiple interfaces to the system:
* Interface 0: Audio Control
* Interface 1: Audio Streaming (OUT)
* Interface 2: Audio Streaming (IN)
* Interface 3: Human Interface Device (HID) - This is the one we need

When multiple controllers are connected, the program will let you select which one to use.

## Permissions

On Linux systems, you may need to run the program with sudo to access the controllers:

```
sudo ./sixaxispairer
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.