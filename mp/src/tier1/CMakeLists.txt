project(tier1)

set(TIER1_SOURCES
        bitbuf.cpp
        byteswap.cpp
        characterset.cpp
        checksum_crc.cpp
        checksum_md5.cpp
        checksum_sha1.cpp
        commandbuffer.cpp
        convar.cpp
        datamanager.cpp
        diff.cpp
        fileio.cpp
        generichash.cpp
        ilocalize.cpp
        interface.cpp
        KeyValues.cpp
        kvpacker.cpp
        lzmaDecoder.cpp
        mempool.cpp
        memstack.cpp
        NetAdr.cpp
        newbitbuf.cpp
        rangecheckedvar.cpp
        reliabletimer.cpp
        snappy.cpp
        snappy-sinksource.cpp
        snappy-stubs-internal.cpp
        sparsematrix.cpp
        splitstring.cpp
        stringpool.cpp
        strtools.cpp
        strtools_unicode.cpp
        tier1.cpp
        tokenreader.cpp
        undiff.cpp
        uniqueid.cpp
        utlbinaryblock.cpp
        utlbuffer.cpp
        utlbufferutil.cpp
        utlstring.cpp
        utlsymbol.cpp
        )

if(CMAKE_HOST_UNIX)
    list(APPEND TIER1_SOURCES
            processor_detect_linux.cpp
            qsort_s.cpp
            pathmatch.cpp
            )
elseif(CMAKE_HOST_WIN32)
    list(APPEND TIER1_SOURCES
            processor_detect.cpp
            )
endif()

add_library(tier1 STATIC ${TIER1_SOURCES})

target_compile_definitions(tier1 PRIVATE _LIB TIER1_STATIC_LIB)

if(CMAKE_HOST_UNIX)
    target_compile_definitions(tier1 PRIVATE POSIX LINUX)
endif()

target_include_directories(tier1 PRIVATE
        "${CMAKE_SOURCE_DIR}/common"
        "${CMAKE_SOURCE_DIR}/public"
        "${CMAKE_SOURCE_DIR}/public/tier0"
        "${CMAKE_SOURCE_DIR}/public/tier1"
        )

if(CMAKE_HOST_WIN32)
    target_link_libraries(tier1 PRIVATE Rpcrt4.lib)
endif()
