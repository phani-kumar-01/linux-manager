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
                    text: "🔋 Battery Health & Power Profiles"
                    font.pixelSize: 22
                    font.bold: true
                    color: Theme.textPrimary
                }
                Text {
                    text: "Monitor hardware battery degradation, wear levels, charge cycles, and switch power profiles"
                    font.pixelSize: 13
                    color: Theme.textSecondary
                }
            }
            Item { Layout.fillWidth: true }
        }

        // Battery Cards Row
        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            StatCard {
                Layout.fillWidth: true
                title: "Battery Health / Wear"
                value: "92.4%"
                subtitle: "66 Wh / 72 Wh design"
                iconText: "🔋"
                accentColor: Theme.success
            }

            StatCard {
                Layout.fillWidth: true
                title: "Charge Cycles"
                value: "142"
                subtitle: "Good lifecycle state"
                iconText: "🔄"
                accentColor: Theme.info
            }

            StatCard {
                Layout.fillWidth: true
                title: "Discharge Rate"
                value: "11.2 W"
                subtitle: "Estimated 4h 15m remaining"
                iconText: "⚡"
                accentColor: Theme.warning
            }
        }

        // Power Profile Switcher
        Text {
            text: "Active Power Profile"
            font.pixelSize: 17
            font.bold: true
            color: Theme.textPrimary
        }

        Rectangle {
            Layout.fillWidth: true
            height: 110
            color: Theme.bgCard
            radius: Theme.radiusMd
            border.color: Theme.borderBase
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Repeater {
                    model: [
                        { name: "Power Saver", desc: "Reduces CPU clocks to maximize battery", icon: "🌱", mode: "power-saver" },
                        { name: "Balanced", desc: "Standard dynamic performance scaling", icon: "⚖️", mode: "balanced", isDefault: true },
                        { name: "Performance", desc: "Maximum CPU frequencies and responsiveness", icon: "🚀", mode: "performance" }
                    ]

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: modelData.isDefault ? Theme.accent + "22" : Theme.bgSurface
                        radius: Theme.radiusSm
                        border.color: modelData.isDefault ? Theme.accent : Theme.borderBase
                        border.width: modelData.isDefault ? 2 : 1

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                console.log("Selected profile " + modelData.mode)
                            }
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12

                            Text {
                                text: modelData.icon
                                font.pixelSize: 24
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                Text {
                                    text: modelData.name
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: Theme.textPrimary
                                }
                                Text {
                                    text: modelData.desc
                                    font.pixelSize: 11
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
