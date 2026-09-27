import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "."

ScrollView {
    id: root
    contentWidth: availableWidth
    clip: true

    ColumnLayout {
        width: parent.width
        spacing: 20

        // Header
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 4
                Text {
                    text: "🚀 Startup & Autostart Applications"
                    font.pixelSize: 22
                    font.bold: true
                    color: Theme.textPrimary
                }
                Text {
                    text: "Manage applications and background services launched automatically on login"
                    font.pixelSize: 13
                    color: Theme.textSecondary
                }
            }
            Item { Layout.fillWidth: true }
        }

        // List of Autostart items
        Rectangle {
            Layout.fillWidth: true
            height: 320
            color: Theme.bgCard
            radius: Theme.radiusMd
            border.color: Theme.borderBase
            border.width: 1

            ListView {
                anchors.fill: parent
                anchors.margins: 12
                clip: true
                spacing: 10
                model: [
                    { name: "Discord", comment: "All-in-one voice and text chat", exec: "/usr/bin/discord", isEnabled: true, icon: "💬" },
                    { name: "Docker Desktop", comment: "Docker Container Manager", exec: "/usr/bin/docker-desktop", isEnabled: false, icon: "🐳" },
                    { name: "Steam", comment: "Application for managing and playing games", exec: "/usr/bin/steam -silent", isEnabled: true, icon: "🎮" },
                    { name: "Slack", comment: "Slack Desktop App", exec: "/usr/bin/slack", isEnabled: true, icon: "💼" }
                ]

                delegate: Rectangle {
                    width: parent.width
                    height: 60
                    color: Theme.bgSurface
                    radius: Theme.radiusSm

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 14

                        Text {
                            text: modelData.icon
                            font.pixelSize: 24
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3
                            Text {
                                text: modelData.name
                                font.pixelSize: 14
                                font.bold: true
                                color: Theme.textPrimary
                            }
                            Text {
                                text: modelData.comment + " (" + modelData.exec + ")"
                                font.pixelSize: 11
                                color: Theme.textSecondary
                                elide: Text.ElideRight
                            }
                        }

                        Switch {
                            checked: modelData.isEnabled
                            onToggled: {
                                console.log("Toggled " + modelData.name + " to " + checked)
                            }
                        }
                    }
                }
            }
        }
    }
}
