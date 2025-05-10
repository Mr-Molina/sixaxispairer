/**
 * controller_connection.h - PlayStation controller connection handling
 * 
 * Functions for connecting to and communicating with PlayStation controllers
 */

#ifndef CONTROLLER_CONNECTION_H
#define CONTROLLER_CONNECTION_H

#include "platform_compat.h"
#include "controller_info.h"

/**
 * Attempts to connect to a controller using the most appropriate method
 * 
 * @param controller The controller information
 * @return Handle to the connected device, or NULL if connection failed
 */
hid_device* connect_to_controller(controller_info_t *controller);

/**
 * Attempts to connect to a DualShock 4 controller using raw device access
 * 
 * @param controller The controller information
 * @return Handle to the connected device, or NULL if connection failed
 */
hid_device* connect_to_dualshock4_raw(controller_info_t *controller);

/**
 * Retrieves and displays all available information from a HID device
 * by trying different report IDs
 *
 * @param dev Handle to the HID device
 */
void dump_device_info(hid_device *dev);

/**
 * Pairs a PlayStation controller with the specified MAC address
 *
 * @param dev Handle to the HID device
 * @param mac MAC address string to pair with
 * @param mac_len Length of the MAC address string
 */
void pair_device(hid_device *dev, const char *mac, size_t mac_len);

/**
 * Displays the currently paired MAC address of the controller
 *
 * @param dev Handle to the HID device
 */
void show_pairing(hid_device *dev);

#endif /* CONTROLLER_CONNECTION_H */
