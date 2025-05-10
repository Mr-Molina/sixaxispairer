/**
 * mac_utils.h - MAC address handling utilities
 * 
 * Functions for converting between string and byte representations of MAC addresses
 */

#ifndef MAC_UTILS_H
#define MAC_UTILS_H

#include <stdlib.h>

/**
 * Converts a hexadecimal character to its 4-bit numeric value (nibble)
 *
 * @param c The character to convert ('0'-'9', 'a'-'f', or 'A'-'F')
 * @return The numeric value (0-15) or 255 if the character is invalid
 */
unsigned char char_to_nibble(char c);

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
int mac_to_bytes(const char *in, size_t in_len, unsigned char *out, size_t out_len);

/**
 * Formats a MAC address byte array as a string
 * 
 * @param in Input byte array containing MAC address
 * @param out Output string buffer
 * @param out_len Length of output buffer
 * @param use_colons Whether to include colons in the output format
 * @return 1 on success, 0 on failure
 */
int bytes_to_mac_string(const unsigned char *in, char *out, size_t out_len, int use_colons);

#endif /* MAC_UTILS_H */