import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "."

Item {
    id: root

    Column {
        anchors.fill: parent
        anchors.margins: Theme.spacingLarge
        spacing: Theme.spacingMedium

        Row {
            width: parent.width
            spacing: Theme.spacingMedium

            Text {
                text: "Storage Analyzer"
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeHeading
                font.bold: true
                font.family: Theme.fontFamily
                anchors.verticalCenter: parent.verticalCenter
            }

            Item { Layout.fillWidth: true }

            ActionButton {
                text: "Scan Large Files (>100 MB)"
                variant: "secondary"
                icon: "🔍"
            }
        }

        // Partition Cards
        Row {
            spacing: Theme.spacingMedium
            width: parent.width

            Rectangle {
                width: 320
                height: 100
                radius: Theme.radiusMedium
                color: Theme.bgCard
                border.color: Theme.borderSubtle
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: Theme.spacingMedium
                    spacing: 6

                    Text { text: "Root Filesystem (/dev/nvme0n1p2)"; color: Theme.textSecondary; font.pixelSize: Theme.fontSizeSmall }
                    Text { text: "184 GB / 512 GB Used (36%)"; color: Theme.textPrimary; font.bold: true; font.pixelSize: Theme.fontSizeLarge }
                    ProgressBarCustom { value: 36; width: parent.width; barColor: Theme.primary }
                }
            }
        }

        // Storage Tree / Breakdown
        Rectangle {
            width: parent.width
            height: parent.height - 180
            radius: Theme.radiusMedium
            color: Theme.bgCard
            border.color: Theme.borderSubtle
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: Theme.spacingMedium
                spacing: Theme.spacingMedium

                Text {
                    text: "Directory Breakdown (~ / Home)"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeLarge
                    font.bold: true
                    font.family: Theme.fontFamily
                }

                ListView {
                    width: parent.width
                    height: parent.height - 50
                    clip: true
                    spacing: Theme.spacingSmall

                    model: [
                        { name: "Projects", path: "~/Projects", size: "42.5 GB", pct: 42 },
                        { name: "Downloads", path: "~/Downloads", size: "14.2 GB", pct: 14 },
                        { name: ".cache", path: "~/.cache", size: "8.6 GB", pct: 8 },
                        { name: ".local", path: "~/.local", size: "7.1 GB", pct: 7 },
                        { name: "Documents", path: "~/Documents", size: "3.4 GB", pct: 3 },
                        { name: "Pictures", path: "~/Pictures", size: "1.8 GB", pct: 2 }
                    ]

                    delegate: Rectangle {
                        width: parent.width
                        height: 52
                        radius: Theme.radiusSmall
                        color: Theme.bgInput

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: Theme.spacingMedium
                            spacing: Theme.spacingMedium

                            Text { text: "📁 " + modelData.name; color: Theme.textPrimary; font.bold: true; Layout.preferredWidth: 160 }
                            Text { text: modelData.path; color: Theme.textMuted; font.family: "Monospace"; Layout.preferredWidth: 200 }

                            ProgressBarCustom {
                                value: modelData.pct
                                Layout.fillWidth: true
                                barColor: Theme.accent
                            }

                            Text { text: modelData.size; color: Theme.textPrimary; font.bold: true; Layout.preferredWidth: 80; horizontalAlignment: Text.AlignRight }
                        }
                    }
                }
            }
        }
    }
}
