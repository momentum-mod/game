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
        "${CMAKE_SOURCE_DIR}/gameui"
        "shared"
        "shared/hl2"
        "shared/hl2mp"
        "shared/momentum"
        )

set(SHARED_DEFINITIONS -DVERSION_SAFE_STEAM_API_INTERFACES -DGE_DLL -DGE_USE_ROLLINGEXP)

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
        ${DMXLOADER_LIBRARY}
        ${CHOREOOBJECTS_LIBRARY}
        tier1
        ${TIER2_LIBRARY}
        ${TIER3_LIBRARY}
        ${LIBZ_LIBRARY}
        libcryptopp
        m
        dl
        pthread
        rt
        util
        nsl
        )

ucm_add_dirs("shared" TO SHARED_SOURCES NO_HEADERS RECURSIVE)
ucm_remove_directories("shared/sdk" "shared/episodic" "shared/hl2mp" FROM SHARED_SOURCES)
ucm_remove_files(
        shared/cs_achievements_and_stats_interface.cpp
        shared/weapon_ifmbase.cpp
        shared/weapon_ifmbasecamera.cpp
        shared/weapon_ifmsteadycam.cpp
        shared/env_meteor_shared.cpp
        shared/playerclass_info_parse.cpp

        # Multiplayer serverbase
        shared/voice_gamemgr.cpp
        shared/SharedFunctorUtils.cpp
        shared/debugoverlay_null.cpp
        shared/sceneimage.cpp
        shared/hl2/achievements_hl2.cpp

        FROM SHARED_SOURCES)
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
ucm_remove_directories("server/sdk" "server/hl2" "server/episodic" FROM SERVER_SOURCES)
ucm_add_files(
        ../public/dt_send.cpp
        ../public/dt_utlvector_send.cpp
        ../public/editor_sendcommand.cpp
        ../public/keyframe/keyframe.cpp
        ../public/map_utils.cpp
        ../public/registry.cpp
        ../public/server_class.cpp

        # not used by client and readded as it dropped out in SHARED_SOURCES
        shared/voice_gamemgr.cpp

        TO SERVER_SOURCES FILTER_POP 1)
LIST(APPEND SERVER_SOURCES ${SHARED_SOURCES})
ucm_remove_files(
        server/GameStats.cpp
        server/ai_behavior_passenger.cpp
        server/data_collector.cpp
        server/nav_area.cpp
        server/nav_colors.cpp
        server/nav_edit.cpp
        server/nav_entities.cpp
        server/nav_file.cpp
        server/nav_generate.cpp
        server/nav_ladder.cpp
        server/nav_merge.cpp
        server/nav_mesh.cpp
        server/nav_mesh_factory.cpp
        server/nav_node.cpp
        server/nav_simplify.cpp
        server/ai_relationship.cpp
        server/basegrenade_concussion.cpp
        server/basegrenade_contact.cpp
        server/basegrenade_timed.cpp
        server/hl2/Func_Monitor.cpp
        server/grenadethrown.cpp
        server/h_cycler.cpp
        server/monstermaker.cpp
        shared/hl2/survival_gamerules.cpp
        server/team_objectiveresource.cpp
        server/team_spawnpoint.cpp
        server/team_control_point.cpp
        server/team_control_point_master.cpp
        server/team_control_point_round.cpp
        server/team_train_watcher.cpp
        shared/teamplayroundbased_gamerules.cpp
        server/trigger_area_capture.cpp
        shared/teamplay_round_timer.cpp
        server/hl2/ai_allymanager.cpp
        server/hl2/ai_behavior_actbusy.cpp
        server/hl2/ai_behavior_functank.cpp
        server/hl2/ai_behavior_holster.cpp
        server/hl2/ai_behavior_police.cpp
        server/hl2/ai_goal_police.cpp
        server/hl2/ai_spotlight.cpp
        server/hl2/antlion_dust.cpp
        server/hl2/antlion_maker.cpp
        server/hl2/ar2_explosion.cpp
        server/basebludgeonweapon.cpp
        server/hl2/basehlcombatweapon.cpp
        shared/hl2/basehlcombatweapon_shared.cpp
        server/hl2/cbasehelicopter.cpp
        server/hl2/cbasespriteprojectile.cpp
        server/hl2/citadel_effects.cpp
        server/hl2/combine_mine.cpp
        server/hl2/env_alyxemp.cpp
        server/hl2/env_headcrabcanister.cpp
        shared/hl2/env_headcrabcanister_shared.cpp
        server/hl2/env_speaker.cpp
        server/hl2/env_starfield.cpp
        server/hl2/func_recharge.cpp
        server/hl2/func_tank.cpp
        server/hl2/grenade_ar2.cpp
        server/hl2/grenade_bugbait.cpp
        server/hl2/grenade_frag.cpp
        server/hl2/hl2_ai_network.cpp
        server/hl2/hl2_eventlog.cpp
        shared/hl2/hl2_gamerules.cpp
        server/hl2/hl2_player.cpp
        server/hl2/hl2_playerlocaldata.cpp
        server/hl2/hl2_triggers.cpp
        shared/hl2/hl2_usermessages.cpp
        shared/hl2/hl_gamemovement.cpp
        server/hl2/hl_playermove.cpp
        server/hl2/info_teleporter_countdown.cpp
        server/hl2/item_ammo.cpp
        server/hl2/item_battery.cpp
        server/hl2/item_dynamic_resupply.cpp
        server/hl2/item_healthkit.cpp
        server/hl2/item_itemcrate.cpp
        server/hl2/item_suit.cpp
        server/hl2/look_door.cpp
        server/hl2/monster_dummy.cpp
        server/hl2/npc_alyx.cpp
        server/hl2/npc_antlion.cpp
        server/hl2/npc_antlionguard.cpp
        server/hl2/npc_apcdriver.cpp
        server/hl2/npc_attackchopper.cpp
        server/hl2/npc_barnacle.cpp
        server/hl2/npc_barney.cpp
        server/hl2/npc_basescanner.cpp
        server/hl2/npc_BaseZombie.cpp
        server/hl2/npc_breen.cpp
        server/hl2/npc_bullseye.cpp
        server/hl2/npc_citizen17.cpp
        server/hl2/npc_combine.cpp
        server/hl2/npc_combinecamera.cpp
        server/hl2/npc_combinedropship.cpp
        server/hl2/npc_combinegunship.cpp
        server/hl2/npc_combines.cpp
        server/hl2/npc_cranedriver.cpp
        server/hl2/npc_crow.cpp
        server/hl2/npc_dog.cpp
        server/hl2/npc_eli.cpp
        server/hl2/npc_enemyfinder.cpp
        server/hl2/npc_fisherman.cpp
        server/hl2/npc_gman.cpp
        server/hl2/npc_headcrab.cpp
        server/hl2/npc_ichthyosaur.cpp
        server/hl2/npc_kleiner.cpp
        server/hl2/npc_launcher.cpp
        server/hl2/npc_manhack.cpp
        server/hl2/npc_metropolice.cpp
        server/hl2/npc_monk.cpp
        server/hl2/npc_mossman.cpp
        server/hl2/npc_playercompanion.cpp
        server/hl2/npc_PoisonZombie.cpp
        server/hl2/npc_rollermine.cpp
        server/hl2/npc_scanner.cpp
        server/hl2/npc_stalker.cpp
        server/hl2/npc_strider.cpp
        server/npc_talker.cpp
        server/hl2/npc_turret_ceiling.cpp
        server/hl2/npc_turret_floor.cpp
        server/hl2/npc_turret_ground.cpp
        server/hl2/npc_vortigaunt_episodic.cpp
        server/hl2/npc_zombie.cpp
        server/hl2/point_apc_controller.cpp
        server/hl2/prop_combine_ball.cpp
        server/hl2/prop_thumper.cpp
        server/hl2/proto_sniper.cpp
        server/hl2/rotorwash.cpp
        server/hl2/script_intro.cpp
        shared/script_intro_shared.cpp
        server/hl2/vehicle_airboat.cpp
        server/hl2/vehicle_crane.cpp
        server/hl2/vehicle_prisoner_pod.cpp
        server/hl2/vehicle_viewcontroller.cpp
        server/hl2/weapon_annabelle.cpp
        server/hl2/weapon_bugbait.cpp
        server/weapon_cubemap.cpp
        server/hl2/grenade_beam.cpp
        server/hl2/grenade_homer.cpp
        server/hl2/grenade_pathfollower.cpp
        server/hl2/npc_missiledefense.cpp
        server/hl2/vehicle_apc.cpp
        server/hl2/weapon_cguard.cpp
        server/hl2/weapon_flaregun.cpp
        server/hl2mp/hl2mp_bot_temp.cpp
        server/hl2mp/hl2mp_client.cpp
        server/hl2mp/hl2mp_cvars.cpp
        server/hl2mp/hl2mp_gameinterface.cpp
        shared/hl2mp/hl2mp_gamerules.cpp
        server/hl2mp/hl2mp_player.cpp
        shared/hl2mp/hl2mp_player_shared.cpp
        shared/hl2mp/hl2mp_weapon_parse.cpp
        server/hl2mp/grenade_satchel.cpp
        server/hl2mp/grenade_tripmine.cpp
        server/hl2mp/te_hl2mp_shotgun_shot.cpp
        shared/hl2mp/weapon_357.cpp
        shared/hl2mp/weapon_ar2.cpp
        shared/hl2mp/weapon_crossbow.cpp
        shared/hl2mp/weapon_crowbar.cpp
        shared/hl2mp/weapon_frag.cpp
        shared/hl2mp/weapon_hl2mpbase.cpp
        shared/hl2mp/weapon_hl2mpbase_machinegun.cpp
        shared/hl2mp/weapon_hl2mpbasebasebludgeon.cpp
        shared/hl2mp/weapon_hl2mpbasehlmpcombatweapon.cpp
        shared/hl2mp/weapon_physcannon.cpp
        shared/hl2mp/weapon_pistol.cpp
        shared/hl2mp/weapon_rpg.cpp
        shared/hl2mp/weapon_shotgun.cpp
        shared/hl2mp/weapon_slam.cpp
        shared/hl2mp/weapon_smg1.cpp
        shared/hl2mp/weapon_stunstick.cpp
        server/trigger_portal.cpp
        shared/cam_thirdperson.cpp
        shared/particle_property.cpp
        shared/voice_status.cpp
        shared/voice_banmgr.cpp
        server/physgun.cpp
        server/base_gameinterface.cpp
        FROM SERVER_SOURCES)

if (MSVC)
    ADD_MSVC_PRECOMPILED_HEADER(" server/stdafx.h" "server/stdafx.cpp" SERVER_SOURCES)
endif ()

add_library(server SHARED ${SERVER_SOURCES})
add_dependencies(server tier1 libcryptopp)

target_link_libraries(server -m32 -Wl,--no-undefined ${SHARED_LIBS})
target_compile_definitions(server PUBLIC ${SHARED_DEFINITIONS} -DGAME_DLL -DSDK_DLL)
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
LIST(APPEND CLIENT_SOURCES ${SHARED_SOURCES})
ucm_remove_directories("client/sdk" "client/replay" "client/ShaderEditor" "client/hl2mp" FROM CLIENT_SOURCES)
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
ucm_remove_files(
        client/MonitorMaterialProxy.cpp
        client/c_forcefeedback.cpp
        client/hud_basedeathnotice.cpp
        client/hud_controlpointicons.cpp
        client/hud_voicestatus.cpp
        client/in_forcefeedback.cpp
        client/hud_vote.cpp
        client/particle_prototype.cpp
        client/thermalmaterialproxy.cpp
        client/vgui_entityimagepanel.cpp
        client/vgui_imagehealthpanel.cpp
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
        client/basepresence_xbox.cpp
        client/episodic/c_prop_scalable.cpp
        client/hl2/c_corpse.cpp
        client/hl2/clientmode_hlnormal.cpp
        client/hl2/fx_hl2_impacts.cpp
        client/hl2/hl_prediction.cpp
        client/hl2/vgui_rootpanel_hl2.cpp
        client/hl2/hl2_clientmode.cpp
        shared/hl2/basehlcombatweapon_shared.cpp
        client/hl2/c_basehlcombatweapon.cpp
        client/hl2/c_weapon__stubs_hl2.cpp
        client/hl2/c_weapon_physcannon.cpp
        client/hl2/c_weapon_stunstick.cpp
        shared/ModelSoundsCache.cpp
        shared/SceneCache.cpp
        client/hl2/hl_in_main.cpp
        client/vgui_textmessagepanel.cpp

        # Momentum related
        client/momentum/ui/ContactPanel.cpp
        FROM CLIENT_SOURCES)
if (MSVC)
    ADD_MSVC_PRECOMPILED_HEADER("client/stdafx.h" "client/stdafx.cpp" CLIENT_SOURCES)
endif ()

add_library(client SHARED ${CLIENT_SOURCES})
add_dependencies(client tier1 gameui libcryptopp raytrace)

find_library(BITMAP_LIBRARY bitmap.a PATHS ${SHARED_LINK_DIRS})
find_library(MATSYS_LIBRARY matsys_controls.a PATHS ${SHARED_LINK_DIRS})
find_library(VTF_LIBRARY vtf.a PATHS ${SHARED_LINK_DIRS})

target_link_libraries(client -m32 -Wl,--no-undefined
        ${BITMAP_LIBRARY}
        ${MATSYS_LIBRARY}
        ${VTF_LIBRARY}
        ${DISCORD_LIBRARY}
        ${SHARED_LIBS}
        vgui_controls
        gameui
        raytrace
        )

if (CMAKE_HOST_WIN32)
    find_library(FMOD_LIBRARY fmod PATHS ${SHARED_LINK_DIRS})
    target_link_libraries(client ${FMOD_LIBRARY})
endif ()

target_compile_definitions(client PUBLIC
        ${SHARED_DEFINITIONS}
        -DCLIENT_DLL -DHL2_CLIENT_DLL -DENABLE_HTMLWINDOW
        -DNO_STRING_T
        -DHL2MP
        -DVERSION_SAFE_STEAM_API_INTERFACES
        -DSDK_DLL
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
        "${CMAKE_SOURCE_DIR}/gameui"
        "${CMAKE_SOURCE_DIR}/thirdparty"
        "${CMAKE_SOURCE_DIR}/thirdparty/discord-rpc"
        ${SHARED_INCLUDE_DIRS}
        )

if (CMAKE_HOST_UNIX)
    # Linux library name is client.so
    set_target_properties(client PROPERTIES PREFIX "")
    set_target_properties(client PROPERTIES OUTPUT_NAME client)

    target_link_libraries(client steam_api tier0 vstdlib)

    install(TARGETS client LIBRARY DESTINATION "bin")
else ()
    install(TARGETS client RUNTIME DESTINATION "bin ")
endif ()
