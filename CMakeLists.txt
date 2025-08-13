file(GLOB_RECURSE CLI_CORE_SRC
    "cli-core/src/*.c"
    "*.c"
)

idf_component_register(
    SRCS ${CLI_CORE_SRC}
    INCLUDE_DIRS
        "cli-core/include"
        "include"
    REQUIRES driver freertos
)
