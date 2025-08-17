#include "cli_usb.h"

#if CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C6

#include "driver/usb_serial_jtag.h"
#include "esp_vfs_usb_serial_jtag.h"

#define USB_BUF_SIZE 256

usb_serial_jtag_driver_config_t driver_usb = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();

cli_struct_t cli_usb = {0};
static char usb_line_buffer[USB_BUF_SIZE];
static int usb_line_pos = 0;

void cli_usb_set_in_process_command()
{
    cli_start_process(&cli_usb);
}

void cli_usb_stop_process()
{
    cli_stop_process(&cli_usb);
}

void cli_usb_print(const char *format, ...)
{
    char buffer[USB_BUF_SIZE];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0)
    {
        usb_serial_jtag_write_bytes((const uint8_t *)buffer, len, 0);
    }
}

void cli_usb_general_help()
{
    cli_usb_print("Use 'help <command>' to get help.\r\n");
    cli_usb_print("\r\nCommands:\r\n");
    for (uint32_t i = 0; i < cli_usb.count_of_commands; i++)
    {
        cli_usb_print(" - %s\r\n", cli_usb.commands[i].command_text);
    }
}

static void cli_usb_task(void *arg)
{
    uint8_t buf[64];
    while (1)
    {
        int len = usb_serial_jtag_read_bytes(buf, sizeof(buf), 10 / portTICK_PERIOD_MS);
        if (len > 0)
        {
            for (int i = 0; i < len; i++)
            {
                char ch = buf[i];
                if (ch == '\r' || ch == '\n')
                {
                    if (usb_line_pos > 0)
                    {
                        usb_line_buffer[usb_line_pos] = '\0';
                        cli_usb_print("\r\n");
                        cli_process_input(&cli_usb, usb_line_buffer);
                        usb_line_pos = 0;
                        cli_usb_print("> ");
                    }
                }
                else if (usb_line_pos < USB_BUF_SIZE - 1)
                {
                    usb_line_buffer[usb_line_pos++] = ch;
                    cli_usb_print("%c", ch); // echo
                }
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void cli_usb_add_command(cli_command_t command)
{
    cli_add_command(&cli_usb, command);
}

void cli_usb_set_configuration(cli_command_t commands[], uint16_t cli_commands_total_enum, void *context)
{
    cli_set_commands(&cli_usb, commands, cli_commands_total_enum);
    cli_usb.context = context;
    cli_usb.actual_command = 0;
    cli_usb.process_running = false;

    cli_set_help_callback(&cli_usb, cli_usb_general_help);
    cli_set_print_callback(&cli_usb, cli_usb_print);
}

void cli_usb_init()
{
    // instala el VFS del usb_serial_jtag para stdout/stderr
    usb_serial_jtag_driver_install(&driver_usb);

    cli_usb_general_help();
    cli_usb_print("> ");

    // crea la tarea que maneja RX
    xTaskCreate(cli_usb_task, "cli_usb_task", 4096, NULL, 5, NULL);
}

#endif