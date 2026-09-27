import QtQuick
import QtQuick.Controls
import ".."

Rectangle {
    id: root

    property string title: "Metric"
    property string value: "0"
    property string subtitle: ""
    property color accentColor: Theme.primary
    property real progress: -1.0 // 0.0 to 1.0; -1 hides bar
    property string iconText: "●"

    width: 220
    height: 120
    radius: Theme.radiusMedium
    color: mouseArea.containsMouse ? Theme.bgCardHover : Theme.bgCard
    border.color: mouseArea.containsMouse ? Qt.darker(accentColor, 1.2) : Theme.borderSubtle
    border.width: 1

    Behavior on color { ColorAnimation { duration: 150 } }
    Behavior on border.color { ColorAnimation { duration: 150 } }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }

    Column {
        anchors.fill: parent
        anchors.margins: Theme.spacingMedium
        spacing: Theme.spacingSmall

        Row {
            width: parent.width
            spacing: Theme.spacingSmall

            Text {
                text: root.iconText
                color: root.accentColor
                font.pixelSize: Theme.fontSizeMedium
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: root.title
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeNormal
                font.family: Theme.fontFamily
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Text {
            text: root.value
            color: Theme.textPrimary
            font.pixelSize: Theme.fontSizeHeading
            font.bold: true
            font.family: Theme.fontFamily
        }

        Rectangle {
            id: progressBar
            visible: root.progress >= 0
            width: parent.width
            height: 4
            radius: 2
            color: Theme.borderSubtle

            Rectangle {
                width: parent.width * Math.min(Math.max(root.progress, 0.0), 1.0)
                height: parent.height
                radius: parent.radius
                color: root.accentColor

                Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
            }
        }

        Text {
            visible: root.subtitle !== "" && root.progress < 0
            text: root.subtitle
            color: Theme.textMuted
            font.pixelSize: Theme.fontSizeSmall
            font.family: Theme.fontFamily
        }
    }
}
