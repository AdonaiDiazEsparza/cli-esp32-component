// usb_cli.c

#include "cli_usb.h"

#define USB_BUF_SIZE 256

cli_struct_t cli_usb = {0};
static char usb_line_buffer[USB_BUF_SIZE];
static int usb_line_pos = 0;

void cli_usb_set_in_process_command(){
    cli_set_in_process_command(&cli_usb);
}

void cli_usb_print(const char* format, ...)
{
    char buffer[USB_BUF_SIZE];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0) {
        tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, (const uint8_t*) buffer, len);
        tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, 0);  // flush out
    }
}

void cli_usb_general_help()
{
    cli_usb_print("Usa 'help <comando>' para obtener ayuda.\r\n");
    cli_usb_print("\r\nComandos disponibles:\r\n");
    for (uint32_t i = 0; i < cli_usb.count_of_commands; i++)
    {
        cli_usb_print(" - %s\r\n", cli_usb.text_commands[i]);
    }
}

static void usb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    if (event->type == CDC_EVENT_RX) {
        uint8_t buf[64];
        size_t len = 0;
        tinyusb_cdcacm_read(itf, buf, sizeof(buf), &len);

        for (int i = 0; i < len; i++) {
            char ch = buf[i];

            if (ch == '\r' || ch == '\n') {
                if (usb_line_pos > 0) {
                    usb_line_buffer[usb_line_pos] = '\0';
                    cli_usb_print("\r\n");
                    cli_process_input(&cli_usb, usb_line_buffer);
                    usb_line_pos = 0;
                    cli_usb_print("> ");
                }
            } else if (usb_line_pos < USB_BUF_SIZE - 1) {
                usb_line_buffer[usb_line_pos++] = ch;
                cli_usb_print("%c", ch);  // echo
            }
        }
    }
}

void cli_usb_set_configuration(cli_calls_t cli_calls_handlers, uint32_t cli_commands_total_enum, const char** commands, void *context)
{
    cli_usb.calls_for_work = cli_calls_handlers;
    cli_usb.text_commands = commands;
    cli_usb.count_of_commands = cli_commands_total_enum;
    cli_usb.context = context;
    cli_usb.actual_command = 0;
    cli_usb.process_running = false;

    cli_set_help_callback(&cli_usb, cli_usb_general_help);
    cli_set_print_callback(&cli_usb, cli_usb_print);
}

void cli_usb_init()
{
    tinyusb_config_t tusb_cfg = {};
    tusb_cfg.device_descriptor = NULL;  // usa el por defecto
    tusb_cfg.string_descriptor = NULL;
    tusb_cfg.external_phy = false;

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    const tinyusb_config_cdcacm_t cdc_config = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = usb_cdc_rx_callback,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL
    };

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&cdc_config));

    cli_usb_print("\r\nCLI USB iniciado.\r\n");
    cli_usb_general_help();
    cli_usb_print("> ");
}
