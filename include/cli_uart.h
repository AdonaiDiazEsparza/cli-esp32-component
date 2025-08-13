#pragma once

/**
 * This library works to interact with the cli via UART, only need to create the comands and send it
 * Author: Adonai Diaz (Donuts Diaz)
 * Date: 6 July 2025
 */
#include "driver/uart.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "cli_core.h"

void cli_uart_set_configuration(uint32_t baudrate, cli_calls_t cli_calls_handlers, uint32_t cli_commands_total_enum, const char** commands, void *context);
void cli_uart_init();
void cli_uart_print(const char *format, ...);
void cli_uart_set_in_process_command();

