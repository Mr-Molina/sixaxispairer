/**
 * controller_info.h - PlayStation controller information and detection
 * 
 * Defines structures and functions for working with PlayStation controllers
 */

#ifndef CONTROLLER_INFO_H
#define CONTROLLER_INFO_H

#include "platform_compat.h"
#include <wchar.h>

/* Sony PlayStation vendor ID */
#define VENDOR_SONY 0x054c

/* Product IDs for supported PlayStation controllers */
#define PRODUCT_SIXAXIS 0x0268  /* PlayStation 3 SixAxis controller */
#define PRODUCT_MOVE    0x042f  /* PlayStation Move Motion controller */
#define PRODUCT_DS4     0x09cc  /* Sony Corp. DualShock 4 [CUH-ZCT2x] */

/* DualShock 4 specific constants */
#define DS4_HID_INTERFACE 3     /* Interface number for HID on DualShock 4 */

/* Maximum number of controllers to handle */
#define MAX_CONTROLLERS 10

/* MAC address report ID for controller pairing */
#define MAC_REPORT_ID 0xf5

/**
 * Structure to store controller information
 */
typedef struct {
    struct hid_device_info *device_info;  /* Original device info from HIDAPI */
    char *path;                           /* Device path (copied from device_info) */
    unsigned short vendor_id;             /* Vendor ID */
    unsigned short product_id;            /* Product ID */
    int interface_number;                 /* Interface number */
    wchar_t *manufacturer_string;         /* Manufacturer string (copied from device_info) */
    wchar_t *product_string;              /* Product string (copied from device_info) */
    wchar_t *serial_number;               /* Serial number (copied from device_info) */
    int is_preferred;                     /* Flag for preferred devices (e.g., DS4 with interface 3) */
} controller_info_t;

/**
 * Gets a human-readable name for a PlayStation controller based on its product ID
 *
 * @param product_id The product ID of the controller
 * @return A string containing the controller name
 */
const char* get_controller_name(unsigned short product_id);

/**
 * Determines if the device is a DualShock 4 controller
 *
 * @param dev Handle to the HID device
 * @return 1 if the device is a DualShock 4, 0 otherwise
 */
int is_dualshock4(hid_device *dev);

/**
 * Creates a deep copy of controller information
 * 
 * @param device_info The original device info from HIDAPI
 * @return A new controller_info_t structure with copied data
 */
controller_info_t* create_controller_info(struct hid_device_info *device_info);

/**
 * Frees a controller_info_t structure
 * 
 * @param info The controller info to free
 */
void free_controller_info(controller_info_t *info);

/**
 * Checks if a device is a supported PlayStation controller
 * 
 * @param vendor_id The vendor ID of the device
 * @param product_id The product ID of the device
 * @return 1 if the device is supported, 0 otherwise
 */
int is_supported_controller(unsigned short vendor_id, unsigned short product_id);

/**
 * Finds all supported controllers and returns their information
 * 
 * @param controllers Array to store controller information
 * @param max_controllers Maximum number of controllers to find
 * @return Number of controllers found
 */
int find_controllers(controller_info_t *controllers[], int max_controllers);

#endif /* CONTROLLER_INFO_H */
