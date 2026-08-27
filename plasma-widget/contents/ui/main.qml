import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

import "../backend" 1.0

PlasmoidItem {
    id: root
    
    width: 350
    height: 380

    CrosshairManager {
        id: crosshairBackend
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
                
                PlasmaComponents.Label { text: "X Offset:" }
                SpinBox {
                    from: -2048; to: 2048
                    value: crosshairBackend.offsetX
                    onValueChanged: {
                        if (crosshairBackend.offsetX !== value) {
                            crosshairBackend.offsetX = value;
                        }
                    }
                }

                PlasmaComponents.Label { text: "Y Offset:" }
                SpinBox {
                    from: -2048; to: 2048
                    value: crosshairBackend.offsetY
                    onValueChanged: {
                        if (crosshairBackend.offsetY !== value) {
                            crosshairBackend.offsetY = value;
                        }
                    }
                }
            }

            PlasmaComponents.Label { text: "Toggle hotkey:"; font.bold: true }
            
            PlasmaComponents.TextField {
                id: hotkeyField
                Layout.fillWidth: true
                placeholderText: "Press keq sequence..."
                
                text: crosshairBackend.hotkey
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
                        crosshairBackend.hotkey = "";
                        event.accepted = true;
                        return;
                    }

                    if (keyText !== "") {
                        modifiers.push(keyText);
                        crosshairBackend.hotkey = modifiers.join("+");
                    }
                    event.accepted = true;
                }
            }

            Item { Layout.preferredHeight: 5 }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: crosshairBackend.isRunning ? "Stop Overlay" : "Start Overlay"
                    checked: crosshairBackend.isRunning
                    onClicked: crosshairBackend.toggleOverlay()
                }

                PlasmaComponents.Button {
                    text: "Save Settings"
                    onClicked: crosshairBackend.saveSettings()
                }
            }

            PlasmaComponents.Label { text: "Processes logs:"; font.bold: true }
            
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

                    Connections {
                        target: crosshairBackend
                        
                        function onLogMessage(msg) {
                            logMonitor.append(msg);
                        }
                    }

                    Component.onCompleted: {
                        crosshairBackend.flushCache();
                    }
                }
            }
            
            PlasmaComponents.Button {
                Layout.fillWidth: true
                text: "Open editor"
                onClicked: {
                    crosshairBackend.openEditorApp();
                }
            }
        }
    }
}
