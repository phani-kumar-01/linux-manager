import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "."

ScrollView {
    id: root
    contentWidth: availableWidth
    clip: true

    ColumnLayout {
        width: parent.width
        spacing: 20

        // Header
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 4
                Text {
                    text: "⚡ Process Explorer & Port Killer"
                    font.pixelSize: 22
                    font.bold: true
                    color: Theme.textPrimary
                }
                Text {
                    text: "Resolve port conflicts (EADDRINUSE), inspect memory footprints, and isolate developer processes"
                    font.pixelSize: 13
                    color: Theme.textSecondary
                }
            }
            Item { Layout.fillWidth: true }
        }

        // Port Killer Quick Action Card
        Rectangle {
            Layout.fillWidth: true
            height: 90
            color: Theme.bgCard
            radius: Theme.radiusMd
            border.color: Theme.borderBase
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Text {
                    text: "🔌"
                    font.pixelSize: 28
                }

                ColumnLayout {
                    spacing: 4
                    Text {
                        text: "Kill Process by Port Number"
                        font.pixelSize: 15
                        font.bold: true
                        color: Theme.textPrimary
                    }
                    Text {
                        text: "Enter port (e.g. 3000, 8080, 5432) to immediately find and terminate the blocking process"
                        font.pixelSize: 12
                        color: Theme.textSecondary
                    }
                }

                Item { Layout.fillWidth: true }

                TextField {
                    id: portInput
                    placeholderText: "e.g. 3000"
                    width: 110
                    font.pixelSize: 13
                    color: Theme.textPrimary
                    background: Rectangle {
                        color: Theme.bgSurface
                        radius: Theme.radiusSm
                        border.color: Theme.borderBase
                    }
                }

                ActionButton {
                    text: "Free Port"
                    btnColor: Theme.danger
                    onClicked: {
                        console.log("Freeing port " + portInput.text)
                    }
                }
            }
        }

        // Active Listening Ports Section
        Text {
            text: "Active Listening Ports"
            font.pixelSize: 17
            font.bold: true
            color: Theme.textPrimary
        }

        Rectangle {
            Layout.fillWidth: true
            height: 180
            color: Theme.bgCard
            radius: Theme.radiusMd
            border.color: Theme.borderBase
            border.width: 1

            ListView {
                anchors.fill: parent
                anchors.margins: 12
                clip: true
                spacing: 8
                model: [
                    { port: 3000, proto: "TCP", addr: "0.0.0.0", name: "node", pid: 14502, user: "candy" },
                    { port: 8000, proto: "TCP", addr: "127.0.0.1", name: "python3", pid: 14890, user: "candy" },
                    { port: 5432, proto: "TCP", addr: "127.0.0.1", name: "postgres", pid: 2100, user: "postgres" }
                ]

                delegate: Rectangle {
                    width: parent.width
                    height: 44
                    color: Theme.bgSurface
                    radius: Theme.radiusSm

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 16

                        Rectangle {
                            width: 60
                            height: 24
                            radius: 4
                            color: Theme.accent + "22"
                            Text {
                                anchors.centerIn: parent
                                text: ":" + modelData.port
                                font.pixelSize: 12
                                font.bold: true
                                color: Theme.accent
                            }
                        }

                        Text {
                            text: modelData.name + " (PID: " + modelData.pid + ")"
                            font.pixelSize: 13
                            font.bold: true
                            color: Theme.textPrimary
                        }

                        Text {
                            text: "• " + modelData.addr + " [" + modelData.proto + "]"
                            font.pixelSize: 12
                            color: Theme.textSecondary
                        }

                        Item { Layout.fillWidth: true }

                        ActionButton {
                            text: "Kill"
                            btnColor: Theme.danger
                            height: 28
                            onClicked: {
                                console.log("Killing PID " + modelData.pid)
                            }
                        }
                    }
                }
            }
        }
    }
}
