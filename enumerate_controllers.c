/* Controller Enumerator - Lists all connected PlayStation controllers with detailed information */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <hidapi/hidapi.h>

/* ANSI color codes for terminal output */
#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_WHITE "\x1b[37m"
#define COLOR_BOLD "\x1b[1m"

/* Sony PlayStation vendor ID */
static const unsigned short VENDOR = 0x054c;

/* Product IDs for supported PlayStation controllers */
static const unsigned short PRODUCT[] = {
    0x0268, /* PlayStation 3 SixAxis controller */
    0x042f, /* PlayStation Move Motion controller */
    0x09cc  /* Sony Corp. DualShock 4 [CUH-ZCT2x] */
};

/* DualShock 4 specific constants */
#define DS4_PRODUCT_ID 0x09cc
#define DS4_HID_INTERFACE 3

/**
 * Gets a human-readable name for a PlayStation controller based on its product ID
 */
static const char* get_controller_name(unsigned short product_id)
{
    if (product_id == 0x0268)
        return "SixAxis Controller";
    else if (product_id == 0x042f)
        return "Move Motion Controller";
    else if (product_id == DS4_PRODUCT_ID)
        return "DualShock 4 [CUH-ZCT2x]";
    else
        return "Compatible Device";
}

/**
 * Checks if a device is a supported PlayStation controller
 */
static int is_supported_controller(unsigned short vendor_id, unsigned short product_id)
{
    if (vendor_id != VENDOR)
        return 0;
        
    for (size_t i = 0; i < sizeof(PRODUCT) / sizeof(*PRODUCT); i++)
    {
        if (product_id == PRODUCT[i])
            return 1;
    }
    
    return 0;
}

/**
 * Attempts to get feature reports from a device
 */
static void test_feature_reports(hid_device *dev, struct hid_device_info *info)
{
    unsigned char report_buf[256];
    int ret;
    
    printf("%s│  Feature Report Tests:%s\n", COLOR_MAGENTA, COLOR_RESET);
    
    /* Test common report IDs */
    unsigned char report_ids[] = {0x01, 0xF2, 0xF5, 0xA3, 0x12, 0x81};
    
    for (size_t i = 0; i < sizeof(report_ids); i++)
    {
        memset(report_buf, 0, sizeof(report_buf));
        report_buf[0] = report_ids[i];
        ret = hid_get_feature_report(dev, report_buf, sizeof(report_buf));
        
        if (ret > 0)
        {
            printf("%s│    Report 0x%02x: %sSupported%s (", 
                   COLOR_MAGENTA, report_ids[i], COLOR_GREEN, COLOR_RESET);
            
            for (int j = 0; j < 8 && j < ret; j++)
            {
                printf("%02x ", report_buf[j]);
            }
            
            if (ret > 8)
                printf("...");
                
            printf(")\n");
        }
        else
        {
            printf("%s│    Report 0x%02x: %sNot supported%s\n", 
                   COLOR_MAGENTA, report_ids[i], COLOR_RED, COLOR_RESET);
        }
    }
}

/**
 * Displays all connected Sony devices with detailed information
 */
static void list_all_sony_devices(void)
{
    struct hid_device_info *devs, *cur_dev;
    hid_device *dev;
    int found_devices = 0;
    
    printf("%s%s=== Enumerating All Sony Devices ===%s\n\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
    
    /* Enumerate all Sony devices */
    devs = hid_enumerate(VENDOR, 0);
    cur_dev = devs;
    
    while (cur_dev)
    {
        found_devices++;
        printf("%s%s┌─ Device %d ─────────────────────────────────────%s\n", 
               COLOR_BOLD, COLOR_MAGENTA, found_devices, COLOR_RESET);
        printf("%s│  Vendor ID:       0x%04x (Sony)\n", COLOR_MAGENTA, cur_dev->vendor_id);
        printf("%s│  Product ID:      0x%04x", COLOR_MAGENTA, cur_dev->product_id);
        
        if (is_supported_controller(cur_dev->vendor_id, cur_dev->product_id))
        {
            printf(" %s(%s)%s", COLOR_YELLOW, get_controller_name(cur_dev->product_id), COLOR_RESET);
        }
        
        printf("\n");
        printf("%s│  Manufacturer:    %ls\n", COLOR_MAGENTA,
               cur_dev->manufacturer_string ? cur_dev->manufacturer_string : L"(Unknown)");
        printf("%s│  Product:         %ls\n", COLOR_MAGENTA,
               cur_dev->product_string ? cur_dev->product_string : L"(Unknown)");
        printf("%s│  Serial Number:   %ls\n", COLOR_MAGENTA,
               cur_dev->serial_number ? cur_dev->serial_number : L"(None)");
        printf("%s│  Interface:       %d", COLOR_MAGENTA, cur_dev->interface_number);
        
        if (cur_dev->product_id == DS4_PRODUCT_ID && cur_dev->interface_number == DS4_HID_INTERFACE)
        {
            printf(" %s(HID Interface - Preferred)%s", COLOR_GREEN, COLOR_RESET);
        }
        
        printf("\n");
        printf("%s│  Path:            %s\n", COLOR_MAGENTA, cur_dev->path);
        printf("%s│  Release Number:  %hx.%hx\n", COLOR_MAGENTA, 
               cur_dev->release_number >> 8, cur_dev->release_number & 0xff);
        printf("%s│  Usage Page:      0x%04x\n", COLOR_MAGENTA, cur_dev->usage_page);
        printf("%s│  Usage:           0x%04x\n", COLOR_MAGENTA, cur_dev->usage);
        
        /* Try to open the device and test feature reports */
        dev = hid_open_path(cur_dev->path);
        if (dev)
        {
            test_feature_reports(dev, cur_dev);
            hid_close(dev);
        }
        else
        {
            printf("%s│  %sCould not open device for testing%s\n", 
                   COLOR_MAGENTA, COLOR_RED, COLOR_RESET);
        }
        
        printf("%s└───────────────────────────────────────────────%s\n\n", COLOR_MAGENTA, COLOR_RESET);
        
        cur_dev = cur_dev->next;
    }
    
    if (found_devices == 0)
    {
        printf("%s[INFO]%s No Sony devices found.\n", COLOR_BLUE, COLOR_RESET);
    }
    else
    {
        printf("%s[INFO]%s Found %s%d%s Sony device(s).\n",
               COLOR_BLUE, COLOR_RESET, COLOR_YELLOW, found_devices, COLOR_RESET);
    }
    
    /* Clean up */
    hid_free_enumeration(devs);
}

/**
 * Lists all USB HID devices (not just Sony)
 */
static void list_all_hid_devices(void)
{
    struct hid_device_info *devs, *cur_dev;
    int found_devices = 0;
    
    printf("%s%s=== Enumerating All USB HID Devices ===%s\n\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
    
    /* Enumerate all USB HID devices */
    devs = hid_enumerate(0, 0);
    cur_dev = devs;
    
    while (cur_dev)
    {
        found_devices++;
        printf("%s%s┌─ Device %d ─────────────────────────────────────%s\n", 
               COLOR_BOLD, COLOR_MAGENTA, found_devices, COLOR_RESET);
        printf("%s│  Vendor ID:       0x%04x", COLOR_MAGENTA, cur_dev->vendor_id);
        
        if (cur_dev->vendor_id == VENDOR)
        {
            printf(" %s(Sony)%s", COLOR_YELLOW, COLOR_RESET);
        }
        
        printf("\n");
        printf("%s│  Product ID:      0x%04x", COLOR_MAGENTA, cur_dev->product_id);
        
        if (is_supported_controller(cur_dev->vendor_id, cur_dev->product_id))
        {
            printf(" %s(%s)%s", COLOR_YELLOW, get_controller_name(cur_dev->product_id), COLOR_RESET);
        }
        
        printf("\n");
        printf("%s│  Manufacturer:    %ls\n", COLOR_MAGENTA,
               cur_dev->manufacturer_string ? cur_dev->manufacturer_string : L"(Unknown)");
        printf("%s│  Product:         %ls\n", COLOR_MAGENTA,
               cur_dev->product_string ? cur_dev->product_string : L"(Unknown)");
        printf("%s│  Serial Number:   %ls\n", COLOR_MAGENTA,
               cur_dev->serial_number ? cur_dev->serial_number : L"(None)");
        printf("%s│  Interface:       %d\n", COLOR_MAGENTA, cur_dev->interface_number);
        printf("%s│  Path:            %s\n", COLOR_MAGENTA, cur_dev->path);
        printf("%s│  Release Number:  %hx.%hx\n", COLOR_MAGENTA, 
               cur_dev->release_number >> 8, cur_dev->release_number & 0xff);
        printf("%s│  Usage Page:      0x%04x\n", COLOR_MAGENTA, cur_dev->usage_page);
        printf("%s│  Usage:           0x%04x\n", COLOR_MAGENTA, cur_dev->usage);
        printf("%s└───────────────────────────────────────────────%s\n\n", COLOR_MAGENTA, COLOR_RESET);
        
        cur_dev = cur_dev->next;
    }
    
    if (found_devices == 0)
    {
        printf("%s[INFO]%s No USB HID devices found.\n", COLOR_BLUE, COLOR_RESET);
    }
    else
    {
        printf("%s[INFO]%s Found %s%d%s USB HID device(s).\n",
               COLOR_BLUE, COLOR_RESET, COLOR_YELLOW, found_devices, COLOR_RESET);
    }
    
    /* Clean up */
    hid_free_enumeration(devs);
}

int main(int argc, char **argv)
{
    /* Initialize the HID API */
    if (hid_init() != 0)
    {
        fprintf(stderr, "%s[ERROR]%s Failed to initialize HID API\n", COLOR_RED, COLOR_RESET);
        return 1;
    }
    
    /* Check command line arguments */
    if (argc > 1 && strcmp(argv[1], "-a") == 0)
    {
        list_all_hid_devices();
    }
    else
    {
        list_all_sony_devices();
    }
    
    /* Provide a summary of key differences */
    printf("\n%s%s=== Key Differences Between Controllers ===%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
    printf("1. DualShock 4 has multiple interfaces, with interface 3 being the HID interface\n");
    printf("2. Different controllers support different feature reports\n");
    printf("3. The path format differs between controller types\n");
    printf("4. Usage page and usage values may differ\n");
    printf("5. Some controllers may require specific permissions to access\n");
    printf("\nTry running with sudo for better access to devices:\n");
    printf("  sudo ./enumerate_controllers     - List Sony devices\n");
    printf("  sudo ./enumerate_controllers -a  - List all USB HID devices\n");
    
    /* Clean up */
    hid_exit();
    
    return 0;
}