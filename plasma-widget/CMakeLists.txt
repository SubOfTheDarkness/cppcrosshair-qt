qt_policy(SET QTP0001 NEW)

set(PLUGIN_URI "crosshair_plugin")

qt_add_qml_module(crosshair_plugin
    SHARED
    URI "${PLUGIN_URI}"
    VERSION 1.0
    SOURCES
        crosshair_manager.h
        crosshair_manager.cpp
)

set_target_properties(crosshair_plugin PROPERTIES 
    EXCLUDE_FROM_ALL FALSE
)

target_link_libraries(crosshair_plugin PRIVATE
    Qt6::Core
    Qt6::Qml
    X11::X11
)

set_target_properties(crosshair_plugin PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
)

set(LOCAL_PLASMOID_DIR "$ENV{HOME}/.local/share/plasma/plasmoids/com.github.subofthedarkness.cppcrosshair")
set(LOCAL_BACKEND_DIR "${LOCAL_PLASMOID_DIR}/contents/backend")

install(CODE "
    message(STATUS \"-> [WIDGET] Installing KDE Plasma 6 widget to: ${LOCAL_PLASMOID_DIR}\")
    
    execute_process(COMMAND cmake -E rm -rf \"${LOCAL_PLASMOID_DIR}\")
    execute_process(COMMAND cmake -E make_directory \"${LOCAL_BACKEND_DIR}\")
    execute_process(COMMAND cmake -E make_directory \"${LOCAL_PLASMOID_DIR}/contents/ui\")
    
    file(INSTALL FILES \"${CMAKE_CURRENT_SOURCE_DIR}/metadata.json\" DESTINATION \"${LOCAL_PLASMOID_DIR}\")
    
    file(INSTALL FILES \"${CMAKE_CURRENT_SOURCE_DIR}/contents/ui/main.qml\" DESTINATION \"${LOCAL_PLASMOID_DIR}/contents/ui\")
    
    file(INSTALL FILES \"${CMAKE_CURRENT_BINARY_DIR}/qmldir\" DESTINATION \"${LOCAL_BACKEND_DIR}\")
    file(INSTALL FILES \"${CMAKE_CURRENT_BINARY_DIR}/crosshair_plugin.qmltypes\" DESTINATION \"${LOCAL_BACKEND_DIR}\")
    
    file(INSTALL FILES \"${CMAKE_CURRENT_BINARY_DIR}/libcrosshair_pluginplugin.so\" DESTINATION \"${LOCAL_BACKEND_DIR}\")
    file(INSTALL FILES \"${CMAKE_CURRENT_BINARY_DIR}/libcrosshair_plugin.so\" DESTINATION \"${LOCAL_BACKEND_DIR}\")
    
    file(APPEND \"${LOCAL_BACKEND_DIR}/qmldir\" \"\\nclassname CrosshairPlugin\\n\")
" COMPONENT widget)
