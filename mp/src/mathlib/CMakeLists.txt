project(mathlib)

set(MATHLIB_SOURCES
        3dnow.cpp
        almostequal.cpp
        anorms.cpp
        bumpvects.cpp
        color_conversion.cpp
        halton.cpp
        IceKey.cpp
        imagequant.cpp
        lightdesc.cpp
        mathlib_base.cpp
        polyhedron.cpp
        powsse.cpp
        quantize.cpp
        randsse.cpp
        simdvectormatrix.cpp
        sparse_convolution_noise.cpp
        spherical.cpp
        sse.cpp
        sseconst.cpp
        ssenoise.cpp
        vector.cpp
        vmatrix.cpp
        )

add_library(mathlib STATIC ${MATHLIB_SOURCES})

target_compile_definitions(mathlib PRIVATE _LIB MATHLIB_LIB)

target_include_directories(mathlib PRIVATE
        "${CMAKE_SOURCE_DIR}/common"
        "${CMAKE_SOURCE_DIR}/public"
        "${CMAKE_SOURCE_DIR}/public/tier0"
        "${CMAKE_SOURCE_DIR}/public/tier1"
        "${CMAKE_SOURCE_DIR}/public/mathlib"
)