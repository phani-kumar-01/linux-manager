import QtQuick
import ".."

Item {
    id: root

    property real value: 0.0 // 0.0 to 100.0
    property color barColor: Theme.primary
    property color trackColor: Theme.borderSubtle
    property int barHeight: 6
    property int radius: 3

    implicitWidth: 200
    implicitHeight: barHeight

    Rectangle {
        anchors.fill: parent
        radius: root.radius
        color: root.trackColor

        Rectangle {
            id: fillRect
            height: parent.height
            radius: root.radius
            color: root.barColor
            width: parent.width * Math.min(Math.max(root.value / 100.0, 0.0), 1.0)

            Behavior on width {
                NumberAnimation { duration: 250; easing.type: Easing.OutQuad }
            }
        }
    }
}
