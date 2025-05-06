/* Copyright (c) 2014 John Schember <john@nachtimwald.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE
 */

/**
 * SixAxis Pairer - A utility for pairing PlayStation controllers with custom MAC addresses
 *
 * This program allows users to pair PlayStation SixAxis and Move Motion controllers
 * with a custom MAC address or display the currently paired MAC address.
 */

#include <stdlib.h> /* For general utilities */
#include <stdio.h>  /* For input/output functions */
#include <string.h> /* For string manipulation functions */

#include <hidapi.h> /* For USB HID device communication */

/* Sony PlayStation vendor ID */
static const unsigned short VENDOR = 0x054c;

/* Product IDs for supported PlayStation controllers */
static const unsigned short PRODUCT[] = {
    0x0268, /* PlayStation 3 SixAxis controller */
    0x042f  /* PlayStation Move Motion controller */
};

/*
 * MAC address report ID for controller pairing
 * 0xf5   == (0x03f5 & ~(3 << 8))
 * 0x03f5 == (0xf5 | (3 << 8))
 * HIDAPI will automatically add (3 << 8) to the report id.
 * Other tools for setting the report id use hid libraries which
 * don't automatically do this.
 */
static const unsigned char MAC_REPORT_ID = 0xf5;

/**
 * Converts a hexadecimal character to its 4-bit numeric value (nibble)
 *
 * @param c The character to convert ('0'-'9', 'a'-'f', or 'A'-'F')
 * @return The numeric value (0-15) or 255 if the character is invalid
 */
static unsigned char char_to_nible(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0'; /* Convert ASCII digit to numeric value */
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10; /* Convert lowercase hex to numeric value */
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10; /* Convert uppercase hex to numeric value */
    return 255;              /* Invalid character */
}

/**
 * Converts a MAC address string to byte array
 * Handles both formats: "AABBCCDDEEFF" and "AA:BB:CC:DD:EE:FF"
 *
 * @param in Input MAC address string
 * @param in_len Length of input string
 * @param out Output byte array to store the converted MAC
 * @param out_len Length of output array
 * @return 1 on success, 0 on failure
 */
static int mac_to_bytes(const char *in, size_t in_len, unsigned char *out, size_t out_len)
{
    const char *p;
    size_t i = 0;

    /* Validate input parameters */
    if (in == NULL || out == NULL || in_len == 0 || out_len == 0)
        return 0;

    /* Initialize output buffer */
    memset(out, 0, out_len);

    /* Parse the MAC address string */
    p = in;
    while (p + 1 < in + in_len && i < out_len)
    {
        /* Skip colons in MAC address format AA:BB:CC:DD:EE:FF */
        if (*p == ':')
        {
            p++;
            continue;
        }

        /* Verify both characters are valid hex digits */
        if (!isxdigit(*p) || !isxdigit(*(p + 1)))
        {
            return 0;
        }

        /* Convert two hex characters to one byte */
        out[i] = (char_to_nible(*p) << 4) | (char_to_nible(*(p + 1) & 0xff));
        i++;
        p += 2;
    }

    /* Ensure we processed the entire input string */
    if (p < in + in_len)
        return 0;
    return 1;
}

/**
 * Pairs a PlayStation controller with the specified MAC address
 *
 * @param dev Handle to the HID device
 * @param mac MAC address string to pair with
 * @param mac_len Length of the MAC address string
 */
static void pair_device(hid_device *dev, const char *mac, size_t mac_len)
{
    unsigned char buf[8]; /* Buffer for the feature report */
    int ret;              /* Return value from HID operations */

    /* Initialize the feature report buffer */
    memset(buf, 0, sizeof(buf));
    buf[0] = MAC_REPORT_ID; /* Set the report ID for MAC address */
    buf[1] = 0x0;           /* Reserved byte, must be zero */

    /* Validate MAC address format and convert to bytes */
    if ((mac_len != 12 && mac_len != 17) || !mac_to_bytes(mac, mac_len, buf + 2, sizeof(buf) - 2))
    {
        printf("Invalid MAC address format: %s\n", mac);
        printf("MAC address must be in format 'AABBCCDDEEFF' or 'AA:BB:CC:DD:EE:FF'\n");
        return;
    }

    /* Send the feature report to the controller */
    printf("Attempting to set MAC address to %02x:%02x:%02x:%02x:%02x:%02x...\n",
           buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    ret = hid_send_feature_report(dev, buf, sizeof(buf));
    if (ret == -1)
    {
        printf("Failed to set MAC address. Error: %ls\n", hid_error(dev));
    }
    else
    {
        printf("Successfully set MAC address to %02x:%02x:%02x:%02x:%02x:%02x\n",
               buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    }
}

/**
 * Displays the currently paired MAC address of the controller
 *
 * @param dev Handle to the HID device
 */
static void show_pairing(hid_device *dev)
{
    unsigned char buf[8]; /* Buffer for the feature report */
    int ret;              /* Return value from HID operations */

    /* Initialize the feature report buffer */
    memset(buf, 0, sizeof(buf));
    buf[0] = MAC_REPORT_ID; /* Set the report ID for MAC address */
    buf[1] = 0x0;           /* Reserved byte, must be zero */

    /* Get the current MAC address from the controller */
    printf("Retrieving current MAC address from controller...\n");
    ret = hid_get_feature_report(dev, buf, sizeof(buf));
    if (ret < 8)
    {
        printf("Failed to read MAC address. Error: %ls\n", hid_error(dev));
        return;
    }

    /* Print the MAC address in standard format XX:XX:XX:XX:XX:XX */
    printf("Current controller MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
}

/**
 * Main function - Entry point of the program
 *
 * Usage:
 *   sixaxispairer         - Shows the current MAC address
 *   sixaxispairer [mac]   - Sets a new MAC address
 *   sixaxispairer -l      - List all connected Sony USB devices
 *   sixaxispairer -a      - List all connected USB devices (not just Sony)
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return 0 on success, 1 on failure
 */
int main(int argc, char **argv)
{
    hid_device *dev; /* Handle to the HID device */

    /* Initialize the HID API */
    if (hid_init() != 0)
    {
        fprintf(stderr, "Failed to initialize HID API\n");
        return 1;
    }

    /* Check command line arguments and show usage if needed */
    if ((argc != 1 && argc != 2) ||
        (argc == 2 && (strncmp(argv[1], "-h", 2) == 0 || strncmp(argv[1], "--help", 6) == 0)))
    {
        printf("Usage:\n");
        printf("\t%s         - Show current controller MAC address\n", argv[0]);
        printf("\t%s [mac]   - Set controller MAC address (format: AABBCCDDEEFF or AA:BB:CC:DD:EE:FF)\n", argv[0]);
        printf("\t%s -l      - List all connected Sony USB devices\n", argv[0]);
        printf("\t%s -a      - List all connected USB devices (not just Sony)\n", argv[0]);
        printf("\t%s -h      - Show this help message\n", argv[0]);
        return 0;
    }

    /* Check if user wants to list devices */
    if (argc == 2 && (strncmp(argv[1], "-l", 2) == 0 || strncmp(argv[1], "-a", 2) == 0))
    {
        struct hid_device_info *devs, *cur_dev;
        int found_devices = 0;
        int list_all = (strncmp(argv[1], "-a", 2) == 0);

        if (list_all) {
            printf("Listing all connected USB devices:\n");
            devs = hid_enumerate(0, 0); /* Enumerate all USB HID devices */
        } else {
            printf("Listing all connected Sony USB devices:\n");
            devs = hid_enumerate(VENDOR, 0); /* Enumerate all devices with Sony vendor ID */
        }
        cur_dev = devs;

        while (cur_dev)
        {
            /* When listing all devices, show everything. When listing Sony devices, filter by vendor ID */
            if (list_all || cur_dev->vendor_id == VENDOR)
            {
                found_devices++;
                printf("Device %d:\n", found_devices);
                printf("  Vendor ID:       0x%04x%s\n", cur_dev->vendor_id, 
                       (cur_dev->vendor_id == VENDOR) ? " (Sony)" : "");
                printf("  Product ID:      0x%04x\n", cur_dev->product_id);
                printf("  Manufacturer:    %ls\n", cur_dev->manufacturer_string ? cur_dev->manufacturer_string : L"(Unknown)");
                printf("  Product:         %ls\n", cur_dev->product_string ? cur_dev->product_string : L"(Unknown)");
                printf("  Serial Number:   %ls\n", cur_dev->serial_number ? cur_dev->serial_number : L"(None)");
                printf("  Interface:       %d\n", cur_dev->interface_number);
                printf("  Path:            %s\n", cur_dev->path);

                /* Check if this is a supported controller */
                if (cur_dev->vendor_id == VENDOR) {
                    for (size_t i = 0; i < sizeof(PRODUCT) / sizeof(*PRODUCT); i++)
                    {
                        if (cur_dev->product_id == PRODUCT[i])
                        {
                            printf("  ** This is a supported PlayStation controller **\n");
                            break;
                        }
                    }
                }
                printf("\n");
            }
            cur_dev = cur_dev->next;
        }

        if (found_devices == 0)
        {
            if (list_all) {
                printf("No USB HID devices found on the system.\n");
            } else {
                printf("No Sony USB devices found.\n");
                printf("Use '%s -a' to list all USB devices on the system.\n", argv[0]);
            }
        }
        else
        {
            if (list_all) {
                printf("Found %d USB device(s).\n", found_devices);
            } else {
                printf("Found %d Sony USB device(s).\n", found_devices);
                printf("Use '%s -a' to list all USB devices on the system.\n", argv[0]);
            }
        }

        hid_free_enumeration(devs);
        hid_exit();
        return 0;
    }

    /* Try to open a connection to any supported PlayStation controller */
    dev = NULL;
    printf("Searching for PlayStation controllers...\n");
    for (size_t i = 0; i < sizeof(PRODUCT) / sizeof(*PRODUCT); i++)
    {
        printf("Trying to connect to %s (Vendor ID: 0x%04x, Product ID: 0x%04x)...\n",
               (PRODUCT[i] == 0x0268) ? "SixAxis Controller" : "Move Motion Controller",
               VENDOR, PRODUCT[i]);
        dev = hid_open(VENDOR, PRODUCT[i], NULL);
        if (dev != NULL)
        {
            printf("Successfully connected to %s\n",
                   (PRODUCT[i] == 0x0268) ? "SixAxis Controller" : "Move Motion Controller");
            break;
        }
    }

    /* Check if a controller was found */
    if (dev == NULL)
    {
        struct hid_device_info *devs, *cur_dev;
        int found_sony_devices = 0;

        fprintf(stderr, "Could not find any PlayStation controllers (Vendor ID: 0x%04x, Product IDs: 0x%04x, 0x%04x)\n",
                VENDOR, PRODUCT[0], PRODUCT[1]);

        /* Search for any Sony devices that might be connected */
        fprintf(stderr, "Searching for other Sony devices...\n");
        devs = hid_enumerate(VENDOR, 0); /* Enumerate all devices with Sony vendor ID */
        cur_dev = devs;

        while (cur_dev)
        {
            if (cur_dev->vendor_id == VENDOR)
            {
                found_sony_devices++;
                fprintf(stderr, "Found Sony device: Product ID: 0x%04x - %ls %ls (Serial: %ls)\n",
                        cur_dev->product_id,
                        cur_dev->manufacturer_string ? cur_dev->manufacturer_string : L"(Unknown manufacturer)",
                        cur_dev->product_string ? cur_dev->product_string : L"(Unknown product)",
                        cur_dev->serial_number ? cur_dev->serial_number : L"(No serial number)");
            }
            cur_dev = cur_dev->next;
        }

        if (found_sony_devices == 0)
        {
            char response[10];
            fprintf(stderr, "No Sony devices found. Make sure the controller is connected via USB and powered on\n");
            fprintf(stderr, "Would you like to list all USB devices? (y/n): ");
            
            if (fgets(response, sizeof(response), stdin) != NULL)
            {
                if (response[0] == 'y' || response[0] == 'Y')
                {
                    /* List all USB devices regardless of vendor */
                    printf("\nListing all connected USB devices:\n");
                    hid_free_enumeration(devs);  /* Free the Sony-only enumeration */
                    
                    devs = hid_enumerate(0, 0);  /* Enumerate all USB HID devices */
                    cur_dev = devs;
                    int all_devices = 0;
                    
                    while (cur_dev)
                    {
                        all_devices++;
                        printf("Device %d:\n", all_devices);
                        printf("  Vendor ID:       0x%04x\n", cur_dev->vendor_id);
                        printf("  Product ID:      0x%04x\n", cur_dev->product_id);
                        printf("  Manufacturer:    %ls\n", cur_dev->manufacturer_string ? cur_dev->manufacturer_string : L"(Unknown)");
                        printf("  Product:         %ls\n", cur_dev->product_string ? cur_dev->product_string : L"(Unknown)");
                        printf("  Serial Number:   %ls\n", cur_dev->serial_number ? cur_dev->serial_number : L"(None)");
                        printf("  Interface:       %d\n", cur_dev->interface_number);
                        printf("  Path:            %s\n\n", cur_dev->path);
                        
                        cur_dev = cur_dev->next;
                    }
                    
                    if (all_devices == 0)
                    {
                        printf("No USB HID devices found on the system.\n");
                    }
                    else
                    {
                        printf("Found %d USB HID device(s).\n", all_devices);
                        printf("If your controller is in the list above but not recognized,\n");
                        printf("it might be in a different mode or require special drivers.\n");
                    }
                }
            }
        }
        else
        {
            fprintf(stderr, "Found %d Sony device(s), but none match the supported PlayStation controllers\n", found_sony_devices);
            fprintf(stderr, "If your controller is listed above, it might be a different model or in an unsupported mode\n");
        }

        hid_free_enumeration(devs);
        return 0;
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

    /* Clean up the HID API */
    hid_exit();
    return 0;
}