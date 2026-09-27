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
            text: "Settings & Safety Policies"
            color: Theme.textPrimary
            font.pixelSize: Theme.fontSizeHeading
            font.bold: true
            font.family: Theme.fontFamily
        }

        // Safety Settings
        Rectangle {
            width: parent.width
            implicitHeight: safetyCol.height + Theme.spacingLarge * 2
            radius: Theme.radiusMedium
            color: Theme.bgCard
            border.color: Theme.borderSubtle
            border.width: 1

            Column {
                id: safetyCol
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: Theme.spacingLarge
                spacing: Theme.spacingMedium

                Text {
                    text: "Safety & Privilege Controls"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeLarge
                    font.bold: true
                    font.family: Theme.fontFamily
                }

                Text {
                    text: "Linux Manager enforces unprivileged execution for the desktop GUI. Privileged actions require Polkit authorization."
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeNormal
                    wrapMode: Text.WordWrap
                    width: parent.width
                }

                RowLayout {
                    width: parent.width
                    Text { text: "Always perform Dry-Run before destructive cleanup"; color: Theme.textPrimary; Layout.fillWidth: true }
                    ActionButton { text: "Enabled (Recommended)"; variant: "primary" }
                }

                RowLayout {
                    width: parent.width
                    Text { text: "Protected System Packages (systemd, libc6, bash, polkit)"; color: Theme.textPrimary; Layout.fillWidth: true }
                    ActionButton { text: "Enforced (Immutable)"; variant: "secondary" }
                }

                RowLayout {
                    width: parent.width
                    Text { text: "Polkit Integration Status"; color: Theme.textPrimary; Layout.fillWidth: true }
                    Text { text: "Active (pkexec available)"; color: Theme.success; font.bold: true }
                }
            }
        }

        // Audit History Card
        Rectangle {
            width: parent.width
            implicitHeight: historyCol.height + Theme.spacingLarge * 2
            radius: Theme.radiusMedium
            color: Theme.bgCard
            border.color: Theme.borderSubtle
            border.width: 1

            Column {
                id: historyCol
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: Theme.spacingLarge
                spacing: Theme.spacingMedium

                Text {
                    text: "Operation History & Audit Log"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeLarge
                    font.bold: true
                    font.family: Theme.fontFamily
                }

                ListView {
                    width: parent.width
                    height: 140
                    clip: true
                    spacing: 4

                    model: [
                        { id: "2026-08-22-001", date: "22 Aug 2026", desc: "System cleanup - 1.4 GB recovered", status: "Success" },
                        { id: "2026-08-21-002", date: "21 Aug 2026", desc: "Removed VLC Media Player - 128 MB", status: "Success" },
                        { id: "2026-08-20-001", date: "20 Aug 2026", desc: "APT system update - 32 packages", status: "Success" }
                    ]

                    delegate: RowLayout {
                        width: parent.width
                        Text { text: "[" + modelData.date + "]"; color: Theme.textMuted; font.family: "Monospace"; Layout.preferredWidth: 120 }
                        Text { text: modelData.id; color: Theme.accent; font.family: "Monospace"; Layout.preferredWidth: 140 }
                        Text { text: modelData.desc; color: Theme.textPrimary; Layout.fillWidth: true }
                        Text { text: modelData.status; color: Theme.success; font.bold: true }
                    }
                }
            }
        }
    }
}
