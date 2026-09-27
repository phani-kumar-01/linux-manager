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
                    text: "🛠️ Developer SDK Doctor & Toolchains"
                    font.pixelSize: 22
                    font.bold: true
                    color: Theme.textPrimary
                }
                Text {
                    text: "Inspect installed compilers, language SDKs, and system build essentials"
                    font.pixelSize: 13
                    color: Theme.textSecondary
                }
            }
            Item { Layout.fillWidth: true }
            ActionButton {
                text: "⚡ Clean Go Caches"
                btnColor: Theme.accent
                onClicked: {
                    console.log("Cleaning Go cache...")
                }
            }
        }

        // Environment Health Summary
        Rectangle {
            Layout.fillWidth: true
            height: 100
            color: Theme.bgCard
            radius: Theme.radiusMd
            border.color: Theme.borderBase
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 24

                Rectangle {
                    width: 48
                    height: 48
                    radius: 24
                    color: "#22c55e22"
                    Text {
                        anchors.centerIn: parent
                        text: "🩺"
                        font.pixelSize: 24
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Text {
                        text: "Toolchain Diagnostics: Healthy"
                        font.pixelSize: 16
                        font.bold: true
                        color: Theme.success
                    }
                    Text {
                        text: "Core C/C++ compilation toolchain (gcc, make, libc) and essential headers detected."
                        font.pixelSize: 13
                        color: Theme.textSecondary
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        // Section Title
        Text {
            text: "Detected Toolchains & Runtimes"
            font.pixelSize: 17
            font.bold: true
            color: Theme.textPrimary
        }

        // Grid of Toolchain Cards
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            rowSpacing: 12
            columnSpacing: 12

            Repeater {
                model: [
                    { name: "Node.js", category: "JavaScript / TypeScript", version: "v20.12.2", icon: "🟢", path: "/usr/bin/node" },
                    { name: "Python 3", category: "Python Ecosystem", version: "Python 3.12.3", icon: "🐍", path: "/usr/bin/python3" },
                    { name: "Rust (rustc)", category: "Systems Programming", version: "rustc 1.78.0", icon: "🦀", path: "~/.cargo/bin/rustc" },
                    { name: "Go (Golang)", category: "Systems / Cloud", version: "go1.22.2 linux/amd64", icon: "🐹", path: "/usr/local/go/bin/go" },
                    { name: "GCC / G++", category: "C/C++ Compilers", version: "13.2.0 (Ubuntu)", icon: "⚙️", path: "/usr/bin/gcc" },
                    { name: "Docker Engine", category: "Containers", version: "26.1.1 (Client/Server)", icon: "🐳", path: "/usr/bin/docker" }
                ]

                Rectangle {
                    Layout.fillWidth: true
                    height: 90
                    color: Theme.bgCard
                    radius: Theme.radiusMd
                    border.color: Theme.borderBase
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 14

                        Text {
                            text: modelData.icon
                            font.pixelSize: 28
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3
                            RowLayout {
                                Text {
                                    text: modelData.name
                                    font.pixelSize: 15
                                    font.bold: true
                                    color: Theme.textPrimary
                                }
                                Text {
                                    text: "• " + modelData.category
                                    font.pixelSize: 12
                                    color: Theme.textMuted
                                }
                            }
                            Text {
                                text: modelData.version
                                font.pixelSize: 13
                                color: Theme.accent
                                font.family: "Monospace"
                            }
                            Text {
                                text: modelData.path
                                font.pixelSize: 11
                                color: Theme.textSecondary
                                elide: Text.ElideMiddle
                            }
                        }
                    }
                }
            }
        }
    }
}
