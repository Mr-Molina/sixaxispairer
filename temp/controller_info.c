/**
 * controller_info.c - PlayStation controller information and detection
 * 
 * Implementation of functions for working with PlayStation controllers
 */

#include "controller_info.h"
#include <stdlib.h>
#include <string.h>

/* Array of supported product IDs */
static const unsigned short SUPPORTED_PRODUCTS[] = {
    PRODUCT_SIXAXIS,  /* PlayStation 3 SixAxis controller */
    PRODUCT_MOVE,     /* PlayStation Move Motion controller */
    PRODUCT_DS4       /* Sony Corp. DualShock 4 [CUH-ZCT2x] */
};

/**
 * Gets a human-readable name for a PlayStation controller based on its product ID
 */
const char* get_controller_name(unsigned short product_id)
{
    if (product_id == PRODUCT_SIXAXIS)
        return "SixAxis Controller";
    else if (product_id == PRODUCT_MOVE)
        return "Move Motion Controller";
    else if (product_id == PRODUCT_DS4)
        return "DualShock 4 [CUH-ZCT2x]";
    else
        return "Compatible Device";
}

/**
 * Determines if the device is a DualShock 4 controller
 */
int is_dualshock4(hid_device *dev)
{
    struct hid_device_info *device_info = hid_get_device_info(dev);
    if (device_info && device_info->vendor_id == VENDOR_SONY && device_info->product_id == PRODUCT_DS4)
    {
        return 1;
    }
    return 0;
}

/**
 * Creates a deep copy of controller information
 */
controller_info_t* create_controller_info(struct hid_device_info *device_info)
{
    controller_info_t *info = (controller_info_t*)malloc(sizeof(controller_info_t));
    if (!info) return NULL;
    
    memset(info, 0, sizeof(controller_info_t));
    
    info->device_info = device_info;
    info->vendor_id = device_info->vendor_id;
    info->product_id = device_info->product_id;
    info->interface_number = device_info->interface_number;
    
    /* Copy strings */
    if (device_info->path) {
        info->path = strdup(device_info->path);
    }
    
    if (device_info->manufacturer_string) {
        size_t len = wcslen(device_info->manufacturer_string) + 1;
        info->manufacturer_string = (wchar_t*)malloc(len * sizeof(wchar_t));
        if (info->manufacturer_string) {
            wcscpy(info->manufacturer_string, device_info->manufacturer_string);
        }
    }
    
    if (device_info->product_string) {
        size_t len = wcslen(device_info->product_string) + 1;
        info->product_string = (wchar_t*)malloc(len * sizeof(wchar_t));
        if (info->product_string) {
            wcscpy(info->product_string, device_info->product_string);
        }
    }
    
    if (device_info->serial_number) {
        size_t len = wcslen(device_info->serial_number) + 1;
        info->serial_number = (wchar_t*)malloc(len * sizeof(wchar_t));
        if (info->serial_number) {
            wcscpy(info->serial_number, device_info->serial_number);
        }
    }
    
    /* For DualShock 4, prefer interface 3 */
    if (device_info->product_id == PRODUCT_DS4 && device_info->interface_number == DS4_HID_INTERFACE) {
        info->is_preferred = 1;
    } else {
        info->is_preferred = 0;
    }
    
    return info;
}

/**
 * Frees a controller_info_t structure
 */
void free_controller_info(controller_info_t *info)
{
    if (!info) return;
    
    if (info->path) free(info->path);
    if (info->manufacturer_string) free(info->manufacturer_string);
    if (info->product_string) free(info->product_string);
    if (info->serial_number) free(info->serial_number);
    
    free(info);
}

/**
 * Checks if a device is a supported PlayStation controller
 */
int is_supported_controller(unsigned short vendor_id, unsigned short product_id)
{
    if (vendor_id != VENDOR_SONY)
        return 0;
        
    for (size_t i = 0; i < sizeof(SUPPORTED_PRODUCTS) / sizeof(*SUPPORTED_PRODUCTS); i++)
    {
        if (product_id == SUPPORTED_PRODUCTS[i])
            return 1;
    }
    
    return 0;
}

/**
 * Finds all supported controllers and returns their information
 */
int find_controllers(controller_info_t *controllers[], int max_controllers)
{
    struct hid_device_info *devs, *cur_dev;
    int controller_count = 0;
    
    /* Enumerate all Sony devices */
    devs = hid_enumerate(VENDOR_SONY, 0);
    cur_dev = devs;
    
    /* First pass: identify all supported devices */
    while (cur_dev && controller_count < max_controllers)
    {
        if (cur_dev->vendor_id == VENDOR_SONY)
        {
            /* Check if this is a supported controller */
            if (is_supported_controller(cur_dev->vendor_id, cur_dev->product_id))
            {
                /* Create controller info */
                controller_info_t *info = create_controller_info(cur_dev);
                if (!info) continue;
                
                /* Add to controllers array */
                controllers[controller_count++] = info;
            }
        }
        cur_dev = cur_dev->next;
    }
    
    /* Sort controllers to prioritize preferred ones (like DS4 with interface 3) */
    for (int i = 0; i < controller_count - 1; i++) {
        for (int j = i + 1; j < controller_count; j++) {
            if (controllers[j]->is_preferred && !controllers[i]->is_preferred) {
                /* Swap controllers to put preferred ones first */
                controller_info_t *temp = controllers[i];
                controllers[i] = controllers[j];
                controllers[j] = temp;
            }
        }
    }
    
    /* Free the enumeration */
    hid_free_enumeration(devs);
    
    return controller_count;
}