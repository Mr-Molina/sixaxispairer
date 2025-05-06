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
#include <ctype.h>  /* For isxdigit function */

#include <hidapi.h> /* For USB HID device communication */

/* ANSI color codes for terminal output */
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_WHITE   "\x1b[37m"
#define COLOR_BOLD    "\x1b[1m"

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
        printf("%s[ERROR]%s Invalid MAC address format: %s\n", COLOR_RED, COLOR_RESET, mac);
        printf("        MAC address must be in format '%sAABBCCDDEEFF%s' or '%sAA:BB:CC:DD:EE:FF%s'\n", 
               COLOR_CYAN, COLOR_RESET, COLOR_CYAN, COLOR_RESET);
        return;
    }

    /* Send the feature report to the controller */
    printf("%s[INFO]%s Attempting to set MAC address to %s%02x:%02x:%02x:%02x:%02x:%02x%s...\n",
           COLOR_BLUE, COLOR_RESET, COLOR_CYAN,
           buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], COLOR_RESET);
    ret = hid_send_feature_report(dev, buf, sizeof(buf));
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
    printf("%s[INFO]%s Retrieving current MAC address from controller...\n", COLOR_BLUE, COLOR_RESET);
    ret = hid_get_feature_report(dev, buf, sizeof(buf));
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
        fprintf(stderr, "%s[ERROR]%s Failed to initialize HID API\n", COLOR_RED, COLOR_RESET);
        return 1;
    }

    /* Check command line arguments and show usage if needed */
    if ((argc != 1 && argc != 2) ||
        (argc == 2 && (strncmp(argv[1], "-h", 2) == 0 || strncmp(argv[1], "--help", 6) == 0)))
    {
        printf("%s%s=== SixAxis Pairer Usage ===%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
        printf("%s\t%s         - Show current controller MAC address%s\n", COLOR_WHITE, argv[0], COLOR_RESET);
        printf("%s\t%s %s[mac]%s   - Set controller MAC address (format: AABBCCDDEEFF or AA:BB:CC:DD:EE:FF)%s\n", 
               COLOR_WHITE, argv[0], COLOR_CYAN, COLOR_WHITE, COLOR_RESET);
        printf("%s\t%s %s-l%s      - List all connected Sony USB devices%s\n", 
               COLOR_WHITE, argv[0], COLOR_CYAN, COLOR_WHITE, COLOR_RESET);
        printf("%s\t%s %s-a%s      - List all connected USB devices (not just Sony)%s\n", 
               COLOR_WHITE, argv[0], COLOR_CYAN, COLOR_WHITE, COLOR_RESET);
        printf("%s\t%s %s-h%s      - Show this help message%s\n", 
               COLOR_WHITE, argv[0], COLOR_CYAN, COLOR_WHITE, COLOR_RESET);
        return 0;
    }

    /* Check if user wants to list devices */
    if (argc == 2 && (strncmp(argv[1], "-l", 2) == 0 || strncmp(argv[1], "-a", 2) == 0))
    {
        struct hid_device_info *devs, *cur_dev;
        int found_devices = 0;
        int list_all = (strncmp(argv[1], "-a", 2) == 0);

        if (list_all)
        {
            printf("%s%s=== Listing all connected USB devices ===%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
            devs = hid_enumerate(0, 0); /* Enumerate all USB HID devices */
        }
        else
        {
            printf("%s%s=== Listing all connected Sony USB devices ===%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
            devs = hid_enumerate(VENDOR, 0); /* Enumerate all devices with Sony vendor ID */
        }
        cur_dev = devs;

        while (cur_dev)
        {
            /* When listing all devices, show everything. When listing Sony devices, filter by vendor ID */
            if (list_all || cur_dev->vendor_id == VENDOR)
            {
                found_devices++;
                printf("%s%s┌─ Device %d ─────────────────────────────────────%s\n", COLOR_BOLD, COLOR_MAGENTA, found_devices, COLOR_RESET);
                printf("%s│  Vendor ID:       0x%04x%s%s\n", COLOR_MAGENTA, cur_dev->vendor_id,
                       (cur_dev->vendor_id == VENDOR) ? COLOR_YELLOW " (Sony)" COLOR_RESET : "", COLOR_RESET);
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
                if (cur_dev->vendor_id == VENDOR)
                {
                    for (size_t i = 0; i < sizeof(PRODUCT) / sizeof(*PRODUCT); i++)
                    {
                        if (cur_dev->product_id == PRODUCT[i])
                        {
                            printf("%s│  %s** This is a supported PlayStation controller **%s\n", 
                                   COLOR_MAGENTA, COLOR_GREEN, COLOR_RESET);
                            break;
                        }
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
                printf("       Use '%s%s -a%s' to list all USB devices on the system.\n", 
                       COLOR_CYAN, argv[0], COLOR_RESET);
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
                printf("       Use '%s%s -a%s' to list all USB devices on the system.\n", 
                       COLOR_CYAN, argv[0], COLOR_RESET);
            }
        }

        hid_free_enumeration(devs);
        hid_exit();
        return 0;
    }

    /* Try to open a connection to any supported PlayStation controller */
    dev = NULL;
    printf("%s[INFO]%s Searching for PlayStation controllers...\n", COLOR_BLUE, COLOR_RESET);
    for (size_t i = 0; i < sizeof(PRODUCT) / sizeof(*PRODUCT); i++)
    {
        printf("%s[INFO]%s Trying to connect to %s%s%s (Vendor ID: %s0x%04x%s, Product ID: %s0x%04x%s)...\n",
               COLOR_BLUE, COLOR_RESET, COLOR_YELLOW,
               (PRODUCT[i] == 0x0268) ? "SixAxis Controller" : "Move Motion Controller",
               COLOR_RESET, COLOR_CYAN, VENDOR, COLOR_RESET, COLOR_CYAN, PRODUCT[i], COLOR_RESET);
        dev = hid_open(VENDOR, PRODUCT[i], NULL);
        if (dev != NULL)
        {
            printf("%s[SUCCESS]%s Connected to %s%s%s\n",
                   COLOR_GREEN, COLOR_RESET, COLOR_YELLOW,
                   (PRODUCT[i] == 0x0268) ? "SixAxis Controller" : "Move Motion Controller",
                   COLOR_RESET);
            break;
        }
    }

    /* Check if a controller was found */
    if (dev == NULL)
    {
        struct hid_device_info *devs, *cur_dev;
        int found_sony_devices = 0;

        fprintf(stderr, "%s[ERROR]%s Could not find any PlayStation controllers (Vendor ID: %s0x%04x%s, Product IDs: %s0x%04x%s, %s0x%04x%s)\n",
                COLOR_RED, COLOR_RESET, COLOR_CYAN, VENDOR, COLOR_RESET, 
                COLOR_CYAN, PRODUCT[0], COLOR_RESET, COLOR_CYAN, PRODUCT[1], COLOR_RESET);

        /* Search for any Sony devices that might be connected */
        fprintf(stderr, "%s[INFO]%s Searching for other Sony devices...\n", COLOR_BLUE, COLOR_RESET);
        devs = hid_enumerate(VENDOR, 0); /* Enumerate all devices with Sony vendor ID */
        cur_dev = devs;

        while (cur_dev)
        {
            if (cur_dev->vendor_id == VENDOR)
            {
                found_sony_devices++;
                fprintf(stderr, "%s[FOUND]%s Sony device: Product ID: %s0x%04x%s - %ls %ls (Serial: %ls)\n",
                        COLOR_YELLOW, COLOR_RESET, COLOR_CYAN, cur_dev->product_id, COLOR_RESET,
                        cur_dev->manufacturer_string ? cur_dev->manufacturer_string : L"(Unknown manufacturer)",
                        cur_dev->product_string ? cur_dev->product_string : L"(Unknown product)",
                        cur_dev->serial_number ? cur_dev->serial_number : L"(No serial number)");
            }
            cur_dev = cur_dev->next;
        }

        if (found_sony_devices == 0)
        {
            char response[10];
            fprintf(stderr, "%s[WARNING]%s No Sony devices found. Make sure the controller is connected via USB and powered on\n", 
                    COLOR_YELLOW, COLOR_RESET);
            fprintf(stderr, "%s[PROMPT]%s Would you like to list all USB devices? (y/n): ", COLOR_MAGENTA, COLOR_RESET);

            if (fgets(response, sizeof(response), stdin) != NULL)
            {
                if (response[0] == 'y' || response[0] == 'Y')
                {
                    /* List all USB devices regardless of vendor */
                    printf("\n%s%s=== Listing all connected USB devices ===%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
                    hid_free_enumeration(devs); /* Free the Sony-only enumeration */

                    devs = hid_enumerate(0, 0); /* Enumerate all USB HID devices */
                    cur_dev = devs;
                    int all_devices = 0;

                    while (cur_dev)
                    {
                        all_devices++;
                        printf("%s%s┌─ Device %d ─────────────────────────────────────%s\n", COLOR_BOLD, COLOR_MAGENTA, all_devices, COLOR_RESET);
                        printf("%s│  Vendor ID:       0x%04x%s\n", COLOR_MAGENTA, cur_dev->vendor_id, COLOR_RESET);
                        printf("%s│  Product ID:      0x%04x%s\n", COLOR_MAGENTA, cur_dev->product_id, COLOR_RESET);
                        printf("%s│  Manufacturer:    %ls%s\n", COLOR_MAGENTA, 
                               cur_dev->manufacturer_string ? cur_dev->manufacturer_string : L"(Unknown)", COLOR_RESET);
                        printf("%s│  Product:         %ls%s\n", COLOR_MAGENTA, 
                               cur_dev->product_string ? cur_dev->product_string : L"(Unknown)", COLOR_RESET);
                        printf("%s│  Serial Number:   %ls%s\n", COLOR_MAGENTA, 
                               cur_dev->serial_number ? cur_dev->serial_number : L"(None)", COLOR_RESET);
                        printf("%s│  Interface:       %d%s\n", COLOR_MAGENTA, cur_dev->interface_number, COLOR_RESET);
                        printf("%s│  Path:            %s%s\n", COLOR_MAGENTA, cur_dev->path, COLOR_RESET);
                        printf("%s└───────────────────────────────────────────────%s\n\n", COLOR_MAGENTA, COLOR_RESET);

                        cur_dev = cur_dev->next;
                    }

                    if (all_devices == 0)
                    {
                        printf("%s[INFO]%s No USB HID devices found on the system.\n", COLOR_BLUE, COLOR_RESET);
                    }
                    else
                    {
                        printf("%s[INFO]%s Found %s%d%s USB HID device(s).\n", 
                               COLOR_BLUE, COLOR_RESET, COLOR_YELLOW, all_devices, COLOR_RESET);
                        printf("       If your controller is in the list above but not recognized,\n");
                        printf("       it might be in a different mode or require special drivers.\n");
                    }
                }
            }
        }
        else
        {
            fprintf(stderr, "%s[WARNING]%s Found %s%d%s Sony device(s), but none match the supported PlayStation controllers\n", 
                    COLOR_YELLOW, COLOR_RESET, COLOR_CYAN, found_sony_devices, COLOR_RESET);
            fprintf(stderr, "          If your controller is listed above, it might be a different model or in an unsupported mode\n");
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