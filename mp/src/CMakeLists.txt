project(momentum)

cmake_minimum_required(VERSION 3.0.0)
set(MOMENTUM_VERSION "0.8.1")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include("ucm")
include("ExternalProject")

# Default to a release build
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE
        STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Global definitions
add_definitions(-DVPROF_LEVEL=1 -DSWDS -D_snprintf=use_Q_snprintf_instead -DRAD_TELEMETRY_DISABLED -D_USRDLL -DVECTOR)

# Platform specific definitions
if(CMAKE_HOST_UNIX)
  add_definitions(-D_LINUX -DLINUX -D_POSIX -DPOSIX -DGNUC -DCOMPILER_GCC -DUSE_SDL -DVPROF_LEVEL=1 -DGNUC -DNO_HOOK_MALLOC
    -DNO_MALLOC_OVERRIDE -D_DLL_EXT=.so)

  add_compile_options(-w -std=gnu++0x -fpermissive -fno-strict-aliasing -ffast-math -fno-omit-frame-pointer
    -ftree-vectorize -fpredictive-commoning -funswitch-loops -m32 -march=core2 -mtune=core2 -msse2 -mfpmath=sse)

#  add_compile_options(-Wall -Wno-invalid-offsetof -Wno-multichar -Wno-overloaded-virtual -Wno-narrowing -Wno-write-strings
#  	-Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-local-typedefs -Wno-unused-result
#    -Wno-unknown-pragmas -Wno-unused-parameter -Wno-unused-value -Wno-missing-field-initializers
#    -Wno-sign-compare -Wno-reorder -Wno-invalid-offsetof -Wno-float-equal -Werror=return-type -Wno-attributes
#    -fdiagnostics-show-option -Wformat -Wformat-security)
elseif(MSVC)
  add_definitions(-DWIN32 -D_WIN32 -D_WINDOWS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_DLL_EXT=.dll)
endif()


# Add in the projects
add_subdirectory("mathlib")
add_subdirectory("tier1")
add_subdirectory("vgui2")
add_subdirectory("gameui")
add_subdirectory("cryptopp")
add_subdirectory("raytrace")
add_subdirectory("game")
