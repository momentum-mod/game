project(raytrace)

set(SHARED_INCLUDE_DIRS
         "${CMAKE_SOURCE_DIR}/common"
        "${CMAKE_SOURCE_DIR}/public"
        "${CMAKE_SOURCE_DIR}/public/tier0"
        "${CMAKE_SOURCE_DIR}/public/tier1"
        "${CMAKE_SOURCE_DIR}/utils/common"
        )
ucm_add_dirs("." TO RAYTRACE_SOURCES NO_HEADERS RECURSIVE)
add_library(raytrace STATIC ${RAYTRACE_SOURCES})
target_include_directories(raytrace PRIVATE ${SHARED_INCLUDE_DIRS})