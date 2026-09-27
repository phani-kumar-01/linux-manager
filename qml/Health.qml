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

        Text {
            text: "System Health & Hardware Telemetry"
            color: Theme.textPrimary
            font.pixelSize: Theme.fontSizeHeading
            font.bold: true
            font.family: Theme.fontFamily
        }

        // Hardware Summary Cards
        Flow {
            width: parent.width
            spacing: Theme.spacingMedium

            StatCard {
                title: "Processor"
                value: "18% Load"
                subtitle: "13th Gen Intel Core i7-13700H"
                accentColor: Theme.primary
                progress: 0.18
                iconText: "⚡"
            }

            StatCard {
                title: "System Temperature"
                value: "48.0°C"
                subtitle: "Thermal Zone Normal"
                accentColor: Theme.success
                progress: 0.48
                iconText: "🌡️"
            }

            StatCard {
                title: "Memory Available"
                value: "8.8 GB Free"
                subtitle: "16 GB Total Physical RAM"
                accentColor: Theme.accent
                progress: 0.45
                iconText: "💾"
            }

            StatCard {
                title: "Battery Health"
                value: "95% Level"
                subtitle: "Discharging • Good Condition"
                accentColor: Theme.warning
                progress: 0.95
                iconText: "🔋"
            }
        }

        // Detailed System Spec Card
        Rectangle {
            width: parent.width
            height: 220
            radius: Theme.radiusMedium
            color: Theme.bgCard
            border.color: Theme.borderSubtle
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: Theme.spacingMedium
                spacing: Theme.spacingSmall

                Text {
                    text: "Platform & Kernel Architecture"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeLarge
                    font.bold: true
                    font.family: Theme.fontFamily
                }

                Grid {
                    columns: 2
                    columnSpacing: Theme.spacingLarge
                    rowSpacing: 8
                    width: parent.width

                    Text { text: "Operating System:"; color: Theme.textSecondary; font.pixelSize: Theme.fontSizeNormal; width: 140 }
                    Text { text: "Ubuntu 24.04 LTS (Noble Numbat)"; color: Theme.textPrimary; font.bold: true; font.pixelSize: Theme.fontSizeNormal }

                    Text { text: "Linux Kernel:"; color: Theme.textSecondary; font.pixelSize: Theme.fontSizeNormal; width: 140 }
                    Text { text: "6.8.0-40-generic (x86_64)"; color: Theme.textPrimary; font.bold: true; font.pixelSize: Theme.fontSizeNormal }

                    Text { text: "Desktop Environment:"; color: Theme.textSecondary; font.pixelSize: Theme.fontSizeNormal; width: 140 }
                    Text { text: "GNOME 46 (Wayland / X11)"; color: Theme.textPrimary; font.bold: true; font.pixelSize: Theme.fontSizeNormal }

                    Text { text: "Package Architecture:"; color: Theme.textSecondary; font.pixelSize: Theme.fontSizeNormal; width: 140 }
                    Text { text: "amd64 (Debian Dpkg + APT Native)"; color: Theme.textPrimary; font.bold: true; font.pixelSize: Theme.fontSizeNormal }
                }
            }
        }
    }
}
