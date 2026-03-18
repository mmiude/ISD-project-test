idf_component_register(
    INCLUDE_DIRS "include"
    REQUIRES driver
)

add_prebuilt_library(bmv080_prebuilt "${CMAKE_CURRENT_LIST_DIR}/lib_bmv080.a")
add_prebuilt_library(postproc_prebuilt "${CMAKE_CURRENT_LIST_DIR}/lib_postProcessor.a")

target_link_libraries(${COMPONENT_LIB}
    INTERFACE
        bmv080_prebuilt
        postproc_prebuilt
)