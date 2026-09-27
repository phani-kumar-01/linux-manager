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
                text: "System Cleanup"
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeHeading
                font.bold: true
                font.family: Theme.fontFamily
                anchors.verticalCenter: parent.verticalCenter
            }

            Item { Layout.fillWidth: true }

            ActionButton {
                text: "Dry Run Preview"
                variant: "secondary"
                icon: "👁️"
                onClicked: confirmModal.open()
            }

            ActionButton {
                text: "Clean Selected (1.77 GB)"
                variant: "primary"
                icon: "✨"
                onClicked: confirmModal.open()
            }
        }

        // Cleanable Items List
        ListView {
            width: parent.width
            height: parent.height - 100
            clip: true
            spacing: Theme.spacingSmall

            model: [
                { id: "apt", title: "APT Package Cache", desc: "Archived .deb packages in /var/cache/apt/archives", path: "/var/cache/apt/archives", size: "420 MB", risk: "Safe", root: true },
                { id: "logs", title: "System Journal Logs", desc: "Systemd archived logs exceeding 100MB", path: "/var/log/journal", size: "260 MB", risk: "Safe", root: true },
                { id: "thumbs", title: "Thumbnail Previews", desc: "Cached file and image thumbnails in ~/.cache/thumbnails", path: "~/.cache/thumbnails", size: "95 MB", risk: "Safe", root: false },
                { id: "crash", title: "Crash Reports", desc: "Apport core dumps and crash traces in /var/crash", path: "/var/crash", size: "40 MB", risk: "Safe", root: true },
                { id: "flatpak", title: "Unused Flatpak Runtimes", desc: "Old runtimes not referenced by any installed app", path: "/var/lib/flatpak/runtime", size: "780 MB", risk: "Safe", root: false },
                { id: "browser", title: "Web Browser Caches", desc: "Temporary HTTP disk cache for Firefox & Chrome", path: "~/.cache/mozilla, ~/.cache/google-chrome", size: "180 MB", risk: "Safe", root: false }
            ]

            delegate: CleanupItemRow {
                itemId: modelData.id
                title: modelData.title
                description: modelData.desc
                path: modelData.path
                sizeFormatted: modelData.size
                riskLevel: modelData.risk
                requiresElevation: modelData.root
                isSelected: true
            }
        }
    }

    ConfirmModal {
        id: confirmModal
        title: "Clean System Caches?"
        description: "The following operations will be executed safely. Essential configuration and personal user files are never touched."
        sizeReclaimText: "1.77 GB"
        detailsList: [
            "Purge /var/cache/apt/archives/* (420 MB)",
            "Vacuum systemd journal logs to 100 MB (260 MB)",
            "Delete ~/.cache/thumbnails/* (95 MB)",
            "Uninstall unused Flatpak runtimes (780 MB)"
        ]
        onAccepted: {
            // execute cleanup
        }
    }
}
