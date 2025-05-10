/**
 * PlayStation Controller Pairer - A utility for pairing PlayStation controllers with custom MAC addresses
 *
 * This program allows users to pair PlayStation controllers (SixAxis, Move Motion, and DualShock 4)
 * with a custom MAC address or display the currently paired MAC address.
 *
 * Copyright (c) 2014 John Schember <john@nachtimwald.com>
 * Licensed under MIT License
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <hidapi/hidapi.h>

#include "mac_utils.h"
#include "controller_info.h"
#include "controller_connection.h"
#include "ui.h"

/**
 * Main function - Entry point of the program
 *
 * Usage:
 *   sixaxispairer         - Shows the current MAC address
 *   sixaxispairer [mac]   - Sets a new MAC address
 *   sixaxispairer -l      - List all connected Sony USB devices
 *   sixaxispairer -a      - List all connected USB devices (not just Sony)
 *   sixaxispairer -d      - Dump all available information from connected controller
 *   sixaxispairer -h      - Show help message
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return 0 on success, 1 on failure
 */
int main(int argc, char **argv)
{
    hid_device *dev = NULL;
    int result = 0;

    /* Initialize the HID API */
    if (hid_init() != 0)
    {
        fprintf(stderr, "%s[ERROR]%s Failed to initialize HID API\n", COLOR_RED, COLOR_RESET);
        return 1;
    }

    /* Check command line arguments and show usage if needed */
    if ((argc != 1 && argc != 2) ||
        (argc == 2 && (strncmp(argv[1], "-h", 2) == 0 || strncmp(argv[1], "--help", 6) == 0)))
    {
        show_usage(argv[0]);
        hid_exit();
        return 0;
    }

    /* Check if user wants to list devices or dump device info */
    if (argc == 2 && (strncmp(argv[1], "-l", 2) == 0 || strncmp(argv[1], "-a", 2) == 0 || strncmp(argv[1], "-d", 2) == 0))
    {
        if (strncmp(argv[1], "-l", 2) == 0)
        {
            result = list_devices(0); /* List only Sony devices */
        }
        else if (strncmp(argv[1], "-a", 2) == 0)
        {
            result = list_devices(1); /* List all USB devices */
        }
        else /* -d */
        {
            result = dump_controller_info();
        }
        
        hid_exit();
        return result;
    }

    /* Find all supported controllers */
    printf("%s[INFO]%s Searching for PlayStation controllers...\n", COLOR_BLUE, COLOR_RESET);
    
    controller_info_t *controllers[MAX_CONTROLLERS];
    int controller_count = find_controllers(controllers, MAX_CONTROLLERS);
    
    /* Check if we found any controllers */
    if (controller_count == 0)
    {
        printf("%s[ERROR]%s No compatible PlayStation controllers found.\n", COLOR_RED, COLOR_RESET);
        printf("         Make sure your controller is connected via USB and powered on.\n");
        printf("         Try running with sudo if you have permission issues.\n");
        hid_exit();
        return 1;
    }
    
    /* Display found controllers */
    printf("\n%s%s=== Available PlayStation Controllers ===%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
    for (int i = 0; i < controller_count; i++)
    {
        display_controller_info(controllers[i], i + 1);
    }
    
    /* Let the user select a controller if multiple are found */
    int selected_index = select_controller(controllers, controller_count);
    if (selected_index < 0)
    {
        printf("%s[ERROR]%s Invalid controller selection.\n", COLOR_RED, COLOR_RESET);
        
        /* Free controller info structures */
        for (int i = 0; i < controller_count; i++)
        {
            free_controller_info(controllers[i]);
        }
        
        hid_exit();
        return 1;
    }
    
    /* Get the selected controller */
    controller_info_t *selected = controllers[selected_index];
    const char* device_name = get_controller_name(selected->product_id);
    
    printf("%s[INFO]%s Selected controller: %s%s%s (Interface: %d)\n",
           COLOR_BLUE, COLOR_RESET, COLOR_YELLOW, device_name, COLOR_RESET,
           selected->interface_number);
    
    /* Try to connect to the selected controller */
    dev = connect_to_controller(selected);

    /* Check if connection was successful */
    if (dev == NULL)
    {
        printf("%s[ERROR]%s Failed to connect to the selected controller.\n", COLOR_RED, COLOR_RESET);
        printf("         This could be due to permission issues or the device being in use by another application.\n");
        printf("         Try running the program with sudo or check if the device is being used by another application.\n");
        
        int is_mac_address_provided = (argc == 2 && argv[1][0] != '-');
        if (is_mac_address_provided)
        {
            printf("%s[INFO]%s MAC address provided but couldn't connect to the controller.\n", COLOR_BLUE, COLOR_RESET);
            printf("         Please make sure the controller is properly connected and try again.\n");
        }
        
        /* Free controller info structures */
        for (int i = 0; i < controller_count; i++)
        {
            free_controller_info(controllers[i]);
        }
        
        hid_exit();
        return 1;
    }

    /* Either pair with a new MAC or show the current pairing */
    if (argc == 2)
    {
        pair_device(dev, argv[1], strlen(argv[1])); /* Set new MAC address */
    }
    else
    {
        show_pairing(dev); /* Show current MAC address */
    }

    /* Clean up and close the connection */
    hid_close(dev);
    
    /* Free controller info structures */
    for (int i = 0; i < controller_count; i++)
    {
        free_controller_info(controllers[i]);
    }

    /* Clean up the HID API */
    hid_exit();
    return 0;
}