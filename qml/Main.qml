import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

Window {
    id: window
    width: 1080
    height: 720
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: "Linux Manager - Ubuntu & Debian Control Center"
    color: Theme.bgBase

    property int activeSection: 0

    readonly property var navigationItems: [
        { label: "Dashboard", icon: "📊", source: "Dashboard.qml" },
        { label: "Software", icon: "📦", source: "Software.qml" },
        { label: "Updates", icon: "🔄", source: "Updates.qml" },
        { label: "Cleanup", icon: "🧹", source: "Cleanup.qml" },
        { label: "Developer", icon: "⚡", source: "Developer.qml" },
        { label: "SDK Doctor", icon: "🛠️", source: "Toolchains.qml" },
        { label: "Processes & Ports", icon: "🔌", source: "Processes.qml" },
        { label: "Startup Apps", icon: "🚀", source: "Startup.qml" },
        { label: "Repositories", icon: "📁", source: "Repositories.qml" },
        { label: "Power & Battery", icon: "🔋", source: "Power.qml" },
        { label: "Storage", icon: "💿", source: "Storage.qml" },
        { label: "Services", icon: "⚙️", source: "Services.qml" },
        { label: "System Health", icon: "❤️", source: "Health.qml" },
        { label: "Settings", icon: "🔧", source: "Settings.qml" }
    ]

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Navigation Sidebar
        Rectangle {
            Layout.preferredWidth: 230
            Layout.fillHeight: true
            color: Theme.bgSidebar
            border.color: Theme.borderSubtle
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacingMedium
                spacing: Theme.spacingMedium

                // Logo Header
                Row {
                    spacing: Theme.spacingSmall
                    Layout.topMargin: Theme.spacingSmall
                    Layout.bottomMargin: Theme.spacingMedium

                    Rectangle {
                        width: 32
                        height: 32
                        radius: 8
                        color: Theme.primary

                        Text {
                            text: "LM"
                            color: "#ffffff"
                            font.bold: true
                            font.pixelSize: 14
                            anchors.centerIn: parent
                        }
                    }

                    Column {
                        spacing: 1
                        Text {
                            text: "Linux Manager"
                            color: Theme.textPrimary
                            font.pixelSize: Theme.fontSizeLarge
                            font.bold: true
                            font.family: Theme.fontFamily
                        }
                        Text {
                            text: "Ubuntu / Debian"
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSizeSmall
                            font.family: Theme.fontFamily
                        }
                    }
                }

                // Nav Items List
                ListView {
                    id: navList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 4
                    model: window.navigationItems

                    delegate: Rectangle {
                        width: parent.width
                        height: 42
                        radius: Theme.radiusSmall
                        color: {
                            if (window.activeSection === index) return Theme.primary
                            if (mouseArea.containsMouse) return Theme.bgCardHover
                            return "transparent"
                        }

                        Behavior on color { ColorAnimation { duration: 120 } }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            spacing: Theme.spacingMedium

                            Text {
                                text: modelData.icon
                                font.pixelSize: Theme.fontSizeMedium
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: modelData.label
                                color: window.activeSection === index ? "#ffffff" : Theme.textPrimary
                                font.pixelSize: Theme.fontSizeNormal
                                font.bold: window.activeSection === index
                                font.family: Theme.fontFamily
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: window.activeSection = index
                        }
                    }
                }

                // User / Footer Info
                Rectangle {
                    Layout.fillWidth: true
                    height: 48
                    radius: Theme.radiusSmall
                    color: Theme.bgInput

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8

                        Text {
                            text: "🔒 Unprivileged Mode"
                            color: Theme.success
                            font.pixelSize: Theme.fontSizeSmall
                            font.bold: true
                            Layout.fillWidth: true
                        }

                        Text {
                            text: "v1.0.0"
                            color: Theme.textMuted
                            font.pixelSize: Theme.fontSizeSmall
                        }
                    }
                }
            }
        }

        // Main Content Area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.bgBase

            Loader {
                id: contentLoader
                anchors.fill: parent
                source: window.navigationItems[window.activeSection].source
            }
        }
    }
}
