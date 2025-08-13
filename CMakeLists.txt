set(CLI_CORE_SRC
    "cli-core/src/cli_core.c"
    "cli_uart_esp32/cli_uart.c"
    "cli_usb_esp32/cli_usb.c"
)

idf_component_register(
    SRCS ${CLI_CORE_SRC}
    INCLUDE_DIRS
        "cli-core/include"
        "include"
    REQUIRES driver freertos
)
