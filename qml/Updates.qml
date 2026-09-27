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
                text: "System Updates"
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeHeading
                font.bold: true
                font.family: Theme.fontFamily
                anchors.verticalCenter: parent.verticalCenter
            }

            Item { Layout.fillWidth: true }

            ActionButton {
                text: "Refresh Repositories"
                variant: "secondary"
                icon: "🔄"
            }

            ActionButton {
                text: "Update All (14)"
                variant: "primary"
                icon: "🚀"
            }
        }

        // Summary Bar
        Rectangle {
            width: parent.width
            height: 48
            radius: Theme.radiusSmall
            color: Theme.bgCard
            border.color: Theme.borderSubtle
            border.width: 1

            Row {
                anchors.centerIn: parent
                spacing: Theme.spacingLarge

                Text { text: "14 Updates Available"; color: Theme.textPrimary; font.bold: true; font.pixelSize: Theme.fontSizeNormal }
                Text { text: "•"; color: Theme.borderSubtle }
                Text { text: "4 Security Updates"; color: Theme.warning; font.bold: true; font.pixelSize: Theme.fontSizeNormal }
                Text { text: "•"; color: Theme.borderSubtle }
                Text { text: "Download Size: ~342 MB"; color: Theme.textSecondary; font.pixelSize: Theme.fontSizeNormal }
            }
        }

        // Updates List
        ListView {
            width: parent.width
            height: parent.height - 150
            clip: true
            spacing: Theme.spacingSmall

            model: [
                { name: "libssl3", currentVer: "3.0.13-0ubuntu3.4", candVer: "3.0.13-0ubuntu3.5", security: true, size: "2.1 MB" },
                { name: "linux-image-generic", currentVer: "6.8.0-38.38", candVer: "6.8.0-40.40", security: true, size: "14.2 MB" },
                { name: "curl", currentVer: "8.5.0-2ubuntu10.1", candVer: "8.5.0-2ubuntu10.3", security: true, size: "220 KB" },
                { name: "python3.12", currentVer: "3.12.3-1ubuntu0.1", candVer: "3.12.3-1ubuntu0.2", security: false, size: "640 KB" },
                { name: "systemd", currentVer: "255.4-1ubuntu8.4", candVer: "255.4-1ubuntu8.5", security: true, size: "4.8 MB" }
            ]

            delegate: Rectangle {
                width: parent.width
                height: 56
                radius: Theme.radiusMedium
                color: Theme.bgCard
                border.color: Theme.borderSubtle
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.spacingMedium
                    spacing: Theme.spacingMedium

                    Rectangle {
                        visible: modelData.security
                        width: 80
                        height: 22
                        radius: 4
                        color: "#7f1d1d"
                        Text {
                            text: "SECURITY"
                            color: "#fca5a5"
                            font.bold: true
                            font.pixelSize: 10
                            anchors.centerIn: parent
                        }
                    }

                    Text {
                        text: modelData.name
                        color: Theme.textPrimary
                        font.bold: true
                        font.pixelSize: Theme.fontSizeNormal
                        Layout.preferredWidth: 200
                    }

                    Text {
                        text: modelData.currentVer + "  ➔  " + modelData.candVer
                        color: Theme.textSecondary
                        font.family: "Monospace"
                        font.pixelSize: Theme.fontSizeSmall
                        Layout.fillWidth: true
                    }

                    Text {
                        text: modelData.size
                        color: Theme.textMuted
                        font.pixelSize: Theme.fontSizeSmall
                    }
                }
            }
        }
    }
}
