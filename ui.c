/**
 * ui.c - User interface functions
 * 
 * Implementation of functions for handling user interface and command-line interactions
 */

#include "ui.h"
#include "controller_connection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hidapi/hidapi.h>

/**
 * Displays the program usage information
 */
void show_usage(const char *program_name)
{
    printf("%s%s=== PlayStation Controller Pairer Usage ===%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
    printf("%s\t%s         - Show current controller MAC address%s\n", COLOR_WHITE, program_name, COLOR_RESET);
    printf("%s\t%s %s[mac]%s   - Set controller MAC address (format: AABBCCDDEEFF or AA:BB:CC:DD:EE:FF)%s\n",
           COLOR_WHITE, program_name, COLOR_CYAN, COLOR_WHITE, COLOR_RESET);
    printf("%s\t%s %s-l%s      - List all connected Sony USB devices%s\n",
           COLOR_WHITE, program_name, COLOR_CYAN, COLOR_WHITE, COLOR_RESET);
    printf("%s\t%s %s-a%s      - List all connected USB devices (not just Sony)%s\n",
           COLOR_WHITE, program_name, COLOR_CYAN, COLOR_WHITE, COLOR_RESET);
    printf("%s\t%s %s-d%s      - Dump all available information from connected controller%s\n",
           COLOR_WHITE, program_name, COLOR_CYAN, COLOR_WHITE, COLOR_RESET);
    printf("%s\t%s %s-h%s      - Show this help message%s\n",
           COLOR_WHITE, program_name, COLOR_CYAN, COLOR_WHITE, COLOR_RESET);
}

/**
 * Lists all connected USB devices
 */
int list_devices(int list_all)
{
    struct hid_device_info *devs, *cur_dev;
    int found_devices = 0;
    
    if (list_all)
    {
        printf("%s%s=== Listing all connected USB devices ===%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
        devs = hid_enumerate(0, 0); /* Enumerate all USB HID devices */
    }
    else
    {
        printf("%s%s=== Listing all connected Sony USB devices ===%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
        devs = hid_enumerate(VENDOR_SONY, 0); /* Enumerate all devices with Sony vendor ID */
    }
    cur_dev = devs;

    while (cur_dev)
    {
        /* When listing all devices, show everything. When listing Sony devices, filter by vendor ID */
        if (list_all || cur_dev->vendor_id == VENDOR_SONY)
        {
            found_devices++;
            printf("%s%s┌─ Device %d ─────────────────────────────────────%s\n", COLOR_BOLD, COLOR_MAGENTA, found_devices, COLOR_RESET);
            printf("%s│  Vendor ID:       0x%04x%s%s\n", COLOR_MAGENTA, cur_dev->vendor_id,
                   (cur_dev->vendor_id == VENDOR_SONY) ? COLOR_YELLOW " (Sony)" COLOR_RESET : "", COLOR_RESET);
            printf("%s│  Product ID:      0x%04x%s\n", COLOR_MAGENTA, cur_dev->product_id, COLOR_RESET);
            printf("%s│  Manufacturer:    %ls%s\n", COLOR_MAGENTA,
                   cur_dev->manufacturer_string ? cur_dev->manufacturer_string : L"(Unknown)", COLOR_RESET);
            printf("%s│  Product:         %ls%s\n", COLOR_MAGENTA,
                   cur_dev->product_string ? cur_dev->product_string : L"(Unknown)", COLOR_RESET);
            printf("%s│  Serial Number:   %ls%s\n", COLOR_MAGENTA,
                   cur_dev->serial_number ? cur_dev->serial_number : L"(None)", COLOR_RESET);
            printf("%s│  Interface:       %d%s\n", COLOR_MAGENTA, cur_dev->interface_number, COLOR_RESET);
            printf("%s│  Path:            %s%s\n", COLOR_MAGENTA, cur_dev->path, COLOR_RESET);

            /* Check if this is a supported controller */
            if (cur_dev->vendor_id == VENDOR_SONY)
            {
                if (is_supported_controller(cur_dev->vendor_id, cur_dev->product_id))
                {
                    printf("%s│  %s** This is a supported PlayStation controller **%s\n",
                           COLOR_MAGENTA, COLOR_GREEN, COLOR_RESET);
                }
            }
            printf("%s└───────────────────────────────────────────────%s\n\n", COLOR_MAGENTA, COLOR_RESET);
        }
        cur_dev = cur_dev->next;
    }

    if (found_devices == 0)
    {
        if (list_all)
        {
            printf("%s[INFO]%s No USB HID devices found on the system.\n", COLOR_BLUE, COLOR_RESET);
        }
        else
        {
            printf("%s[INFO]%s No Sony USB devices found.\n", COLOR_BLUE, COLOR_RESET);
        }
    }
    else
    {
        if (list_all)
        {
            printf("%s[INFO]%s Found %s%d%s USB device(s).\n",
                   COLOR_BLUE, COLOR_RESET, COLOR_YELLOW, found_devices, COLOR_RESET);
        }
        else
        {
            printf("%s[INFO]%s Found %s%d%s Sony USB device(s).\n",
                   COLOR_BLUE, COLOR_RESET, COLOR_YELLOW, found_devices, COLOR_RESET);
        }
    }

    hid_free_enumeration(devs);
    return 0;
}

/**
 * Dumps information about connected controllers
 */
int dump_controller_info(void)
{
    hid_device *dev = NULL;
    
    printf("%s%s=== Dumping PlayStation Controller Information ===%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
    
    /* Try to open a connection to any supported PlayStation controller */
    printf("%s[INFO]%s Searching for PlayStation controllers...\n", COLOR_BLUE, COLOR_RESET);
    
    /* Find all controllers */
    controller_info_t *controllers[MAX_CONTROLLERS];
    int controller_count = find_controllers(controllers, MAX_CONTROLLERS);
    
    if (controller_count == 0)
    {
        printf("%s[ERROR]%s No PlayStation controllers found.\n", COLOR_RED, COLOR_RESET);
        return 1;
    }
    
    /* Display found controllers */
    for (int i = 0; i < controller_count; i++)
    {
        display_controller_info(controllers[i], i + 1);
    }
    
    /* Connect to the first controller */
    dev = connect_to_controller(controllers[0]);
    
    if (dev != NULL)
    {
        /* Dump all available device information */
        dump_device_info(dev);
        
        /* Clean up */
        hid_close(dev);
    }
    else
    {
        printf("%s[ERROR]%s Failed to connect to controller.\n", COLOR_RED, COLOR_RESET);
    }
    
    /* Free controller info structures */
    for (int i = 0; i < controller_count; i++)
    {
        free_controller_info(controllers[i]);
    }
    
    return (dev != NULL) ? 0 : 1;
}

/**
 * Displays information about a controller
 */
void display_controller_info(controller_info_t *controller, int index)
{
    const char* device_name = get_controller_name(controller->product_id);
    
    printf("%s%s┌─ Controller %d ─────────────────────────────────%s\n", 
           COLOR_BOLD, COLOR_MAGENTA, index, COLOR_RESET);
    printf("%s│  Type:            %s%s%s\n", 
           COLOR_MAGENTA, COLOR_YELLOW, device_name, COLOR_RESET);
    printf("%s│  Vendor ID:       0x%04x (Sony)\n", COLOR_MAGENTA, controller->vendor_id);
    printf("%s│  Product ID:      0x%04x\n", COLOR_MAGENTA, controller->product_id);
    printf("%s│  Manufacturer:    %ls\n", COLOR_MAGENTA,
           controller->manufacturer_string ? controller->manufacturer_string : L"(Unknown)");
    printf("%s│  Product:         %ls\n", COLOR_MAGENTA,
           controller->product_string ? controller->product_string : L"(Unknown)");
    printf("%s│  Interface:       %d%s\n", COLOR_MAGENTA, controller->interface_number,
           (controller->product_id == PRODUCT_DS4 && controller->interface_number == DS4_HID_INTERFACE) ? 
           COLOR_GREEN " (Preferred)" COLOR_RESET : "");
    printf("%s│  Path:            %s\n", COLOR_MAGENTA, controller->path);
    printf("%s└───────────────────────────────────────────────%s\n\n", COLOR_MAGENTA, COLOR_RESET);
}

/**
 * Prompts the user to select a controller from a list
 */
int select_controller(controller_info_t *controllers[], int controller_count)
{
    /* If only one controller, select it automatically */
    if (controller_count == 1)
    {
        printf("%s[INFO]%s One controller found, automatically selecting it.\n", COLOR_BLUE, COLOR_RESET);
        return 0;
    }
    
    /* If multiple controllers, let the user select one */
    char input[10];
    int valid_selection = 0;
    int selected_index = -1;
    
    printf("%s[PROMPT]%s Multiple controllers found. Please select one (1-%d): ", 
           COLOR_MAGENTA, COLOR_RESET, controller_count);
    
    while (!valid_selection)
    {
        if (fgets(input, sizeof(input), stdin) != NULL)
        {
            int selection = atoi(input);
            if (selection >= 1 && selection <= controller_count)
            {
                selected_index = selection - 1;
                valid_selection = 1;
            }
            else
            {
                printf("%s[ERROR]%s Invalid selection. Please enter a number between 1 and %d: ", 
                       COLOR_RED, COLOR_RESET, controller_count);
            }
        }
    }
    
    return selected_index;
}