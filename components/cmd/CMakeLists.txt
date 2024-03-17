idf_component_register(SRC_DIRS "."
                       INCLUDE_DIRS "."
                       REQUIRES cmock valves load_cell cli)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format" "-Wall" "-Werror")
