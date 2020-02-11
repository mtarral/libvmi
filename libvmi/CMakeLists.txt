set(libvmi_src
    accessors.c
    convenience.c
    core.c
    events.c
    performance.c
    pretty_print.c
    read.c
    slat.c
    strmatch.c
    write.c
    memory.c
    msr-index.c
    arch/arch_interface.c
    arch/intel.c
    arch/amd64.c
    arch/arm_aarch32.c
    arch/arm_aarch64.c
    driver/driver_interface.c
    driver/memory_cache.c
    os/os_interface.c
)

add_library(vmi OBJECT ${libvmi_src})
# force -fPIC
set_property(TARGET vmi PROPERTY POSITION_INDEPENDENT_CODE ON)

# define Libvmi public headers to be installed
# Note: We use a property because we want to modify this list
# in subdirectories
set_property(GLOBAL PROPERTY G_VMI_PUBLIC_HEADERS
    libvmi.h
    libvmi_extra.h
    slat.h
    x86.h)

# define list of external dependencies, used by libtool for the static library
# Note: use a property here as well
set_property(GLOBAL PROPERTY G_VMI_PUBLIC_DEPS "")

# create libvmi.so
add_library (vmi_shared SHARED $<TARGET_OBJECTS:vmi>)
# performance.c uses math.h in implementation
target_link_libraries(vmi_shared PRIVATE m)
set_property(GLOBAL APPEND PROPERTY G_VMI_PUBLIC_DEPS m)
# one libvmi_extra.h function returns a GSList*
target_link_libraries(vmi_shared PUBLIC ${GLIB_LDFLAGS})
# cleanup GLIB_LDFLAGS (remove -l prefix)
string(REGEX REPLACE "-l" "" GLIB_LDFLAGS ${GLIB_LDFLAGS})
set_property(GLOBAL APPEND PROPERTY G_VMI_PUBLIC_DEPS ${GLIB_LDFLAGS})
set_target_properties(vmi_shared PROPERTIES OUTPUT_NAME "vmi")
# set soname
set_target_properties(vmi_shared PROPERTIES
    VERSION
    0.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    SOVERSION 0
)
if (ENABLE_STATIC)
    # create libvmi.a
    add_library (vmi_static STATIC $<TARGET_OBJECTS:vmi>)
    set_target_properties(vmi_static PROPERTIES OUTPUT_NAME "vmi")
endif ()

if (ENABLE_CONFIGFILE)
    add_subdirectory(config)
endif ()

add_subdirectory(driver)
add_subdirectory(os)

if (ENABLE_WINDOWS)
    set_property(GLOBAL APPEND PROPERTY G_VMI_PUBLIC_HEADERS peparse.h)
endif ()

if (ENABLE_ADDRESS_CACHE)
    target_sources(vmi_shared PRIVATE cache.c)
endif ()

if (REKALL_PROFILES OR VOLATILITY_IST)
    find_package(JSON-C)
    set_package_properties(JSON-C PROPERTIES
        PURPOSE "Dependency for Rekall profiles and Volatility IST parsing")
    if (NOT JSON-C_FOUND)
        set(ENABLE_JSON_PROFILES OFF)
        set(REKALL_PROFILES OFF)
        set(VOLATILITY_IST OFF)
        message(WARNING "Cannot find JSON: disabling Rekall profiles and Volatility IST")
    else ()
        set(ENABLE_JSON_PROFILES ON)
        target_sources(vmi_shared PRIVATE json_profiles/json_profiles.c)
        if (REKALL_PROFILES)
            target_sources(vmi_shared PRIVATE json_profiles/rekall.c)
        endif ()
        if (VOLATILITY_IST)
            target_sources(vmi_shared PRIVATE json_profiles/volatility_ist.c)
        endif ()
        target_include_directories(vmi_shared PRIVATE ${JSON-C_INCLUDE_DIRS})
        target_link_libraries(vmi_shared PRIVATE ${JSON-C_LIBRARIES})
        set_property(GLOBAL APPEND PROPERTY G_VMI_PUBLIC_DEPS ${JSON-C_LIBRARIES})
    endif ()
endif ()

install(TARGETS vmi_shared DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
if (ENABLE_STATIC)
    install(TARGETS vmi_static DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
endif ()

get_property(VMI_PUBLIC_HEADERS GLOBAL PROPERTY G_VMI_PUBLIC_HEADERS)
install(FILES ${VMI_PUBLIC_HEADERS} DESTINATION include/libvmi)

# remove duplicates
get_property(VMI_PUBLIC_DEPS GLOBAL PROPERTY G_VMI_PUBLIC_DEPS)
list(REMOVE_DUPLICATES VMI_PUBLIC_DEPS)

set (DEPENDENCY_LIBS "")
foreach (PUB_LIB ${VMI_PUBLIC_DEPS})
    set(DEPENDENCY_LIBS "${DEPENDENCY_LIBS} -l${PUB_LIB}")
endforeach ()
# expand ${VAR}
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libtool_template.in
    ${CMAKE_CURRENT_BINARY_DIR}/libtool_template.gen)

# expand $<EXPRESSION>
file(GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libvmi.la
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/libtool_template.gen)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libvmi.la DESTINATION
    ${CMAKE_INSTALL_FULL_LIBDIR})
