/**
 * controller_connection.c - PlayStation controller connection handling
 * 
 * Implementation of functions for connecting to and communicating with PlayStation controllers
 */

#include "controller_connection.h"
#include "mac_utils.h"
#include <stdio.h>
#include <string.h>

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

/**
 * Attempts to connect to a controller using the most appropriate method
 */
hid_device* connect_to_controller(controller_info_t *controller)
{
    hid_device *dev = NULL;
    const char* device_name = get_controller_name(controller->product_id);
    
    printf("%s[INFO]%s Connecting to %s (Interface: %d)...\n", 
           COLOR_BLUE, COLOR_RESET, device_name, controller->interface_number);
    
    /* Try to open by path first (more reliable) */
    dev = hid_open_path(controller->path);
    
    /* If that fails, try standard method */
    if (dev == NULL)
    {
        printf("%s[INFO]%s Path method failed, trying standard connection...\n", 
               COLOR_BLUE, COLOR_RESET);
        dev = hid_open(controller->vendor_id, controller->product_id, NULL);
    }
    
    /* If standard methods failed, try alternative methods based on controller type */
    if (dev == NULL)
    {
        /* For DualShock 4, try raw device method */
        if (controller->product_id == PRODUCT_DS4)
        {
            dev = connect_to_dualshock4_raw(controller);
        }
        /* For other controllers, try direct connection again */
        else
        {
            printf("%s[INFO]%s Standard methods failed, trying direct connection...\n", 
                   COLOR_BLUE, COLOR_RESET);
            
            dev = hid_open(controller->vendor_id, controller->product_id, NULL);
            if (dev != NULL)
            {
                printf("%s[SUCCESS]%s Connected to %s%s%s using direct connection\n",
                       COLOR_GREEN, COLOR_RESET, COLOR_YELLOW, device_name, COLOR_RESET);
            }
        }
    }
    else
    {
        printf("%s[SUCCESS]%s Connected to %s%s%s\n",
               COLOR_GREEN, COLOR_RESET, COLOR_YELLOW, device_name, COLOR_RESET);
    }
    
    return dev;
}

/**
 * Attempts to connect to a DualShock 4 controller using raw device access
 */
hid_device* connect_to_dualshock4_raw(controller_info_t *controller)
{
    hid_device *dev = NULL;
    
    printf("%s[INFO]%s Standard methods failed for DualShock 4, trying raw device access...\n", 
           COLOR_BLUE, COLOR_RESET);
    
    /* Try to execute system command to get device info */
    FILE *fp;
    char path[256];
    char command[512];
    
    /* Try to find the device path using system commands */
    sprintf(command, "find /dev/hidraw* -print 2>/dev/null | xargs -I{} sh -c 'udevadm info -q path -n {} 2>/dev/null | grep -q \"054c/%04x\" && echo {}'", PRODUCT_DS4);
    fp = _popen(command, "r");
    if (fp != NULL)
    {
        if (fgets(path, sizeof(path), fp) != NULL)
        {
            /* Remove newline if present */
            size_t len = strlen(path);
            if (len > 0 && path[len-1] == '\n')
                path[len-1] = '\0';
                
            printf("%s[INFO]%s Trying to open raw device: %s\n", 
                   COLOR_BLUE, COLOR_RESET, path);
                   
            /* Try to open the raw device directly */
            dev = hid_open_path(path);
            if (dev != NULL)
            {
                printf("%s[SUCCESS]%s Connected to DualShock 4 using raw device path\n", 
                       COLOR_GREEN, COLOR_RESET);
            }
        }
        _pclose(fp);
    }
    
    return dev;
}

/**
 * Retrieves and displays all available information from a HID device
 * by trying different report IDs
 */
void dump_device_info(hid_device *dev)
{
    struct hid_device_info *device_info = hid_get_device_info(dev);
    unsigned char report_buf[256];
    int ret;
    
    printf("\n%s%s=== Detailed Device Information ===%s\n", COLOR_BOLD, COLOR_GREEN, COLOR_RESET);
    
    /* Display basic device information from the HID API */
    if (device_info)
    {
        printf("%s%s┌─ Basic Device Information ─────────────────────%s\n", COLOR_BOLD, COLOR_MAGENTA, COLOR_RESET);
        printf("%s│  Vendor ID:       0x%04x%s%s\n", COLOR_MAGENTA, device_info->vendor_id,
               (device_info->vendor_id == VENDOR_SONY) ? COLOR_YELLOW " (Sony)" COLOR_RESET : "", COLOR_RESET);
        printf("%s│  Product ID:      0x%04x%s\n", COLOR_MAGENTA, device_info->product_id, COLOR_RESET);
        printf("%s│  Manufacturer:    %ls%s\n", COLOR_MAGENTA,
               device_info->manufacturer_string ? device_info->manufacturer_string : L"(Unknown)", COLOR_RESET);
        printf("%s│  Product:         %ls%s\n", COLOR_MAGENTA,
               device_info->product_string ? device_info->product_string : L"(Unknown)", COLOR_RESET);
        printf("%s│  Serial Number:   %ls%s\n", COLOR_MAGENTA,
               device_info->serial_number ? device_info->serial_number : L"(None)", COLOR_RESET);
        printf("%s│  Interface:       %d%s\n", COLOR_MAGENTA, device_info->interface_number, COLOR_RESET);
        printf("%s│  Path:            %s%s\n", COLOR_MAGENTA, device_info->path, COLOR_RESET);
        printf("%s│  Release Number:  %hx.%hx%s\n", COLOR_MAGENTA, 
               device_info->release_number >> 8, device_info->release_number & 0xff, COLOR_RESET);
        printf("%s│  Usage Page:      0x%04x%s\n", COLOR_MAGENTA, device_info->usage_page, COLOR_RESET);
        printf("%s│  Usage:           0x%04x%s\n", COLOR_MAGENTA, device_info->usage, COLOR_RESET);
        printf("%s└───────────────────────────────────────────────%s\n\n", COLOR_MAGENTA, COLOR_RESET);
    }
    
    /* Try to get controller-specific information using known report IDs */
    printf("%s%s┌─ Controller-Specific Information ──────────────%s\n", COLOR_BOLD, COLOR_MAGENTA, COLOR_RESET);
    
    /* Report 0xF2 - Controller information (firmware version, Bluetooth MAC) */
    memset(report_buf, 0, sizeof(report_buf));
    report_buf[0] = 0xF2;
    ret = hid_get_feature_report(dev, report_buf, sizeof(report_buf));
    if (ret > 0)
    {
        printf("%s│  [Report 0xF2] Controller Information:%s\n", COLOR_MAGENTA, COLOR_RESET);
        printf("%s│    Firmware Version: %d.%d%s\n", COLOR_MAGENTA, 
               report_buf[1], report_buf[2], COLOR_RESET);
        printf("%s│    Bluetooth MAC:    %02x:%02x:%02x:%02x:%02x:%02x%s\n", COLOR_MAGENTA,
               report_buf[4], report_buf[5], report_buf[6], 
               report_buf[7], report_buf[8], report_buf[9], COLOR_RESET);
    }
    
    /* Report 0xF5 - Current MAC address pairing */
    memset(report_buf, 0, sizeof(report_buf));
    report_buf[0] = MAC_REPORT_ID; /* 0xF5 */
    ret = hid_get_feature_report(dev, report_buf, sizeof(report_buf));
    if (ret > 0)
    {
        printf("%s│  [Report 0xF5] Current MAC Pairing:%s\n", COLOR_MAGENTA, COLOR_RESET);
        printf("%s│    Paired MAC:       %02x:%02x:%02x:%02x:%02x:%02x%s\n", COLOR_MAGENTA,
               report_buf[2], report_buf[3], report_buf[4], 
               report_buf[5], report_buf[6], report_buf[7], COLOR_RESET);
    }
    
    /* Try other known PlayStation controller report IDs */
    
    /* Report 0xA3 - PS3 Controller status */
    memset(report_buf, 0, sizeof(report_buf));
    report_buf[0] = 0xA3;
    ret = hid_get_feature_report(dev, report_buf, sizeof(report_buf));
    if (ret > 0)
    {
        printf("%s│  [Report 0xA3] Controller Status:%s\n", COLOR_MAGENTA, COLOR_RESET);
        printf("%s│    Data: ", COLOR_MAGENTA);
        for (int i = 1; i < 10 && i < ret; i++)
        {
            printf("%02x ", report_buf[i]);
        }
        printf("...%s\n", COLOR_RESET);
    }
    
    /* Report 0x01 - Controller capabilities/features */
    memset(report_buf, 0, sizeof(report_buf));
    report_buf[0] = 0x01;
    ret = hid_get_feature_report(dev, report_buf, sizeof(report_buf));
    if (ret > 0)
    {
        printf("%s│  [Report 0x01] Controller Capabilities:%s\n", COLOR_MAGENTA, COLOR_RESET);
        printf("%s│    Data: ", COLOR_MAGENTA);
        for (int i = 1; i < 10 && i < ret; i++)
        {
            printf("%02x ", report_buf[i]);
        }
        printf("...%s\n", COLOR_RESET);
    }
    
    /* Try to discover other report IDs by scanning */
    printf("%s│  [Report Discovery] Scanning for additional report IDs:%s\n", COLOR_MAGENTA, COLOR_RESET);
    int found_reports = 0;
    
    /* Only scan a subset of possible report IDs to avoid taking too long */
    unsigned char report_ids[] = {0x00, 0x02, 0x10, 0x12, 0x81, 0xA0, 0xF0, 0xF1, 0xF3, 0xF4, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA};
    
    for (size_t i = 0; i < sizeof(report_ids); i++)
    {
        /* Skip report IDs we've already tried */
        if (report_ids[i] == 0x01 || report_ids[i] == 0xA3 || 
            report_ids[i] == 0xF2 || report_ids[i] == MAC_REPORT_ID)
            continue;
            
        memset(report_buf, 0, sizeof(report_buf));
        report_buf[0] = report_ids[i];
        ret = hid_get_feature_report(dev, report_buf, sizeof(report_buf));
        
        if (ret > 0)
        {
            found_reports++;
            printf("%s│    [Report 0x%02x] Data: ", COLOR_MAGENTA, report_ids[i]);
            for (int j = 1; j < 8 && j < ret; j++)
            {
                printf("%02x ", report_buf[j]);
            }
            printf("...%s\n", COLOR_RESET);
        }
    }
    
    if (found_reports == 0)
    {
        printf("%s│    No additional report IDs found%s\n", COLOR_MAGENTA, COLOR_RESET);
    }
    
    printf("%s└───────────────────────────────────────────────%s\n", COLOR_MAGENTA, COLOR_RESET);
}

/**
 * Pairs a PlayStation controller with the specified MAC address
 */
void pair_device(hid_device *dev, const char *mac, size_t mac_len)
{
    unsigned char buf[8]; /* Buffer for the feature report */
    int ret;              /* Return value from HID operations */
    int is_ds4 = is_dualshock4(dev);

    /* Print controller type information */
    if (is_ds4) {
        printf("%s[INFO]%s Device identified as DualShock 4 controller\n", 
               COLOR_BLUE, COLOR_RESET);
    }

    /* Initialize the feature report buffer */
    memset(buf, 0, sizeof(buf));
    buf[0] = MAC_REPORT_ID; /* Set the report ID for MAC address */
    buf[1] = 0x0;           /* Reserved byte, must be zero */

    /* Validate MAC address format and convert to bytes */
    if ((mac_len != 12 && mac_len != 17) || !mac_to_bytes(mac, mac_len, buf + 2, sizeof(buf) - 2))
    {
        printf("%s[ERROR]%s Invalid MAC address format: %s\n", COLOR_RED, COLOR_RESET, mac);
        printf("        MAC address must be in format '%sAABBCCDDEEFF%s' or '%sAA:BB:CC:DD:EE:FF%s'\n",
               COLOR_CYAN, COLOR_RESET, COLOR_CYAN, COLOR_RESET);
        return;
    }

    /* Send the feature report to the controller */
    printf("%s[INFO]%s Attempting to set MAC address to %s%02x:%02x:%02x:%02x:%02x:%02x%s...\n",
           COLOR_BLUE, COLOR_RESET, COLOR_CYAN,
           buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], COLOR_RESET);
           
    /* For DualShock 4, we might need to try different approaches */
    if (is_ds4) {
        /* First try the standard method */
        ret = hid_send_feature_report(dev, buf, sizeof(buf));
        
        /* If that fails, try alternative methods for DualShock 4 */
        if (ret == -1) {
            printf("%s[INFO]%s Standard method failed for DualShock 4, trying alternatives...\n", 
                   COLOR_BLUE, COLOR_RESET);
                   
            /* Try with Bluetooth report ID 0x12 which is used by some DualShock 4 models */
            unsigned char alt_buf[8];
            memcpy(alt_buf, buf, sizeof(alt_buf));
            alt_buf[0] = 0x12;
            ret = hid_send_feature_report(dev, alt_buf, sizeof(alt_buf));
            
            if (ret == -1) {
                /* Try with report ID 0x81 */
                alt_buf[0] = 0x81;
                ret = hid_send_feature_report(dev, alt_buf, sizeof(alt_buf));
            }
        }
    } else {
        /* Standard method for other controllers */
        ret = hid_send_feature_report(dev, buf, sizeof(buf));
    }
    
    if (ret == -1)
    {
        printf("%s[ERROR]%s Failed to set MAC address. Error: %ls\n",
               COLOR_RED, COLOR_RESET, hid_error(dev));
    }
    else
    {
        printf("%s[SUCCESS]%s Set MAC address to %s%02x:%02x:%02x:%02x:%02x:%02x%s\n",
               COLOR_GREEN, COLOR_RESET, COLOR_CYAN,
               buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], COLOR_RESET);
        
        /* Get detailed device information when pairing is successful */
        dump_device_info(dev);
    }
}

/**
 * Displays the currently paired MAC address of the controller
 */
void show_pairing(hid_device *dev)
{
    unsigned char buf[8]; /* Buffer for the feature report */
    int ret;              /* Return value from HID operations */
    int is_ds4 = is_dualshock4(dev);

    /* Print controller type information */
    if (is_ds4) {
        printf("%s[INFO]%s Device identified as DualShock 4 controller\n", 
               COLOR_BLUE, COLOR_RESET);
    }

    /* Initialize the feature report buffer */
    memset(buf, 0, sizeof(buf));
    buf[0] = MAC_REPORT_ID; /* Set the report ID for MAC address */
    buf[1] = 0x0;           /* Reserved byte, must be zero */

    /* Get the current MAC address from the controller */
    printf("%s[INFO]%s Retrieving current MAC address from controller...\n", COLOR_BLUE, COLOR_RESET);
    
    /* For DualShock 4, we might need to try different approaches */
    if (is_ds4) {
        /* First try the standard method */
        ret = hid_get_feature_report(dev, buf, sizeof(buf));
        
        /* If that fails, try alternative methods for DualShock 4 */
        if (ret < 8) {
            printf("%s[INFO]%s Standard method failed for DualShock 4, trying alternatives...\n", 
                   COLOR_BLUE, COLOR_RESET);
                   
            /* Try with Bluetooth report ID 0x12 which is used by some DualShock 4 models */
            memset(buf, 0, sizeof(buf));
            buf[0] = 0x12;
            ret = hid_get_feature_report(dev, buf, sizeof(buf));
            
            if (ret < 8) {
                /* Try with report ID 0x81 */
                memset(buf, 0, sizeof(buf));
                buf[0] = 0x81;
                ret = hid_get_feature_report(dev, buf, sizeof(buf));
            }
        }
    } else {
        /* Standard method for other controllers */
        ret = hid_get_feature_report(dev, buf, sizeof(buf));
    }
    
    if (ret < 8)
    {
        printf("%s[ERROR]%s Failed to read MAC address. Error: %ls\n",
               COLOR_RED, COLOR_RESET, hid_error(dev));
        return;
    }

    /* Print the MAC address in standard format XX:XX:XX:XX:XX:XX */
    printf("%s[INFO]%s Current controller MAC address: %s%02x:%02x:%02x:%02x:%02x:%02x%s\n",
           COLOR_BLUE, COLOR_RESET, COLOR_CYAN,
           buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], COLOR_RESET);
           
    /* Ask if user wants to see detailed device information */
    char response[10];
    printf("%s[PROMPT]%s Would you like to see detailed device information? (y/n): ", COLOR_MAGENTA, COLOR_RESET);
    if (fgets(response, sizeof(response), stdin) != NULL)
    {
        if (response[0] == 'y' || response[0] == 'Y')
        {
            dump_device_info(dev);
        }
    }
}
