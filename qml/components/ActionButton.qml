import QtQuick
import QtQuick.Controls
import ".."

Rectangle {
    id: root

    signal clicked()

    property string text: "Button"
    property string variant: "primary" // "primary", "secondary", "danger", "flat"
    property bool enabled: true
    property string icon: ""

    implicitWidth: Math.max(100, labelText.contentWidth + 32)
    implicitHeight: 36
    radius: Theme.radiusSmall
    opacity: enabled ? 1.0 : 0.4

    color: {
        if (!enabled) return Theme.bgInput
        if (mouseArea.pressed) {
            if (variant === "danger") return Qt.darker(Theme.danger, 1.3)
            if (variant === "secondary") return Theme.bgCardHover
            if (variant === "flat") return Theme.bgCardHover
            return Qt.darker(Theme.primary, 1.3)
        }
        if (mouseArea.containsMouse) {
            if (variant === "danger") return Qt.lighter(Theme.danger, 1.1)
            if (variant === "secondary") return Theme.bgCardHover
            if (variant === "flat") return Theme.bgCardHover
            return Theme.primaryHover
        }
        if (variant === "danger") return Theme.danger
        if (variant === "secondary") return Theme.bgCard
        if (variant === "flat") return "transparent"
        return Theme.primary
    }

    border.color: {
        if (variant === "secondary") return Theme.borderSubtle
        return "transparent"
    }
    border.width: variant === "secondary" ? 1 : 0

    Behavior on color { ColorAnimation { duration: 120 } }

    Row {
        anchors.centerIn: parent
        spacing: Theme.spacingSmall

        Text {
            visible: root.icon !== ""
            text: root.icon
            color: (variant === "secondary" || variant === "flat") ? Theme.textPrimary : "#ffffff"
            font.pixelSize: Theme.fontSizeNormal
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            id: labelText
            text: root.text
            color: (variant === "secondary" || variant === "flat") ? Theme.textPrimary : "#ffffff"
            font.pixelSize: Theme.fontSizeNormal
            font.bold: variant === "primary" || variant === "danger"
            font.family: Theme.fontFamily
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: root.enabled
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: if (root.enabled) root.clicked()
    }
}
