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

#include <stdlib.h>  /* For general utilities */
#include <stdio.h>   /* For input/output functions */
#include <string.h>  /* For string manipulation functions */

#include <hidapi.h>  /* For USB HID device communication */

/* Sony PlayStation vendor ID */
static const unsigned short VENDOR    = 0x054c;

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
        return c - '0';             /* Convert ASCII digit to numeric value */
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;        /* Convert lowercase hex to numeric value */
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;        /* Convert uppercase hex to numeric value */
    return 255;                     /* Invalid character */
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
    size_t      i=0;

    /* Validate input parameters */
    if (in == NULL || out == NULL || in_len == 0 || out_len == 0)
        return 0;

    /* Initialize output buffer */
    memset(out, 0, out_len);
    
    /* Parse the MAC address string */
    p = in;
    while (p+1 < in+in_len && i < out_len) {
        /* Skip colons in MAC address format AA:BB:CC:DD:EE:FF */
        if (*p == ':') {
            p++;
            continue;
        }

        /* Verify both characters are valid hex digits */
        if (!isxdigit(*p) || !isxdigit(*(p+1))) {
            return 0;
        }

        /* Convert two hex characters to one byte */
        out[i] = (char_to_nible(*p) << 4) | (char_to_nible(*(p+1) & 0xff));
        i++;
        p += 2;
    }

    /* Ensure we processed the entire input string */
    if (p < in+in_len)
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
    unsigned char buf[8];  /* Buffer for the feature report */
    int           ret;     /* Return value from HID operations */

    /* Initialize the feature report buffer */
    memset(buf, 0, sizeof(buf));
    buf[0] = MAC_REPORT_ID;  /* Set the report ID for MAC address */
    buf[1] = 0x0;            /* Reserved byte, must be zero */
    
    /* Validate MAC address format and convert to bytes */
    if ((mac_len != 12 && mac_len != 17) || !mac_to_bytes(mac, mac_len, buf+2, sizeof(buf)-2)) {
        printf("Invalid MAC address format: %s\n", mac);
        printf("MAC address must be in format 'AABBCCDDEEFF' or 'AA:BB:CC:DD:EE:FF'\n");
        return;
    }

    /* Send the feature report to the controller */
    printf("Attempting to set MAC address to %02x:%02x:%02x:%02x:%02x:%02x...\n", 
           buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    ret = hid_send_feature_report(dev, buf, sizeof(buf));
    if (ret == -1) {
        printf("Failed to set MAC address. Error: %ls\n", hid_error(dev));
    } else {
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
    unsigned char buf[8];  /* Buffer for the feature report */
    int           ret;     /* Return value from HID operations */

    /* Initialize the feature report buffer */
    memset(buf, 0, sizeof(buf));
    buf[0] = MAC_REPORT_ID;  /* Set the report ID for MAC address */
    buf[1] = 0x0;            /* Reserved byte, must be zero */

    /* Get the current MAC address from the controller */
    printf("Retrieving current MAC address from controller...\n");
    ret = hid_get_feature_report(dev, buf, sizeof(buf));
    if (ret < 8) {
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
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return 0 on success
 */
int main(int argc, char **argv)
{
    hid_device *dev;  /* Handle to the HID device */

    /* Check command line arguments and show usage if needed */
    if ((argc != 1 && argc != 2) ||
        (argc == 2 && (strncmp(argv[1], "-h", 2) == 0 || strncmp(argv[1], "--help", 6) == 0)))
    {
        printf("usage:\n\t%s [mac]\n", argv[0]);
        return 0;
    }

    /* Try to open a connection to any supported PlayStation controller */
    dev = NULL;
    printf("Searching for PlayStation controllers...\n");
    for (size_t i = 0; i < sizeof(PRODUCT) / sizeof(*PRODUCT); i++) {
        printf("Trying to connect to %s (Vendor ID: 0x%04x, Product ID: 0x%04x)...\n", 
               (PRODUCT[i] == 0x0268) ? "SixAxis Controller" : "Move Motion Controller", 
               VENDOR, PRODUCT[i]);
        dev = hid_open(VENDOR, PRODUCT[i], NULL);
        if (dev != NULL) {
            printf("Successfully connected to %s\n", 
                   (PRODUCT[i] == 0x0268) ? "SixAxis Controller" : "Move Motion Controller");
            break;
        }
    }
    
    /* Check if a controller was found */
    if (dev == NULL) {
        fprintf(stderr, "Could not find any PlayStation controllers (Vendor ID: 0x%04x, Product IDs: 0x%04x, 0x%04x)\n", 
                VENDOR, PRODUCT[0], PRODUCT[1]);
        fprintf(stderr, "Make sure the controller is connected via USB and powered on\n");
        return 0;
    }

    /* Either pair with a new MAC or show the current pairing */
    if (argc == 2) {
        pair_device(dev, argv[1], strlen(argv[1]));  /* Set new MAC address */
    } else {
        show_pairing(dev);  /* Show current MAC address */
    }

    /* Clean up and close the connection */
    hid_close(dev);
    return 0;
}