import QtQuick
import QtQuick.Controls
import ".."

Rectangle {
    id: root

    property string title: "Confirm Action"
    property string description: "Review the items that will be deleted before proceeding."
    property string primaryActionText: "Proceed"
    property string primaryActionVariant: "danger"
    property var detailsList: []
    property string sizeReclaimText: ""

    signal accepted()
    signal rejected()

    anchors.fill: parent
    color: "#80000000" // Modal overlay backdrop
    visible: false
    z: 999

    MouseArea {
        anchors.fill: parent
        // Eat outside clicks
    }

    Rectangle {
        id: dialogCard
        width: Math.min(520, parent.width - 40)
        implicitHeight: dialogCol.height + Theme.spacingLarge * 2
        anchors.centerIn: parent
        radius: Theme.radiusLarge
        color: Theme.bgCard
        border.color: Theme.borderSubtle
        border.width: 1

        Column {
            id: dialogCol
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: Theme.spacingLarge
            spacing: Theme.spacingMedium

            Text {
                text: root.title
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeHeading
                font.bold: true
                font.family: Theme.fontFamily
            }

            Text {
                text: root.description
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeNormal
                font.family: Theme.fontFamily
                wrapMode: Text.WordWrap
                width: parent.width
            }

            // Impact Details Box
            Rectangle {
                visible: root.detailsList.length > 0 || root.sizeReclaimText !== ""
                width: parent.width
                implicitHeight: detailsCol.height + Theme.spacingMedium * 2
                radius: Theme.radiusMedium
                color: Theme.bgInput
                border.color: Theme.borderSubtle
                border.width: 1

                Column {
                    id: detailsCol
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: Theme.spacingMedium
                    spacing: Theme.spacingSmall

                    Text {
                        visible: root.sizeReclaimText !== ""
                        text: "Recoverable Space: " + root.sizeReclaimText
                        color: Theme.accent
                        font.pixelSize: Theme.fontSizeNormal
                        font.bold: true
                        font.family: Theme.fontFamily
                    }

                    Repeater {
                        model: root.detailsList
                        delegate: Row {
                            spacing: Theme.spacingSmall
                            width: parent.width

                            Text {
                                text: "•"
                                color: Theme.warning
                                font.pixelSize: Theme.fontSizeNormal
                            }

                            Text {
                                text: modelData
                                color: Theme.textPrimary
                                font.pixelSize: Theme.fontSizeSmall
                                font.family: "Monospace"
                                elide: Text.ElideMiddle
                                width: parent.width - 20
                            }
                        }
                    }
                }
            }

            // Action Buttons
            Row {
                anchors.right: parent.right
                spacing: Theme.spacingMedium

                ActionButton {
                    text: "Cancel"
                    variant: "secondary"
                    onClicked: {
                        root.visible = false
                        root.rejected()
                    }
                }

                ActionButton {
                    text: root.primaryActionText
                    variant: root.primaryActionVariant
                    onClicked: {
                        root.visible = false
                        root.accepted()
                    }
                }
            }
        }
    }

    function open() {
        visible = true
    }

    function close() {
        visible = false
    }
}
