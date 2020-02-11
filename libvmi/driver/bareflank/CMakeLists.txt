find_package(JSON-C)
set_package_properties(JSON-C PROPERTIES
    PURPOSE "Dependency for Bareflank driver")
if (NOT JSON-C_FOUND)
    set(ENABLE_BAREFLANK OFF CACHE BOOL "Build Bareflank driver" FORCE)
    message(WARNING "Cannot find JSON: disabling Bareflank driver")
else ()
    target_sources(vmi_shared PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/bareflank.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hypercall.S
    )
    target_include_directories(vmi_shared PRIVATE ${JSON-C_INCLUDE_DIRS})
    target_link_libraries(vmi_shared PRIVATE ${JSON-C_LIBRARIES})
    set_property(GLOBAL APPEND PROPERTY G_VMI_PUBLIC_DEPS ${JSON-C_LIBRARIES})
endif ()
