cmake_minimum_required(VERSION 3.16)
project(CrosshairToolKit VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

find_package(X11 REQUIRED)

if(NOT X11_Xpm_FOUND)
    message(FATAL_ERROR "libXpm development package not found! Please install libxpm-dev (Debian/LMDE) or libxpm (Arch/Garuda).")
endif()

add_executable(crosshair_overlay overlay_main.cpp)

set_target_properties(crosshair_overlay PROPERTIES 
    AUTOMOC OFF 
    AUTOUIC OFF
)

target_link_libraries(crosshair_overlay PRIVATE 
    X11::X11 
    X11::Xext
    Xpm
)

target_compile_options(crosshair_overlay PRIVATE -Os -s)


add_executable(crosshair_editor
    editor_main.cpp
    overlay_editor_window.h
    overlay_editor_window.cpp
    overlay_editor_window.ui
    pixel_canvas.h
    pixel_canvas.cpp
)
target_include_directories(crosshair_editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(crosshair_editor PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)


install(TARGETS crosshair_editor crosshair_overlay DESTINATION bin)

install(FILES cppcrosshair.desktop DESTINATION share/applications)

install(FILES icon.png DESTINATION share/pixmaps RENAME cppcrosshair.png)

set(CPACK_PACKAGE_NAME "cppcrosshair-toolkit")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_RELEASE "1")
set(CPACK_PACKAGE_CONTACT "SubOfTheDarkness <204970490+SubOfTheDarkness@users.noreply.github.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Pixel crosshair manager: standalone lightweight X11 overlay and Qt6 editor")
set(CPACK_PACKAGE_VENDOR "SubOfTheDarkness")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libqt6widgets6 (>= 6.0), libx11-6, libxext6, libxpm4, procps")

include(CPack)
