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
                    text: "📦 APT Repositories & PPAs"
                    font.pixelSize: 22
                    font.bold: true
                    color: Theme.textPrimary
                }
                Text {
                    text: "Inspect software sources, third-party PPAs, and detect duplicate or obsolete entries"
                    font.pixelSize: 13
                    color: Theme.textSecondary
                }
            }
            Item { Layout.fillWidth: true }
        }

        // Repository List
        Rectangle {
            Layout.fillWidth: true
            height: 340
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
                    { name: "Ubuntu Main Mirror", uri: "http://archive.ubuntu.com/ubuntu", suite: "noble", isPpa: false, isEnabled: true },
                    { name: "Mozilla Team PPA", uri: "https://ppa.launchpadcontent.net/mozillateam/ppa/ubuntu", suite: "noble", isPpa: true, isEnabled: true },
                    { name: "Docker CE Official", uri: "https://download.docker.com/linux/ubuntu", suite: "noble", isPpa: false, isEnabled: true },
                    { name: "VS Code Repository", uri: "https://packages.microsoft.com/repos/code", suite: "stable", isPpa: false, isEnabled: true }
                ]

                delegate: Rectangle {
                    width: parent.width
                    height: 56
                    color: Theme.bgSurface
                    radius: Theme.radiusSm

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 14

                        Text {
                            text: modelData.isPpa ? "📦" : "🌐"
                            font.pixelSize: 20
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3
                            RowLayout {
                                Text {
                                    text: modelData.name
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: Theme.textPrimary
                                }
                                Text {
                                    text: "[" + modelData.suite + "]"
                                    font.pixelSize: 11
                                    color: Theme.accent
                                }
                            }
                            Text {
                                text: modelData.uri
                                font.pixelSize: 11
                                color: Theme.textSecondary
                                elide: Text.ElideRight
                            }
                        }

                        Switch {
                            checked: modelData.isEnabled
                            onToggled: {
                                console.log("Toggled repo " + modelData.name)
                            }
                        }
                    }
                }
            }
        }
    }
}
