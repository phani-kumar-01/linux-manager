import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "."

ScrollView {
    id: root

    contentWidth: width
    clip: true

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Theme.spacingLarge
        spacing: Theme.spacingLarge

        // Welcome Header
        Row {
            width: parent.width

            Column {
                spacing: 4

                Text {
                    text: "System Overview"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeHeading
                    font.bold: true
                    font.family: Theme.fontFamily
                }

                Text {
                    text: "Ubuntu 24.04 LTS • Linux 6.8.0-generic"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeNormal
                    font.family: Theme.fontFamily
                }
            }
        }

        // Top Metrics Grid
        Flow {
            width: parent.width
            spacing: Theme.spacingMedium

            StatCard {
                title: "CPU Utilization"
                value: "18%"
                subtitle: "51°C • 16 Cores"
                accentColor: Theme.primary
                progress: 0.18
                iconText: "⚡"
            }

            StatCard {
                title: "Memory (RAM)"
                value: "7.2 / 16 GB"
                subtitle: "45% Allocated"
                accentColor: Theme.accent
                progress: 0.45
                iconText: "💾"
            }

            StatCard {
                title: "Disk Storage"
                value: "184 / 512 GB"
                subtitle: "328 GB Free"
                accentColor: Theme.info
                progress: 0.36
                iconText: "💿"
            }

            StatCard {
                title: "Cleanable Cache"
                value: "3.8 GB"
                subtitle: "System & Dev Caches"
                accentColor: Theme.warning
                progress: -1
                iconText: "🧹"
            }
        }

        // Status & Alerts Grid
        RowLayout {
            width: parent.width
            spacing: Theme.spacingMedium

            // System Health & Updates Card
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 200
                radius: Theme.radiusMedium
                color: Theme.bgCard
                border.color: Theme.borderSubtle
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: Theme.spacingMedium
                    spacing: Theme.spacingMedium

                    Text {
                        text: "System Maintenance Status"
                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontSizeLarge
                        font.bold: true
                        font.family: Theme.fontFamily
                    }

                    Row {
                        spacing: Theme.spacingLarge

                        Column {
                            spacing: 4
                            Text { text: "Pending Updates"; color: Theme.textSecondary; font.pixelSize: Theme.fontSizeSmall }
                            Text { text: "14 Available (4 Security)"; color: Theme.warning; font.pixelSize: Theme.fontSizeMedium; font.bold: true }
                        }

                        Column {
                            spacing: 4
                            Text { text: "Failed Services"; color: Theme.textSecondary; font.pixelSize: Theme.fontSizeSmall }
                            Text { text: "0 Units Failed"; color: Theme.success; font.pixelSize: Theme.fontSizeMedium; font.bold: true }
                        }

                        Column {
                            spacing: 4
                            Text { text: "System Uptime"; color: Theme.textSecondary; font.pixelSize: Theme.fontSizeSmall }
                            Text { text: "3 days, 6 hours"; color: Theme.textPrimary; font.pixelSize: Theme.fontSizeMedium; font.bold: true }
                        }
                    }

                    Row {
                        spacing: Theme.spacingMedium
                        ActionButton {
                            text: "Check Updates"
                            variant: "primary"
                            icon: "🔄"
                        }
                        ActionButton {
                            text: "Quick System Cleanup"
                            variant: "secondary"
                            icon: "🧹"
                        }
                    }
                }
            }
        }
    }
}
