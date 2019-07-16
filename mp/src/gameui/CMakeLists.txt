project(gameui)

set(SHARED_INCLUDE_DIRS
        "${CMAKE_SOURCE_DIR}/gameui"
        "${CMAKE_SOURCE_DIR}/game/shared/momentum"
        )

ucm_add_dirs("." TO GAMEUI_SOURCES NO_HEADERS RECURSIVE)
ucm_remove_files(
        CustomTabExplanationDialog.cpp
        LoadingTipPanel.cpp
        FROM GAMEUI_SOURCES)


add_library(gameui STATIC ${GAMEUI_SOURCES})

target_compile_definitions(gameui PRIVATE _LIB GAMEUI_STATIC_LIB GAMEUI_EXPORTS VERSION_SAFE_STEAM_API_INTERFACES)

if(CMAKE_HOST_UNIX)
    target_compile_definitions(gameui PRIVATE POSIX LINUX)
endif()

target_include_directories(gameui PRIVATE
        "${CMAKE_SOURCE_DIR}/common"
        "${CMAKE_SOURCE_DIR}/public"
        "${CMAKE_SOURCE_DIR}/public/tier0"
        "${CMAKE_SOURCE_DIR}/public/tier1"
        ${SHARED_INCLUDE_DIRS}
        )

if(CMAKE_HOST_WIN32)
#    target_link_libraries(gameui PRIVATE Rpcrt4.lib)
endif()
