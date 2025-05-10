/**
 * ui.h - User interface functions
 * 
 * Functions for handling user interface and command-line interactions
 */

#ifndef UI_H
#define UI_H

#include "controller_info.h"

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
 * Displays the program usage information
 * 
 * @param program_name The name of the program executable
 */
void show_usage(const char *program_name);

/**
 * Lists all connected USB devices
 * 
 * @param list_all Whether to list all USB devices or just Sony devices
 * @return 0 on success, non-zero on failure
 */
int list_devices(int list_all);

/**
 * Dumps information about connected controllers
 * 
 * @return 0 on success, non-zero on failure
 */
int dump_controller_info(void);

/**
 * Displays information about a controller
 * 
 * @param controller The controller information
 * @param index The index of the controller (1-based)
 */
void display_controller_info(controller_info_t *controller, int index);

/**
 * Prompts the user to select a controller from a list
 * 
 * @param controllers Array of controller information
 * @param controller_count Number of controllers in the array
 * @return Index of the selected controller, or -1 on error
 */
int select_controller(controller_info_t *controllers[], int controller_count);

#endif /* UI_H */