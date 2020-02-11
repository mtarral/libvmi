find_package(FLEX)
set_package_properties(FLEX PROPERTIES
    DESCRIPTION "Scanner generator for lexing in C and C++"
    URL "https://github.com/westes/flex"
    TYPE OPTIONAL
    PURPOSE "Lexing LibVMI configuration file")
find_package(BISON)
set_package_properties(BISON PROPERTIES
    DESCRIPTION "Parser generator"
    URL "https://www.gnu.org/software/bison"
    TYPE OPTIONAL
    PURPOSE "Parsing LibVMI configuration file")
if (NOT FLEX_FOUND OR NOT BISON_FOUND)
    set(ENABLE_CONFIGFILE OFF CACHE BOOL "Enable config file" FORCE)
    message(WARNING "Cannot find flex or bison: config file parsing will be
    disabled")
else ()
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/scanner.c
        COMMAND ${FLEX_EXECUTABLE}
            --outfile=${CMAKE_CURRENT_BINARY_DIR}/scanner.c
            ${CMAKE_CURRENT_SOURCE_DIR}/lexicon.l
        COMMENT "Generating scanner.c"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/parser.c
        COMMAND ${BISON_EXECUTABLE}
            --output=${CMAKE_CURRENT_BINARY_DIR}/parser.c
            --defines=${CMAKE_CURRENT_BINARY_DIR}/grammar.h
            ${CMAKE_CURRENT_SOURCE_DIR}/grammar.y
        COMMENT "Generating parser.c"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # build specific target for scanner.c and parser.c
    # to remove Werror flag, otherwise flex and bison's generated
    # source file won't compile
    add_library(config_parser STATIC
        ${CMAKE_CURRENT_BINARY_DIR}/scanner.c
        ${CMAKE_CURRENT_BINARY_DIR}/parser.c
    )

    target_include_directories(config_parser PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR} # grammar.h
        ${CMAKE_CURRENT_SOURCE_DIR} # config_parser.h
    )
    # remove Werror flag just for config_parser
    # compile with -fPIC to allow static linking
    set_target_properties(config_parser PROPERTIES COMPILE_FLAGS "-Wno-error -fPIC")
    # link with main libvmi library
    target_link_libraries(vmi_shared PRIVATE config_parser)
endif ()
