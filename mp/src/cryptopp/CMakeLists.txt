project(libcryptopp)

ucm_add_dirs("." TO CRYPTOPP_SOURCES NO_HEADERS RECURSIVE)

if (MSVC)
    ADD_MSVC_PRECOMPILED_HEADER("pch.cpp" "pch.h" CRYPTOPP_SOURCES)
endif ()

add_library(libcryptopp STATIC ${CRYPTOPP_SOURCES})

set(SHARED_DEFINITIONS -DCRYPTOPP_DISABLE_AVX -DCRYPTOPP_DISABLE_AVX2)
if (MSVC)
    target_compile_definitions(libcryptopp PRIVATE ${SHARED_DEFINITIONS})
else ()
    target_compile_definitions(libcryptopp PRIVATE ${SHARED_DEFINITIONS} -DCRYPTOPP_DISABLE_AESNI -DCRYPTOPP_DISABLE_SSSE3 -DCRYPTOPP_DISABLE_SSSE4)
endif ()

target_include_directories(libcryptopp PRIVATE ${CMAKE_SOURCE_DIR}/public/cryptopp)