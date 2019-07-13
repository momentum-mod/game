include("precompile_header")

######################
# SHARED DEFINITIONS #
######################
set(SHARED_INCLUDE_DIRS
        "server"
        "${CMAKE_SOURCE_DIR}/common"
        "${CMAKE_SOURCE_DIR}/public"
        "${CMAKE_SOURCE_DIR}/public/tier0"
        "${CMAKE_SOURCE_DIR}/public/tier1"
        "${CMAKE_SOURCE_DIR}/utils/common"
        "${CMAKE_SOURCE_DIR}/thirdparty/discord-rpc"
        "shared"
        "shared/hl2"
        "shared/hl2mp"
        "shared/momentum"
)

set(SHARED_DEFINITIONS -DHL2MP -DVERSION_SAFE_STEAM_API_INTERFACES -DGE_DLL -DGE_USE_ROLLINGEXP)

if (CMAKE_HOST_UNIX)
    set(SHARED_LINK_DIRS
            "${CMAKE_SOURCE_DIR}/lib/public/linux32"
            "${CMAKE_SOURCE_DIR}/lib/common/linux32")
else ()
    set(SHARED_LINK_DIRS "${CMAKE_SOURCE_DIR}/lib/public" "${CMAKE_SOURCE_DIR}/lib/common")
endif ()

find_library(MATHLIB_LIBRARY mathlib.a PATHS ${SHARED_LINK_DIRS})
find_library(PARTICLES_LIBRARY particles.a PATHS ${SHARED_LINK_DIRS})
find_library(DISCORD_LIBRARY libdiscord-rpc.so PATHS ${SHARED_LINK_DIRS})
find_library(DMXLOADER_LIBRARY dmxloader.a PATHS ${SHARED_LINK_DIRS})
find_library(CHOREOOBJECTS_LIBRARY choreoobjects.a PATHS ${SHARED_LINK_DIRS})
find_library(TIER2_LIBRARY tier2.a PATHS ${SHARED_LINK_DIRS})
find_library(TIER3_LIBRARY tier3.a PATHS ${SHARED_LINK_DIRS})
find_library(LIBZ_LIBRARY z PATHS ${SHARED_LINK_DIRS})

SET(SHARED_LIBS
        ${PARTICLES_LIBRARY}
        ${MATHLIB_LIBRARY}
        ${DISCORD_LIBRARY}
        ${DMXLOADER_LIBRARY}
        ${CHOREOOBJECTS_LIBRARY}
        tier1
        ${TIER2_LIBRARY}
        ${TIER3_LIBRARY}
        ${LIBZ_LIBRARY}
        m
        dl
        pthread
        ldap
        rt
        util
        nsl
        )

ucm_add_dirs("shared" TO SHARED_SOURCES NO_HEADERS RECURSIVE)
ucm_remove_files(
        shared/SharedFunctorUtils.cpp
        shared/cs_achievements_and_stats_interface.cpp
        shared/weapon_ifmbase.cpp
        shared/weapon_ifmbasecamera.cpp
        shared/weapon_ifmsteadycam.cpp
        shared/gamerules.cpp
        shared/hl2/hl_gamemovement.cpp
        shared/hl2/survival_gamerules.cpp
        shared/hl2mp/hl2mp_player_shared.cpp
        shared/hl2mp/hl2mp_weapon_parse.cpp
        FROM SHARED_SOURCES)
ucm_remove_directories("shared/sdk" "shared/episodic" FROM SHARED_SOURCES)
ucm_add_files(
        ../common/language.cpp
        ../common/randoverride.cpp
        ../public/bone_setup.cpp
        ../public/collisionutils.cpp
        ../public/dt_utlvector_common.cpp
        ../public/filesystem_helpers.cpp
        ../public/interpolatortypes.cpp
        ../public/haptics/haptic_msgs.cpp
        ../public/networkvar.cpp
        ../public/rope_physics.cpp
        ../public/scratchpad3d.cpp
        ../public/ScratchPadUtils.cpp
        ../public/simple_physics.cpp
        ../public/SoundParametersInternal.cpp
        ../public/stringregistry.cpp
        ../public/studio.cpp
        TO SHARED_SOURCES FILTER_POP 1)
if (CMAKE_HOST_WIN32)
    ucm_add_files(../public/haptics/haptic_utils.cpp TO SHARED_SOURCES FILTER_POP 1)
endif ()

if (CMAKE_HOST_UNIX)
    # Link to pre-built libraries
    file(COPY "${CMAKE_SOURCE_DIR}/lib/public/linux32/libsteam_api.so" DESTINATION "${CMAKE_BINARY_DIR}/game/")
    add_library(steam_api SHARED IMPORTED)
    set_target_properties(steam_api PROPERTIES
            IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/game/libsteam_api.so"
            IMPORTED_SONAME "libsteam_api.so")

    file(COPY "${CMAKE_SOURCE_DIR}/lib/public/linux32/libtier0.so" DESTINATION "${CMAKE_BINARY_DIR}/game/")
    add_library(tier0 SHARED IMPORTED)
    set_target_properties(tier0 PROPERTIES
            IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/game/libtier0.so"
            IMPORTED_SONAME "libtier0.so")

    file(COPY "${CMAKE_SOURCE_DIR}/lib/public/linux32/libvstdlib.so" DESTINATION "${CMAKE_BINARY_DIR}/game/")
    add_library(vstdlib SHARED IMPORTED)
    set_target_properties(vstdlib PROPERTIES
            IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/game/libvstdlib.so"
            IMPORTED_SONAME "libvstdlib.so")
endif ()


#################
## SERVER BUILD #
#################
ucm_add_dirs("server" TO SERVER_SOURCES NO_HEADERS RECURSIVE)
ucm_remove_directories("server/sdk" FROM SERVER_SOURCES)
ucm_add_files(
        ../public/dt_send.cpp
        ../public/dt_utlvector_send.cpp
        ../public/editor_sendcommand.cpp
        ../public/keyframe/keyframe.cpp
        ../public/map_utils.cpp
        ../public/registry.cpp
        ../public/server_class.cpp
        TO SERVER_SOURCES FILTER_POP 1)
LIST(APPEND SERVER_SOURCES ${SHARED_SOURCES})
ADD_MSVC_PRECOMPILED_HEADER("server/stdafx.h" "server/stdafx.cpp" SERVER_SOURCES)

add_library(server SHARED ${SERVER_SOURCES})
add_dependencies(server tier1)

target_link_libraries(server -m32 -Wl,--no-undefined ${SHARED_LIBS})
target_compile_definitions(server PUBLIC ${SHARED_DEFINITIONS} GAME_DLL HL2_DLL)
target_include_directories(server PUBLIC "server" "server/hl2" "server/hl2mp" ${SHARED_INCLUDE_DIRS})

if (CMAKE_HOST_UNIX)
    # Linux library name is server.so
    set_target_properties(server PROPERTIES PREFIX "")
    set_target_properties(server PROPERTIES OUTPUT_NAME server)

    target_link_libraries(server steam_api tier0 vstdlib)

    install(TARGETS server LIBRARY DESTINATION "bin")
else ()
    install(TARGETS server RUNTIME DESTINATION "bin")
endif ()
#
#
#################
## CLIENT BUILD #
#################
ucm_add_dirs("client" TO CLIENT_SOURCES NO_HEADERS RECURSIVE)
ucm_remove_files(
        client/MonitorMaterialProxy.cpp
        client/c_forcefeedback.cpp
        client/cdll_client_int.cpp
        client/hud_basedeathnotice.cpp
        client/hud_chat.cpp
        client/hud_controlpointicons.cpp
        client/hud_voicestatus.cpp
        client/in_forcefeedback.cpp
        client/in_mouse.cpp
        client/hud_vote.cpp
        client/particle_prototype.cpp
        client/thermalmaterialproxy.cpp
        client/vgui_entityimagepanel.cpp
        client/vgui_imagehealthpanel.cpp
        client/viewrender.cpp
        client/episodic/c_npc_advisor.cpp
        client/episodic/c_vort_charge_token.cpp
        client/hl2/c_energy_wave.cpp
        client/hl2/c_extinguisher.cpp
        client/hl2/c_npc_antlionguard.cpp
        client/hl2/c_npc_hydra.cpp
        client/hl2/c_te_flare.cpp
        client/hl2/c_weapon_gravitygun.cpp
        client/hl2/fx_antlion.cpp
        client/hl2/fx_hl2_tracers.cpp
        client/hl2/hud_locator.cpp
        client/hl2mp/c_hl2mp_player.cpp
        client/momentum/ui/ContactPanel.cpp
        FROM CLIENT_SOURCES)
ucm_remove_directories("client/sdk" "client/replay" "client/ShaderEditor" FROM CLIENT_SOURCES)
ucm_add_files(
        ../common/compiledcaptionswap.cpp
        ../public/bone_accessor.cpp
        ../public/client_class.cpp
        ../public/crtmemdebug.cpp
        ../public/dt_recv.cpp
        ../public/dt_utlvector_recv.cpp
        ../public/jigglebones.cpp
        ../public/posedebugger.cpp
        ../public/renamed_recvtable_compat.cpp
        ../public/sentence.cpp
        ../public/vallocator.cpp
        ../public/tools/bonelist.cpp
        ../public/vgui_controls/vgui_controls.cpp
        TO CLIENT_SOURCES FILTER_POP 1)

LIST(APPEND CLIENT_SOURCES ${SHARED_SOURCES})
ADD_MSVC_PRECOMPILED_HEADER("client/stdafx.h" "client/stdafx.cpp" CLIENT_SOURCES)

add_library(client SHARED ${CLIENT_SOURCES})
add_dependencies(client tier1)

find_library(BITMAP_LIBRARY bitmap.a PATHS ${SHARED_LINK_DIRS})
find_library(MATSYS_LIBRARY matsys_controls.a PATHS ${SHARED_LINK_DIRS})
find_library(VTF_LIBRARY vtf.a PATHS ${SHARED_LINK_DIRS})

target_link_libraries(client -m32 -Wl,--no-undefined
        ${BITMAP_LIBRARY}
        ${MATSYS_LIBRARY}
        ${VTF_LIBRARY}
        ${SHARED_LIBS}
        vgui_controls
)

if (CMAKE_HOST_WIN32)
    find_library(FMOD_LIBRARY fmod PATHS ${SHARED_LINK_DIRS})
    target_link_libraries(client ${FMOD_LIBRARY})
endif ()

target_compile_definitions(client PUBLIC
        ${SHARED_DEFINITIONS}
        CLIENT_DLL HL2_CLIENT_DLL ENABLE_HTMLWINDOW
        NO_STRING_T
        VERSION_SAFE_STEAM_API_INTERFACES
)

target_include_directories(client PUBLIC
        "client"
        "client/game_controls"
        "client/hl2"
        "client/hl2mp"
        "client/hl2mp/ui"
        "client/momentum"
        "client/momentum/ui"
        "client/momentum/ui/controls"
        "client/momentum/ui/HUD"
        "client/momentum/ui/leaderboards"
        "client/momentum/ui/MapSelection"
        "client/momentum/ui/SettingsPanel"
        "client/momentum/ui/spectate"
        "client/momentum/ui/ZoneMenu"
        "../thirdparty/discord-rpc/include"
        "${CMAKE_SOURCE_DIR}/vgui2/include"
        "${CMAKE_SOURCE_DIR}/vgui2/controls"
        "${CMAKE_SOURCE_DIR}/thirdparty"
        ${SHARED_INCLUDE_DIRS}
)

if (CMAKE_HOST_UNIX)
    # Linux library name is client.so
    set_target_properties(client PROPERTIES PREFIX "")
    set_target_properties(client PROPERTIES OUTPUT_NAME client)

    target_link_libraries(client steam_api tier0 vstdlib)

    install(TARGETS client LIBRARY DESTINATION "bin")
else ()
    install(TARGETS client RUNTIME DESTINATION "bin")
endif ()
