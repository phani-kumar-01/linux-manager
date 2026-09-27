import QtQuick
import QtQuick.Controls
import ".."

Rectangle {
    id: root

    property string itemId: ""
    property string title: "Cache Item"
    property string description: "Description of the cache"
    property string path: "/path/to/cache"
    property string sizeFormatted: "0 MB"
    property string riskLevel: "Safe" // "Safe", "Moderate", "Caution"
    property bool isSelected: true
    property bool requiresElevation: false
    property var subItems: []

    signal toggled(bool selected)

    width: parent ? parent.width : 600
    implicitHeight: mainCol.height + Theme.spacingMedium * 2
    radius: Theme.radiusMedium
    color: mouseArea.containsMouse ? Theme.bgCardHover : Theme.bgCard
    border.color: isSelected ? Qt.darker(Theme.primary, 1.4) : Theme.borderSubtle
    border.width: 1

    Behavior on color { ColorAnimation { duration: 120 } }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            root.isSelected = !root.isSelected
            root.toggled(root.isSelected)
        }
    }

    Column {
        id: mainCol
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Theme.spacingMedium
        spacing: Theme.spacingSmall

        Row {
            width: parent.width
            spacing: Theme.spacingMedium

            // Custom Checkbox
            Rectangle {
                width: 20
                height: 20
                radius: 4
                anchors.verticalCenter: parent.verticalCenter
                color: root.isSelected ? Theme.primary : "transparent"
                border.color: root.isSelected ? Theme.primary : Theme.borderSubtle
                border.width: 1.5

                Text {
                    visible: root.isSelected
                    text: "✓"
                    color: "#ffffff"
                    font.pixelSize: 12
                    font.bold: true
                    anchors.centerIn: parent
                }
            }

            Column {
                width: parent.width - 220
                spacing: 2

                Row {
                    spacing: Theme.spacingSmall

                    Text {
                        text: root.title
                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontSizeNormal
                        font.bold: true
                        font.family: Theme.fontFamily
                    }

                    // Risk Badge
                    Rectangle {
                        height: 18
                        width: riskText.contentWidth + 12
                        radius: 9
                        anchors.verticalCenter: parent.verticalCenter
                        color: {
                            if (root.riskLevel === "Safe") return "#064e3b"
                            if (root.riskLevel === "Moderate") return "#78350f"
                            return "#7f1d1d"
                        }

                        Text {
                            id: riskText
                            text: root.riskLevel
                            anchors.centerIn: parent
                            color: {
                                if (root.riskLevel === "Safe") return "#34d399"
                                if (root.riskLevel === "Moderate") return "#fbbf24"
                                return "#f87171"
                            }
                            font.pixelSize: 10
                            font.bold: true
                        }
                    }

                    // Sudo / Polkit Badge
                    Rectangle {
                        visible: root.requiresElevation
                        height: 18
                        width: sudoText.contentWidth + 10
                        radius: 9
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#312e81"

                        Text {
                            id: sudoText
                            text: "root"
                            anchors.centerIn: parent
                            color: "#a5b4fc"
                            font.pixelSize: 10
                            font.bold: true
                        }
                    }
                }

                Text {
                    text: root.description
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    font.family: Theme.fontFamily
                    elide: Text.ElideRight
                    width: parent.width
                }

                Text {
                    text: root.path
                    color: Theme.textMuted
                    font.pixelSize: 10
                    font.family: "Monospace"
                    elide: Text.ElideMiddle
                    width: parent.width
                }
            }

            Text {
                text: root.sizeFormatted
                color: Theme.accent
                font.pixelSize: Theme.fontSizeMedium
                font.bold: true
                font.family: Theme.fontFamily
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignRight
                width: 120
            }
        }
    }
}
