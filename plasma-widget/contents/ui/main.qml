import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

PlasmoidItem {
    id: root
    
    width: 350
    height: 380

    readonly property var crosshairBackend: backendLoader.item

    Loader {
        id: backendLoader
        active: false
        source: "BackendWrapper.qml"

        onStatusChanged: {
            if (backendLoader.status === Loader.Ready && backendLoader.item) {
                console.log("[CROSSHAIR_UI] Backend is loaded. Flashing logs cache...");
                
                backendLoader.item.flushCache();
            }
        }
    }


    fullRepresentation: Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 8

            PlasmaComponents.Label { text: "Offset settings:"; font.bold: true }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                enabled: root.crosshairBackend !== null 
                
                PlasmaComponents.Label { text: "X Offset:" }
                SpinBox {
                    from: -2048; to: 2048
                    value: root.crosshairBackend ? root.crosshairBackend.offsetX : 0
                    onValueChanged: {
                        if (root.crosshairBackend && root.crosshairBackend.offsetX !== value) {
                            root.crosshairBackend.offsetX = value;
                        }
                    }
                }

                PlasmaComponents.Label { text: "Y Offset:" }
                SpinBox {
                    from: -2048; to: 2048
                    value: root.crosshairBackend ? root.crosshairBackend.offsetY : 0
                    onValueChanged: {
                        if (root.crosshairBackend && root.crosshairBackend.offsetY !== value) {
                            root.crosshairBackend.offsetY = value;
                        }
                    }
                }
            }

            PlasmaComponents.Label { text: "Toggle Hotkey:"; font.bold: true }
            
            PlasmaComponents.TextField {
                id: hotkeyField
                Layout.fillWidth: true
                enabled: root.crosshairBackend !== null
                placeholderText: "Press key sequence..."
                
                text: root.crosshairBackend ? root.crosshairBackend.hotkey : ""
                readOnly: true
                
                background: Rectangle {
                    color: hotkeyField.activeFocus ? "#2c3e50" : "#2d3436"
                    border.color: hotkeyField.activeFocus ? "#3498db" : "#7f8c8d"
                    border.width: hotkeyField.activeFocus ? 2 : 1
                    radius: 4
                }

                Keys.onPressed: (event) => {
                    if (event.key === Qt.Key_Control || event.key === Qt.Key_Alt || 
                        event.key === Qt.Key_Shift || event.key === Qt.Key_Meta) {
                        return;
                    }

                    var modifiers = [];
                    if (event.modifiers & Qt.ControlModifier) modifiers.push("Ctrl");
                    if (event.modifiers & Qt.AltModifier) modifiers.push("Alt");
                    if (event.modifiers & Qt.ShiftModifier) modifiers.push("Shift");
                    if (event.modifiers & Qt.MetaModifier) modifiers.push("Meta");

                    var keyText = String.fromCharCode(event.key).toUpperCase();
                    
                    if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
                        if (root.crosshairBackend) root.crosshairBackend.hotkey = "";
                        event.accepted = true;
                        return;
                    }

                    if (keyText !== "") {
                        modifiers.push(keyText);
                        if (root.crosshairBackend) {
                            root.crosshairBackend.hotkey = modifiers.join("+");
                        }
                    }
                    event.accepted = true;
                }
            }

            Item { Layout.preferredHeight: 5 }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6
                enabled: root.crosshairBackend !== null

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: (root.crosshairBackend && root.crosshairBackend.isRunning) ? "Stop Overlay" : "Start Overlay"
                    checked: root.crosshairBackend ? root.crosshairBackend.isRunning : false
                    onClicked: if (root.crosshairBackend) root.crosshairBackend.toggleOverlay()
                }

                PlasmaComponents.Button {
                    text: "Save Settings"
                    onClicked: if (root.crosshairBackend) root.crosshairBackend.saveSettings()
                }
            }

            PlasmaComponents.Label { text: "Processes log:"; font.bold: true }
            
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ScrollBar.vertical.policy: ScrollBar.AsNeeded

                TextArea {
                    id: logMonitor
                    readOnly: true
                    font.family: "monospace"
                    font.pointSize: 8
                    background: Rectangle { color: "#1e1e1e" }
                    color: "#00C3FF"
                    wrapMode: TextArea.Wrap
                    width: parent.width

                    Component.onCompleted: {
                        console.log("[CROSSHAIR_LOG] Interface is fully built. Connecting the C++ backend....");
                        
                        backendLoader.active = true;
                    }

                    Connections {
                        target: root.crosshairBackend
                        
                        function onLogMessage(msg) {
                            logMonitor.append(msg);
                        }
                    }
                }
            }
            
            PlasmaComponents.Button {
                Layout.fillWidth: true
                text: "Open editor"
                enabled: root.crosshairBackend !== null
                onClicked: {
                    if (root.crosshairBackend) {
                        root.crosshairBackend.openEditorApp();
                    }
                }
            }
        }
    }
}
