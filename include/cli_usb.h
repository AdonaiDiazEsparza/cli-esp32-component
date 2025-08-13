#pragma once

/**
 * To work with this library needs to add a dependency, the esp_tinyusb dependency
 * It helps to comunicate via USB to our computer or port USB
 *      idf.py add-dependency "espressif/esp_tinyusb^1.7.6"
 * Author: Adonai Diaz (Donuts Diaz)
 * Date: 6 July 2025
 */


#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "cli_core.h"

void cli_usb_set_configuration(cli_command_t comands[] ,uint16_t cli_commands_total_enum, void *context);
void cli_usb_init(void);
void cli_usb_print(const char* format, ...);
void cli_usb_set_in_process_command();
