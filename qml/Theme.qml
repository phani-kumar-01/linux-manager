pragma Singleton
import QtQuick

QtObject {
    id: theme

    property bool isDarkMode: true

    // Background Colors
    readonly property color bgBase: isDarkMode ? "#121418" : "#f8f9fa"
    readonly property color bgSidebar: isDarkMode ? "#181b20" : "#ffffff"
    readonly property color bgCard: isDarkMode ? "#1e2229" : "#ffffff"
    readonly property color bgCardHover: isDarkMode ? "#262b34" : "#f1f3f5"
    readonly property color bgInput: isDarkMode ? "#15181d" : "#edf0f2"

    // Accent & Status Colors
    readonly property color primary: "#3b82f6"       // Electric Blue
    readonly property color primaryHover: "#2563eb"
    readonly property color accent: "#06b6d4"        // Cyan
    readonly property color success: "#10b981"       // Emerald Green
    readonly property color warning: "#f59e0b"       // Amber
    readonly property color danger: "#ef4444"        // Coral Red
    readonly property color info: "#6366f1"          // Indigo

    // Text Colors
    readonly property color textPrimary: isDarkMode ? "#f3f4f6" : "#111827"
    readonly property color textSecondary: isDarkMode ? "#9ca3af" : "#6b7280"
    readonly property color textMuted: isDarkMode ? "#6b7280" : "#9ca3af"

    // Borders & Dividers
    readonly property color borderSubtle: isDarkMode ? "#2d333b" : "#e5e7eb"
    readonly property color borderFocus: "#3b82f6"

    // Typography
    readonly property string fontFamily: "Inter, Ubuntu, -apple-system, sans-serif"
    readonly property int fontSizeSmall: 11
    readonly property int fontSizeNormal: 13
    readonly property int fontSizeMedium: 15
    readonly property int fontSizeLarge: 18
    readonly property int fontSizeHeading: 22

    // Spacing & Metrics
    readonly property int radiusSmall: 6
    readonly property int radiusMedium: 10
    readonly property int radiusLarge: 14
    readonly property int spacingSmall: 8
    readonly property int spacingMedium: 16
    readonly property int spacingLarge: 24
}
