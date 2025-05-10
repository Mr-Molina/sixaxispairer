# PlayStation Controller Differences

Based on the analysis of the connected controllers, here are the key differences that affect how the code interacts with them:

## DualShock 4 [CUH-ZCT2x] (0x09cc)

1. **Multiple Interfaces**: The DualShock 4 presents multiple interfaces to the system:
   - Interface 0: Audio Control
   - Interface 1: Audio Streaming (OUT)
   - Interface 2: Audio Streaming (IN)
   - Interface 3: Human Interface Device (HID) - This is the one we need for controller functions

2. **Access Permissions**: The device is accessible only to root by default:
   ```
   crw------- 1 root root 246, 2 May 10 15:05 /dev/hidraw2
   ```

3. **Device Path Format**: The path includes the interface number:
   ```
   1-2:1.3  (where .3 indicates interface 3)
   ```

4. **Feature Reports**: The DualShock 4 may support different feature reports than other controllers.

## Why Different Behavior

The main reasons for different behavior between controllers:

1. **Interface Selection**: The code needs to specifically target interface 3 for DualShock 4, while other controllers might use different interfaces or have only one interface.

2. **Permission Issues**: Since the device is only accessible to root, running without sudo may cause connection failures.

3. **Feature Report Compatibility**: Different controllers support different feature reports, so the code needs to try multiple report IDs for different controller types.

4. **Connection Method**: Some controllers work better with `hid_open_path()` while others work better with `hid_open()`.

## Solution

The updated code now:

1. Enumerates all connected controllers
2. Shows their details including interface numbers
3. Lets you select which controller to use
4. Uses the appropriate connection method based on controller type
5. Tries multiple feature report IDs for different controller types

This ensures consistent behavior regardless of which controller you're using.

## Recommendations

1. Run the program with sudo to ensure proper access to the devices
2. When multiple controllers are connected, select the one with the appropriate interface
3. For DualShock 4, always select the one with interface 3 (HID interface)