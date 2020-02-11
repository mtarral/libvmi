# common dependencies
find_package(Libvirt)
if (NOT Libvirt_FOUND)
    set(ENABLE_KVM OFF CACHE BOOL "Build KVM driver" FORCE)
    message(WARNING "Cannot find libvirt: disabling KVM driver")
else ()
    if (ENABLE_KVM_LEGACY)
        # legacy KVM driver
        find_package(JSON-C)
        set_package_properties(JSON-C PROPERTIES
            PURPOSE "Dependency for KVM driver")
        if (NOT JSON-C_FOUND)
            set(ENABLE_KVM OFF CACHE BOOL "Build KVM driver" FORCE)
            message(WARNING "Cannot find JSON: disabling KVM driver")
        else ()
            find_package(LibvmiRequest)
            # add sources
            # common
            message("HEEERE")
            target_sources(vmi_shared PRIVATE
                    ${CMAKE_CURRENT_SOURCE_DIR}/kvm_common.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libvirt_wrapper.c)
            # legacy driver
            target_sources(vmi_shared PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/kvm_legacy.c)
            target_include_directories(vmi_shared PRIVATE ${JSON-C_INCLUDE_DIRS})
            # CMAKE_DL_LIBS -> dlopen* lib
            target_link_libraries(vmi_shared PRIVATE ${CMAKE_DL_LIBS} ${JSON-C_LIBRARIES})
            set_property(GLOBAL APPEND PROPERTY G_VMI_PUBLIC_DEPS ${CMAKE_DL_LIBS} ${JSON-C_LIBRARIES})
        endif ()
    else ()
        # KVM driver based on KVMi series, API is exposed via libkvmi
        find_package(Libkvmi)
        if (NOT Libkvmi_FOUND)
            set(ENABLE_KVM OFF CACHE BOOL "Build KVM driver" FORCE)
            message(WARNING "Cannot find libkvmi: disabling KVM driver")
        else ()
            # add sources
            # common
            target_sources(vmi_shared PRIVATE
                    ${CMAKE_CURRENT_SOURCE_DIR}/kvm_common.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libvirt_wrapper.c)
            # KVMi driver
            target_sources(vmi_shared PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/kvm.c
                ${CMAKE_CURRENT_SOURCE_DIR}/kvm_events.c)

            target_include_directories(vmi_shared PRIVATE
                ${Libkvmi_INCLUDE_DIRS})
            target_link_libraries(vmi_shared PRIVATE ${Libkvmi_LIBRARIES})
            # CMAKE_DL_LIBS -> dlopen* lib
            target_link_libraries(vmi_shared PRIVATE ${CMAKE_DL_LIBS})
            # expose events.h
            set_property(GLOBAL APPEND PROPERTY G_VMI_PUBLIC_HEADERS events.h)
            set_property(GLOBAL APPEND PROPERTY G_VMI_PUBLIC_DEPS ${CMAKE_DL_LIBS})
        endif ()
    endif ()
endif ()
