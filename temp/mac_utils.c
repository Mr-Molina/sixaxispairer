/**
 * mac_utils.c - MAC address handling utilities
 * 
 * Implementation of functions for converting between string and byte representations of MAC addresses
 */

#include "mac_utils.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/**
 * Converts a hexadecimal character to its 4-bit numeric value (nibble)
 */
unsigned char char_to_nibble(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';           /* Convert ASCII digit to numeric value */
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;      /* Convert lowercase hex to numeric value */
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;      /* Convert uppercase hex to numeric value */
    return 255;                   /* Invalid character */
}

/**
 * Converts a MAC address string to byte array
 * Handles both formats: "AABBCCDDEEFF" and "AA:BB:CC:DD:EE:FF"
 */
int mac_to_bytes(const char *in, size_t in_len, unsigned char *out, size_t out_len)
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
        out[i] = (char_to_nibble(*p) << 4) | (char_to_nibble(*(p + 1) & 0xff));
        i++;
        p += 2;
    }

    /* Ensure we processed the entire input string */
    if (p < in + in_len)
        return 0;
    return 1;
}

/**
 * Formats a MAC address byte array as a string
 */
int bytes_to_mac_string(const unsigned char *in, char *out, size_t out_len, int use_colons)
{
    /* Validate input parameters */
    if (in == NULL || out == NULL || out_len == 0)
        return 0;
    
    /* Check if output buffer is large enough */
    size_t required_len = use_colons ? 18 : 13; /* 17 chars + null or 12 chars + null */
    if (out_len < required_len)
        return 0;
    
    /* Format the MAC address */
    if (use_colons) {
        snprintf(out, out_len, "%02x:%02x:%02x:%02x:%02x:%02x", 
                 in[0], in[1], in[2], in[3], in[4], in[5]);
    } else {
        snprintf(out, out_len, "%02x%02x%02x%02x%02x%02x", 
                 in[0], in[1], in[2], in[3], in[4], in[5]);
    }
    
    return 1;
}