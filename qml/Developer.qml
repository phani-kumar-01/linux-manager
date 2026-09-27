import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "."

Item {
    id: root

    property string activeTab: "node" // "node", "python", "java", "rust", "android", "docker"

    Column {
        anchors.fill: parent
        anchors.margins: Theme.spacingLarge
        spacing: Theme.spacingMedium

        Row {
            width: parent.width
            spacing: Theme.spacingMedium

            Text {
                text: "Developer Environment Cleanup"
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeHeading
                font.bold: true
                font.family: Theme.fontFamily
                anchors.verticalCenter: parent.verticalCenter
            }

            Item { Layout.fillWidth: true }

            ActionButton {
                text: "Scan Dev Roots (~/Projects)"
                variant: "secondary"
                icon: "🔍"
            }
        }

        // Language Tabs
        Row {
            spacing: Theme.spacingSmall

            ActionButton {
                text: "Node.js (5.9 GB)"
                variant: root.activeTab === "node" ? "primary" : "secondary"
                onClicked: root.activeTab = "node"
            }
            ActionButton {
                text: "Python (1.2 GB)"
                variant: root.activeTab === "python" ? "primary" : "secondary"
                onClicked: root.activeTab = "python"
            }
            ActionButton {
                text: "Java / Gradle (4.2 GB)"
                variant: root.activeTab === "java" ? "primary" : "secondary"
                onClicked: root.activeTab = "java"
            }
            ActionButton {
                text: "Rust / Cargo (2.4 GB)"
                variant: root.activeTab === "rust" ? "primary" : "secondary"
                onClicked: root.activeTab = "rust"
            }
            ActionButton {
                text: "Android (8.6 GB)"
                variant: root.activeTab === "android" ? "primary" : "secondary"
                onClicked: root.activeTab = "android"
            }
            ActionButton {
                text: "Docker (16.4 GB)"
                variant: root.activeTab === "docker" ? "primary" : "secondary"
                onClicked: root.activeTab = "docker"
            }
        }

        // Active Tab Details
        Rectangle {
            width: parent.width
            height: parent.height - 130
            radius: Theme.radiusMedium
            color: Theme.bgCard
            border.color: Theme.borderSubtle
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: Theme.spacingMedium
                spacing: Theme.spacingMedium

                Text {
                    text: {
                        if (root.activeTab === "node") return "Node.js Caches & Project node_modules"
                        if (root.activeTab === "python") return "Python pip Caches & __pycache__ Artifacts"
                        if (root.activeTab === "java") return "Maven ~/.m2 & Gradle Cache Repositories"
                        if (root.activeTab === "rust") return "Cargo Crates Cache & Project target/ Dirs"
                        if (root.activeTab === "android") return "Android SDK, Emulator Images & Build Caches"
                        return "Docker Containers, Images, Build Caches & Volumes"
                    }
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeLarge
                    font.bold: true
                    font.family: Theme.fontFamily
                }

                ListView {
                    width: parent.width
                    height: parent.height - 100
                    clip: true
                    spacing: Theme.spacingSmall

                    model: {
                        if (root.activeTab === "node") {
                            return [
                                { title: "npm Global Cache", path: "~/.npm", size: "1.2 GB", risk: "Safe" },
                                { title: "Project: web-app-frontend/node_modules", path: "~/Projects/web-app-frontend/node_modules", size: "2.8 GB", risk: "Moderate" },
                                { title: "Project: backend-api/node_modules", path: "~/Projects/backend-api/node_modules", size: "1.9 GB", risk: "Moderate" }
                            ]
                        } else if (root.activeTab === "python") {
                            return [
                                { title: "pip & uv Cache", path: "~/.cache/pip", size: "620 MB", risk: "Safe" },
                                { title: "Python __pycache__ Bytecode", path: "~/Projects/**/__pycache__", size: "180 MB", risk: "Safe" },
                                { title: "Inactive VirtualEnv: ml-experiment/.venv", path: "~/Projects/ml-experiment/.venv", size: "400 MB", risk: "Caution" }
                            ]
                        } else {
                            return [
                                { title: "Global Cache", path: "~/.cache", size: "2.1 GB", risk: "Safe" },
                                { title: "Build Target Artifacts", path: "~/Projects/**/build", size: "1.4 GB", risk: "Moderate" }
                            ]
                        }
                    }

                    delegate: CleanupItemRow {
                        title: modelData.title
                        path: modelData.path
                        sizeFormatted: modelData.size
                        riskLevel: modelData.risk
                        isSelected: modelData.risk === "Safe"
                    }
                }

                Row {
                    anchors.right: parent.right
                    spacing: Theme.spacingMedium

                    ActionButton {
                        text: "Clean Selected Dev Caches"
                        variant: "primary"
                        icon: "🧹"
                        onClicked: confirmModal.open()
                    }
                }
            }
        }
    }

    ConfirmModal {
        id: confirmModal
        title: "Clean Developer Artifacts?"
        description: "Projects will need to re-download dependencies (e.g. npm install / cargo build) after cleanup."
        sizeReclaimText: "4.0 GB"
        detailsList: [
            "Purge ~/.npm cache (1.2 GB)",
            "Remove ~/Projects/web-app-frontend/node_modules (2.8 GB)"
        ]
        onAccepted: {
            // execute dev cleanup
        }
    }
}
