cmake_minimum_required (VERSION 3.0)
project (imweb LANGUAGES CXX
         HOMEPAGE_URL https://github.com/utoni/nDPId-Graph
         DESCRIPTION "nDPId + Dear ImGui + ImPlot + WebGui"
         VERSION 0.0.1)
include (ExternalProject)
find_package (PkgConfig REQUIRED)

set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_RUNTIME OFF)
find_package (Boost 1.81.0 COMPONENTS json)

if (Boost_FOUND)
    include_directories (${Boost_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Boost::json is mandatory!")
endif ()

option (ENABLE_LTO "Enable link-time optimizations." OFF)
option (ENABLE_SANITIZER "Enable ASAN/LSAN/UBSAN." OFF)

if (ENABLE_SANITIZER)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -g3 -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -fno-sanitize=alignment -fsanitize=enum -fsanitize=leak")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -g3 -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -fno-sanitize=alignment -fsanitize=enum -fsanitize=leak")
    link_libraries (asan ubsan)
endif ()

if (NOT UNIX)
    message (FATAL_ERROR "Only *nix supported at the moment.")
endif ()

if (ENABLE_LTO)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -ffunction-sections -fdata-sections -Wl,--gc-sections")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -ffunction-sections -fdata-sections -Wl,--gc-sections")
endif ()

if (NOT DEFINED EMSCRIPTEN)
    pkg_check_modules (GL REQUIRED gl)
endif ()
pkg_check_modules (GLFW REQUIRED glfw3)

ExternalProject_Add (imweb SOURCE_DIR ${CMAKE_SOURCE_DIR}/imweb
                           CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/libimweb-install -DBUILD_EXAMPLES=OFF -DSINGLE_LIBRARY=ON -DEMBED_DATA=ON -DENABLE_SANITIZER=${ENABLE_SANITIZER} -DENABLE_LTO=${ENABLE_LTO})

add_executable (nDPId-graph main.cpp controller.cpp json.cpp client.cpp graph.cpp widgets.cpp)
set_property (TARGET nDPId-graph PROPERTY CXX_STANDARD 20)
set_property (TARGET nDPId-graph PROPERTY INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/libimweb-install/include/imweb)
set_property (TARGET nDPId-graph PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/libimweb-install/lib/imweb/libimweb.a)
target_link_libraries (nDPId-graph ${CMAKE_BINARY_DIR}/libimweb-install/lib/imweb/libimweb.a ${pkgcfg_lib_GLFW_glfw} ${pkgcfg_lib_GL_GL} ${Boost_LIBRARIES} pthread dl)
add_dependencies (nDPId-graph imweb)
