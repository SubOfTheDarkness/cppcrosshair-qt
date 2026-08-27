qt_policy(SET QTP0001 NEW)

set(PLUGIN_URI "com.github.subofthedarkness.cppcrosshair")

qt_add_qml_module(crosshair_plugin
    URI "${PLUGIN_URI}"
    VERSION 1.0
    
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/contents/com/github/subofthedarkness/cppcrosshair"
    
    SOURCES
        crosshair_manager.h
        crosshair_manager.cpp
)

target_link_libraries(crosshair_plugin PRIVATE
    Qt6::Core
    Qt6::Qml
    X11::X11
)

set(LOCAL_PLASMOID_DIR "$ENV{HOME}/.local/share/plasma/plasmoids/${PLUGIN_URI}")

install(CODE "
    message(STATUS \"-> Installing KDE Plasma 6 widget to: ${LOCAL_PLASMOID_DIR}\")
    
    execute_process(COMMAND cmake -E rm -rf \"${LOCAL_PLASMOID_DIR}\")
    execute_process(COMMAND cmake -E make_directory \"${LOCAL_PLASMOID_DIR}\")
    
    file(INSTALL FILES \"${CMAKE_CURRENT_SOURCE_DIR}/metadata.json\" DESTINATION \"${LOCAL_PLASMOID_DIR}\")
    
    file(INSTALL DESTINATION \"${LOCAL_PLASMOID_DIR}\" TYPE DIRECTORY FILES \"${CMAKE_CURRENT_SOURCE_DIR}/contents\")
    
    file(INSTALL DESTINATION \"${LOCAL_PLASMOID_DIR}\" TYPE DIRECTORY FILES \"${CMAKE_CURRENT_BINARY_DIR}/contents/\")
" COMPONENT widget)
