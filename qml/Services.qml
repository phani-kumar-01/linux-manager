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
                text: "Systemd Services"
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeHeading
                font.bold: true
                font.family: Theme.fontFamily
                anchors.verticalCenter: parent.verticalCenter
            }

            Item { Layout.fillWidth: true }

            ActionButton {
                text: "Refresh Services"
                variant: "secondary"
                icon: "🔄"
            }
        }

        // Services Table
        ListView {
            width: parent.width
            height: parent.height - 80
            clip: true
            spacing: Theme.spacingSmall

            model: [
                { unit: "docker.service", active: "active", sub: "running", desc: "Docker Application Container Engine", failed: false, prot: false },
                { unit: "ssh.service", active: "active", sub: "running", desc: "OpenBSD Secure Shell server", failed: false, prot: false },
                { unit: "dbus.service", active: "active", sub: "running", desc: "D-Bus System Message Bus", failed: false, prot: true },
                { unit: "cups.service", active: "inactive", sub: "dead", desc: "CUPS Scheduler", failed: false, prot: false },
                { unit: "example-broken.service", active: "failed", sub: "failed", desc: "Custom Background Worker", failed: true, prot: false }
            ]

            delegate: Rectangle {
                width: parent.width
                height: 60
                radius: Theme.radiusMedium
                color: Theme.bgCard
                border.color: modelData.failed ? Theme.danger : Theme.borderSubtle
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.spacingMedium
                    spacing: Theme.spacingMedium

                    Rectangle {
                        width: 10
                        height: 10
                        radius: 5
                        color: modelData.failed ? Theme.danger : (modelData.active === "active" ? Theme.success : Theme.textMuted)
                    }

                    Column {
                        Layout.preferredWidth: 240
                        Text { text: modelData.unit; color: Theme.textPrimary; font.bold: true; font.pixelSize: Theme.fontSizeNormal }
                        Text { text: modelData.desc; color: Theme.textSecondary; font.pixelSize: Theme.fontSizeSmall; elide: Text.ElideRight; width: 240 }
                    }

                    Text {
                        text: modelData.active + " (" + modelData.sub + ")"
                        color: modelData.failed ? Theme.danger : Theme.textSecondary
                        font.pixelSize: Theme.fontSizeSmall
                        Layout.preferredWidth: 140
                    }

                    Item { Layout.fillWidth: true }

                    ActionButton {
                        text: "Logs"
                        variant: "secondary"
                    }

                    ActionButton {
                        text: "Restart"
                        variant: "primary"
                    }
                }
            }
        }
    }
}
