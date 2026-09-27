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
                text: "Software Manager"
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeHeading
                font.bold: true
                font.family: Theme.fontFamily
                anchors.verticalCenter: parent.verticalCenter
            }

            Item { Layout.fillWidth: true; width: 1 }

            // Search Box
            Rectangle {
                width: 260
                height: 36
                radius: Theme.radiusSmall
                color: Theme.bgInput
                border.color: Theme.borderSubtle
                border.width: 1

                TextInput {
                    anchors.fill: parent
                    anchors.margins: 8
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeNormal
                    clip: true
                    Text {
                        text: "Search packages or flatpaks..."
                        color: Theme.textMuted
                        visible: parent.text === ""
                        font.pixelSize: Theme.fontSizeNormal
                    }
                }
            }
        }

        // Tabs: Installed / Available / Orphans / Flatpaks
        Row {
            spacing: Theme.spacingSmall

            ActionButton { text: "Installed Packages"; variant: "primary" }
            ActionButton { text: "Orphan Dependencies"; variant: "secondary" }
            ActionButton { text: "Flatpak Apps"; variant: "secondary" }
        }

        // Package Table List
        ListView {
            width: parent.width
            height: parent.height - 130
            clip: true
            spacing: Theme.spacingSmall

            model: [
                { name: "code", version: "1.92.0-17230", size: "412 MB", source: "APT", section: "devel", desc: "Visual Studio Code - Code editor" },
                { name: "docker-ce", version: "27.1.1-1~ubuntu", size: "98 MB", source: "APT", section: "admin", desc: "Docker Community Edition" },
                { name: "firefox", version: "129.0.1", size: "260 MB", source: "APT", section: "web", desc: "Mozilla Firefox Web Browser" },
                { name: "git", version: "2.43.0-1ubuntu7", size: "42 MB", source: "APT", section: "vcs", desc: "Fast, scalable, distributed revision control system" },
                { name: "org.gimp.GIMP", version: "2.10.38", size: "380 MB", source: "Flatpak", section: "graphics", desc: "GNU Image Manipulation Program" }
            ]

            delegate: Rectangle {
                width: parent.width
                height: 64
                radius: Theme.radiusMedium
                color: Theme.bgCard
                border.color: Theme.borderSubtle
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.spacingMedium
                    spacing: Theme.spacingMedium

                    Column {
                        Layout.preferredWidth: 200
                        Text { text: modelData.name; color: Theme.textPrimary; font.bold: true; font.pixelSize: Theme.fontSizeNormal }
                        Text { text: modelData.desc; color: Theme.textSecondary; font.pixelSize: Theme.fontSizeSmall; elide: Text.ElideRight; width: 200 }
                    }

                    Text { text: modelData.version; color: Theme.textMuted; Layout.preferredWidth: 140; font.pixelSize: Theme.fontSizeSmall }
                    Text { text: modelData.source; color: Theme.accent; Layout.preferredWidth: 80; font.pixelSize: Theme.fontSizeSmall; font.bold: true }
                    Text { text: modelData.size; color: Theme.textPrimary; Layout.preferredWidth: 90; font.pixelSize: Theme.fontSizeNormal }

                    Item { Layout.fillWidth: true }

                    ActionButton {
                        text: "Remove"
                        variant: "danger"
                        onClicked: confirmModal.open()
                    }
                }
            }
        }
    }

    ConfirmModal {
        id: confirmModal
        title: "Remove Application?"
        description: "This will remove the selected package and prompt to clean associated user config and cache leftovers."
        sizeReclaimText: "412 MB"
        detailsList: [
            "Package: code (412 MB)",
            "Config: ~/.config/Code (12 MB)",
            "Cache: ~/.cache/Code (48 MB)"
        ]
        onAccepted: {
            // execute removal
        }
    }
}
