#include "cli_uart.h"

#define UART_PORT_NUM UART_NUM_0
#define UART_TX_PIN 43
#define UART_RX_PIN 44
#define UART_BUF_SIZE 1024

static QueueHandle_t uart_queue; // Queue for UART
cli_struct_t cli_uart = {0}; // struct for cli

// Buffer temporal para entrada por línea
static char line_buffer[UART_BUF_SIZE];
static int line_pos = 0;

// Task for UART CLI
static void uart_event_task(void *pvParameters);

void cli_uart_set_in_process_command(){
    cli_set_in_process_command(&cli_uart);
}

// Supposed to work to print the cli
void cli_uart_print(const char *format, ...)
{
    char buffer[UART_BUF_SIZE]; // Ajusta el tamaño si necesitas más
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0)
    {
        uart_write_bytes(UART_PORT_NUM, buffer, len);
    }
}

// Call for give in terminal the general help
void cli_uart_general_help()
{
    cli_uart_print("Usa 'help <comando>' para obtener ayuda.\r\n");
    cli_uart_print("\r\nComandos disponibles:\r\n");
    for (uint32_t i = 0; i < cli_uart.count_of_commands; i++)
    {
        cli_uart_print(" - %s\r\n", cli_uart.text_commands[i]);
    }
}

// Start configuration
void cli_uart_set_configuration(uint32_t baudrate, cli_calls_t cli_calls_handlers, uint32_t cli_commands_total_enum, const char** commands, void *context)
{
    cli_uart.calls_for_work = cli_calls_handlers;
    cli_uart.text_commands = commands;
    cli_uart.count_of_commands = cli_commands_total_enum;
    cli_uart.context = context;
    cli_uart.actual_command = 0;
    cli_uart.process_running = false;
    
    // cli set help callback
    cli_set_help_callback(&cli_uart, cli_uart_general_help);

    // cli set print callback
    cli_set_print_callback(&cli_uart, cli_uart_print);

    const uart_config_t uart_config = {
        .baud_rate = baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE * 2, 0, 10, &uart_queue, 0);
    uart_param_config(UART_PORT_NUM, &uart_config);
    uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

// Start cli 
void cli_uart_init()
{
    cli_struct_t* cli_pointer = &cli_uart;
    if(!cli_pointer){
        cli_uart_print("CLI NOT CONFIGURED");
        return;
    }

    xTaskCreate(uart_event_task, "uart_event_task", 4096, NULL, 10, NULL);
}

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;

    cli_uart_print("\r\nCLI iniciado.");
    cli_uart_general_help();
    cli_uart_print(">");

    while (1)
    {
        if (xQueueReceive(uart_queue, &event, portMAX_DELAY))
        {
            switch (event.type)
            {
            case UART_DATA: {
                uint8_t data[128];
                int len = uart_read_bytes(UART_PORT_NUM, data, event.size, portMAX_DELAY);
                for (int i = 0; i < len; i++) {
                    uint8_t ch = data[i];
                    if (ch == '\r' || ch == '\n') {
                        if (line_pos > 0) {
                            line_buffer[line_pos] = '\0';
                            cli_uart_print("\r\n");
                            cli_process_input(&cli_uart,line_buffer);
                            line_pos = 0;
                            cli_uart_print("> ");
                        }
                    } else if (line_pos < UART_BUF_SIZE - 1) {
                        line_buffer[line_pos++] = ch;
                        uart_write_bytes(UART_PORT_NUM, (const char*)&ch, 1); // echo
                    }
                }
                break;
            }

            case UART_FIFO_OVF:
            case UART_BUFFER_FULL:
                uart_flush_input(UART_PORT_NUM);
                xQueueReset(uart_queue);
                break;

            default:
                break;
            }
        }
    }
}