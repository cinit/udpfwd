cmake_minimum_required(VERSION 3.10...3.27)

project(udpfwd)

enable_language(CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc -static -static-pie ")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=format-invalid-specifier -Werror=return-type -Wno-invalid-offsetof")
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    SET(CLANG_CXX_EXTRA_OPT "-Werror=unknown-warning-option -Werror=format-invalid-specifier -Werror=call-to-pure-virtual-from-ctor-dtor")
    SET(CLANG_C_EXTRA_OPT "-Werror=format-invalid-specifier")
else ()
    SET(CLANG_CXX_EXTRA_OPT "")
    SET(CLANG_C_EXTRA_OPT "")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_CXX_EXTRA_OPT} -fPIC -Werror=return-type -Wno-invalid-offsetof -frtti -fexceptions")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CLANG_C_EXTRA_OPT} -fPIC -Werror=return-type")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-allow-shlib-undefined,--no-undefined -Wl,-z,defs,-z,now,-z,relro")

# add -fno-omit-frame-pointer -Werror=non-virtual-dtor -Werror=delete-non-virtual-dtor
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -Werror=non-virtual-dtor -Werror=delete-non-virtual-dtor")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer")

add_subdirectory(libs/fmt fmt)

target_compile_definitions(fmt-header-only INTERFACE FMT_STATIC_THOUSANDS_SEPARATOR=1)

add_executable(udpfwd src/platform/log/Log.cpp src/support/text/TextUtils.cc src/support/text/ParseNumber.cc src/platform/Runtime.cc
        src/platform/Time.cpp src/common/INetAddress.cc src/common/INetSocketAddress.cc src/platform/IoUtils.cc src/platform/ErrorMsg.cc
        src/udpfwd/UdpForwardWorker.cc src/udpfwd/UdpForward.cpp src/wgcf/WgcfRelay.cc src/wgcf/WgcfRelayWorker.cc
        src/common/Main.cc src/common/DebugConfig.cc
        src/veth/VEtherForward.cc src/veth/EtherFrame.cc src/veth/EtherPool.cc src/veth/BaseTapEndpoint.cc
        src/veth/TapInterface.cc
)

target_include_directories(udpfwd PRIVATE src)

target_link_libraries(udpfwd fmt-header-only)
