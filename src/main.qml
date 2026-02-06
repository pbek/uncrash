import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root
    title: "Uncrash - System Protection"
    width: settingsManager.windowWidth
    height: settingsManager.windowHeight
    minimumWidth: 600
    minimumHeight: 700

    // Signal to request application quit
    signal quitRequested

    // Menu bar
    menuBar: Controls.MenuBar {
        Controls.Menu {
            title: "&File"
            Controls.MenuItem {
                text: "&Quit"
                icon.name: "application-exit"
                action: Controls.Action {
                    text: "&Quit"
                    shortcut: StandardKey.Quit
                    onTriggered: root.quitRequested()
                }
            }
        }
    }

    // Handle close event - hide to tray instead of quitting
    onClosing: function (close) {
        close.accepted = false;
        root.hide();
    }

    // Save window size when it changes
    onWidthChanged: {
        if (width >= minimumWidth) {
            settingsManager.windowWidth = width;
        }
    }

    onHeightChanged: {
        if (height >= minimumHeight) {
            settingsManager.windowHeight = height;
        }
    }

    pageStack.initialPage: Kirigami.Page {
        title: "System Monitor & Protection"

        actions: [
            Kirigami.Action {
                text: daemonClient?.autoProtection ? "Disable Auto Protection" : "Enable Auto Protection"
                icon.name: daemonClient?.autoProtection ? "dialog-ok" : "dialog-cancel"
                enabled: daemonClient?.connected ?? false
                onTriggered: {
                    if (daemonClient) {
                        daemonClient.autoProtection = !daemonClient.autoProtection;
                    }
                }
            }
        ]

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.largeSpacing

            // Connection Status Banner
            Kirigami.InlineMessage {
                Layout.fillWidth: true
                visible: daemonClient ? !daemonClient.connected : false
                type: Kirigami.MessageType.Warning
                text: "Not connected to Uncrash daemon. Please ensure uncrashd service is running."
            }

            // Top Row: Status and Temperature Cards side by side
            RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.largeSpacing

                // Status Card
                Kirigami.Card {
                    Layout.fillWidth: true
                    Layout.preferredWidth: parent.width / 2
                    Layout.preferredHeight: Math.max(statusCardContent.implicitHeight, tempCardContent.implicitHeight) + 80

                    header: Kirigami.Heading {
                        text: "Current Status"
                        level: 2
                    }

                    contentItem: Item {
                        implicitHeight: statusCardContent.implicitHeight
                        implicitWidth: statusCardContent.implicitWidth

                        ColumnLayout {
                            id: statusCardContent
                            width: parent.width
                            spacing: Kirigami.Units.smallSpacing

                            RowLayout {
                                Layout.fillWidth: true
                                Kirigami.Icon {
                                    source: daemonClient?.cpuLimitApplied ? "data-warning" : "dialog-ok"
                                    color: daemonClient?.cpuLimitApplied ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                                    implicitWidth: Kirigami.Units.iconSizes.medium
                                    implicitHeight: Kirigami.Units.iconSizes.medium
                                }
                                Controls.Label {
                                    text: daemonClient?.cpuLimitApplied ? "⚠ CPU frequency limit is ACTIVE" : "✓ CPU running at full speed"
                                    font.bold: true
                                    color: daemonClient?.cpuLimitApplied ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                                    Layout.fillWidth: true
                                }
                            }

                            Kirigami.Separator {
                                Layout.fillWidth: true
                            }

                            GridLayout {
                                columns: 2
                                Layout.fillWidth: true

                                Controls.Label {
                                    text: "CPU Limit Status:"
                                    font.bold: true
                                }
                                Controls.Label {
                                    text: daemonClient?.cpuLimitApplied ? "APPLIED" : "Not Applied"
                                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                                    font.bold: true
                                    color: daemonClient?.cpuLimitApplied ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                                }

                                Controls.Label {
                                    text: "GPU Power:"
                                    font.bold: true
                                }
                                Controls.Label {
                                    text: (daemonClient?.gpuPower ?? 0.0).toFixed(2) + " W"
                                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                                }

                                Controls.Label {
                                    text: "GPU Threshold:"
                                    font.bold: true
                                }
                                Controls.Label {
                                    text: (daemonClient?.gpuPowerThreshold ?? 0.0).toFixed(2) + " W" + (daemonClient?.thresholdExceeded ? " (EXCEEDED)" : "")
                                    color: daemonClient?.thresholdExceeded ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.disabledTextColor
                                }

                                Controls.Label {
                                    text: "CPU Max Frequency:"
                                    font.bold: true
                                }
                                Controls.Label {
                                    text: (daemonClient?.currentMaxFrequency ?? 0.0).toFixed(2) + " GHz"
                                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                                }

                                Controls.Label {
                                    text: "CPU Current Frequency:"
                                    font.bold: true
                                }
                                Controls.Label {
                                    text: (daemonClient?.currentFrequency ?? 0.0).toFixed(2) + " GHz"
                                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                                    color: Kirigami.Theme.highlightColor
                                }

                                Controls.Label {
                                    text: "Auto Protection:"
                                    font.bold: true
                                }
                                Controls.Label {
                                    text: daemonClient?.autoProtection ? "Enabled" : "Disabled"
                                    color: daemonClient?.autoProtection ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.neutralTextColor
                                }
                            }
                        }
                    }
                }

                // Temperature Monitoring Card
                Kirigami.Card {
                    Layout.fillWidth: true
                    Layout.preferredWidth: parent.width / 2
                    Layout.preferredHeight: Math.max(statusCardContent.implicitHeight, tempCardContent.implicitHeight) + 80

                    header: Kirigami.Heading {
                        text: "Temperature & Fan Speeds"
                        level: 2
                    }

                    contentItem: Item {
                        implicitHeight: tempCardContent.implicitHeight
                        implicitWidth: tempCardContent.implicitWidth

                        GridLayout {
                            id: tempCardContent
                            width: parent.width
                            columns: 2
                            columnSpacing: Kirigami.Units.largeSpacing
                            rowSpacing: Kirigami.Units.smallSpacing

                            // GPU Section
                            Controls.Label {
                                text: "GPU"
                                font.bold: true
                                Layout.columnSpan: 2
                            }

                            Controls.Label {
                                text: "Vendor:"
                            }
                            Controls.Label {
                                text: daemonClient?.gpuVendor || "Unknown"
                                color: Kirigami.Theme.textColor
                            }

                            Controls.Label {
                                text: "Model:"
                            }
                            Controls.Label {
                                text: daemonClient?.gpuName || "Unknown"
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }

                            Controls.Label {
                                text: "Temperature:"
                            }
                            RowLayout {
                                spacing: Kirigami.Units.smallSpacing
                                Controls.Label {
                                    text: (daemonClient?.gpuTemperature ?? 0).toFixed(1) + " °C"
                                    color: daemonClient?.gpuTemperature > 80 ? Kirigami.Theme.negativeTextColor : daemonClient?.gpuTemperature > 70 ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.positiveTextColor
                                    font.bold: true
                                }
                                Kirigami.Icon {
                                    source: "temperature-warm"
                                    width: Kirigami.Units.iconSizes.small
                                    height: width
                                    opacity: (daemonClient?.gpuTemperature ?? 0) > 0 ? 1.0 : 0.0
                                }
                            }

                            Controls.Label {
                                text: "Fan Speed:"
                            }
                            RowLayout {
                                spacing: Kirigami.Units.smallSpacing
                                Controls.Label {
                                    text: {
                                        let speed = daemonClient?.gpuFanSpeed ?? 0;
                                        let vendor = daemonClient?.gpuVendor ?? "";

                                        if (speed <= 0) {
                                            return "Idle";
                                        }

                                        // AMD GPUs report RPM, NVIDIA reports percentage
                                        if (vendor === "AMD") {
                                            return speed + " RPM";
                                        } else if (vendor === "NVIDIA") {
                                            return speed + " %";
                                        } else {
                                            // Fallback: if >= 1000, assume RPM, otherwise percentage
                                            return speed >= 1000 ? speed + " RPM" : speed + " %";
                                        }
                                    }
                                }
                                Kirigami.Icon {
                                    source: "speedometer"
                                    width: Kirigami.Units.iconSizes.small
                                    height: width
                                    opacity: (daemonClient?.gpuFanSpeed ?? 0) > 0 ? 1.0 : 0.0
                                }
                            }

                            // Separator
                            Rectangle {
                                Layout.columnSpan: 2
                                Layout.fillWidth: true
                                Layout.topMargin: Kirigami.Units.smallSpacing
                                Layout.bottomMargin: Kirigami.Units.smallSpacing
                                height: 1
                                color: Kirigami.Theme.separatorColor
                            }

                            // CPU Section
                            Controls.Label {
                                text: "CPU"
                                font.bold: true
                                Layout.columnSpan: 2
                            }

                            Controls.Label {
                                text: "Temperature:"
                            }
                            RowLayout {
                                spacing: Kirigami.Units.smallSpacing
                                Controls.Label {
                                    text: (daemonClient?.cpuTemperature ?? 0).toFixed(1) + " °C"
                                    color: daemonClient?.cpuTemperature > 85 ? Kirigami.Theme.negativeTextColor : daemonClient?.cpuTemperature > 75 ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.positiveTextColor
                                    font.bold: true
                                }
                                Kirigami.Icon {
                                    source: "temperature-warm"
                                    width: Kirigami.Units.iconSizes.small
                                    height: width
                                    opacity: (daemonClient?.cpuTemperature ?? 0) > 0 ? 1.0 : 0.0
                                }
                            }

                            Controls.Label {
                                text: "Fan Speed:"
                            }
                            RowLayout {
                                spacing: Kirigami.Units.smallSpacing
                                Controls.Label {
                                    text: {
                                        let speed = daemonClient?.cpuFanSpeed ?? 0;
                                        if (speed > 0) {
                                            return speed + " RPM";
                                        } else {
                                            return "Unknown";
                                        }
                                    }
                                }
                                Kirigami.Icon {
                                    source: "speedometer"
                                    width: Kirigami.Units.iconSizes.small
                                    height: width
                                    opacity: (daemonClient?.cpuFanSpeed ?? 0) > 0 ? 1.0 : 0.0
                                }
                            }

                            // Separator
                            Rectangle {
                                Layout.columnSpan: 2
                                Layout.fillWidth: true
                                Layout.topMargin: Kirigami.Units.smallSpacing
                                Layout.bottomMargin: Kirigami.Units.smallSpacing
                                height: 1
                                color: Kirigami.Theme.separatorColor
                            }

                            // Motherboard Section
                            Controls.Label {
                                text: "Motherboard"
                                font.bold: true
                                Layout.columnSpan: 2
                            }

                            Controls.Label {
                                text: "Temperature:"
                            }
                            RowLayout {
                                spacing: Kirigami.Units.smallSpacing
                                Controls.Label {
                                    text: (daemonClient?.motherboardTemperature ?? 0).toFixed(1) + " °C"
                                    color: daemonClient?.motherboardTemperature > 60 ? Kirigami.Theme.negativeTextColor : daemonClient?.motherboardTemperature > 50 ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.positiveTextColor
                                    font.bold: true
                                }
                                Kirigami.Icon {
                                    source: "temperature-warm"
                                    width: Kirigami.Units.iconSizes.small
                                    height: width
                                    opacity: (daemonClient?.motherboardTemperature ?? 0) > 0 ? 1.0 : 0.0
                                }
                            }
                        }
                    }
                }
            }

            // GPU Power Threshold Card
            Kirigami.Card {
                Layout.fillWidth: true

                header: Kirigami.Heading {
                    text: "GPU Power Threshold"
                    level: 3
                }

                contentItem: Item {
                    implicitHeight: gpuCardContent.implicitHeight
                    implicitWidth: gpuCardContent.implicitWidth

                    ColumnLayout {
                        id: gpuCardContent
                        width: parent.width
                        spacing: Kirigami.Units.smallSpacing

                        Controls.Label {
                            text: "Set the maximum GPU power before CPU throttling is applied:"
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Kirigami.Units.largeSpacing

                            Controls.Slider {
                                id: gpuThresholdSlider
                                Layout.fillWidth: true
                                from: 50
                                to: 300
                                stepSize: 5
                                value: daemonClient?.gpuPowerThreshold ?? 100.0
                                enabled: daemonClient?.connected ?? false
                                onMoved: {
                                    if (daemonClient) {
                                        daemonClient.gpuPowerThreshold = value;
                                    }
                                }

                                Controls.ToolTip {
                                    parent: gpuThresholdSlider.handle
                                    visible: gpuThresholdSlider.pressed
                                    text: gpuThresholdSlider.value.toFixed(0) + " W"
                                }
                            }

                            Controls.SpinBox {
                                from: 50
                                to: 300
                                stepSize: 5
                                value: daemonClient?.gpuPowerThreshold ?? 100.0
                                enabled: daemonClient?.connected ?? false
                                onValueModified: {
                                    if (daemonClient) {
                                        daemonClient.gpuPowerThreshold = value;
                                    }
                                }
                                editable: true
                                textFromValue: function (value) {
                                    return value + " W";
                                }
                            }
                        }
                    }
                }
            }

            // CPU Frequency Limit Card
            Kirigami.Card {
                Layout.fillWidth: true

                header: Kirigami.Heading {
                    text: "CPU Frequency Limit"
                    level: 3
                }

                contentItem: Item {
                    implicitHeight: cpuCardContent.implicitHeight
                    implicitWidth: cpuCardContent.implicitWidth

                    ColumnLayout {
                        id: cpuCardContent
                        width: parent.width
                        spacing: Kirigami.Units.smallSpacing

                        Controls.Label {
                            text: "Set the maximum CPU frequency when GPU power exceeds threshold:"
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Kirigami.Units.largeSpacing

                            Controls.Slider {
                                id: cpuFreqSlider
                                Layout.fillWidth: true
                                from: 1.0
                                to: 5.0
                                stepSize: 0.1
                                value: daemonClient?.maxFrequency ?? 3.5
                                enabled: daemonClient?.connected ?? false
                                onMoved: {
                                    if (daemonClient) {
                                        daemonClient.maxFrequency = value;
                                    }
                                }

                                Controls.ToolTip {
                                    parent: cpuFreqSlider.handle
                                    visible: cpuFreqSlider.pressed
                                    text: cpuFreqSlider.value.toFixed(1) + " GHz"
                                }
                            }

                            Controls.SpinBox {
                                from: 10
                                to: 50
                                stepSize: 1
                                value: (daemonClient?.maxFrequency ?? 3.5) * 10
                                enabled: daemonClient?.connected ?? false
                                onValueModified: {
                                    if (daemonClient) {
                                        daemonClient.maxFrequency = value / 10.0;
                                    }
                                }
                                editable: true
                                textFromValue: function (value) {
                                    return (value / 10.0).toFixed(1) + " GHz";
                                }
                                valueFromText: function (text) {
                                    return parseFloat(text) * 10;
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Kirigami.Units.largeSpacing

                            Controls.Button {
                                text: "Apply Limit Now"
                                icon.name: "system-run"
                                enabled: (daemonClient?.connected ?? false) && (daemonClient?.regulationEnabled ?? false)
                                onClicked: {
                                    if (daemonClient) {
                                        daemonClient.applyFrequencyLimit();
                                    }
                                }
                            }

                            Controls.Button {
                                text: "Remove Limit Now"
                                icon.name: "kt-restore"
                                enabled: daemonClient?.connected ?? false
                                onClicked: {
                                    if (daemonClient) {
                                        daemonClient.removeFrequencyLimit();
                                    }
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            Controls.Switch {
                                text: "Enable CPU Regulation"
                                checked: daemonClient?.regulationEnabled ?? false
                                enabled: daemonClient?.connected ?? false
                                onToggled: {
                                    if (daemonClient) {
                                        daemonClient.regulationEnabled = checked;
                                    }
                                }
                            }
                        }

                        Kirigami.Separator {
                            Layout.fillWidth: true
                        }

                        Controls.Label {
                            text: "Auto-Protection Cooldown (prevents rapid toggling):"
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Kirigami.Units.largeSpacing

                            Controls.Slider {
                                id: cooldownSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 30
                                stepSize: 1
                                value: daemonClient?.cooldownSeconds ?? 5
                                enabled: daemonClient?.connected ?? false
                                onMoved: {
                                    if (daemonClient) {
                                        daemonClient.cooldownSeconds = Math.round(value);
                                    }
                                }

                                Controls.ToolTip {
                                    parent: cooldownSlider.handle
                                    visible: cooldownSlider.pressed
                                    text: Math.round(cooldownSlider.value) + " seconds"
                                }
                            }

                            Controls.SpinBox {
                                from: 0
                                to: 30
                                stepSize: 1
                                value: daemonClient?.cooldownSeconds ?? 5
                                enabled: daemonClient?.connected ?? false
                                onValueModified: {
                                    if (daemonClient) {
                                        daemonClient.cooldownSeconds = value;
                                    }
                                }
                                editable: true
                                textFromValue: function (value) {
                                    return value + " sec";
                                }
                                valueFromText: function (text) {
                                    return parseInt(text);
                                }
                            }
                        }

                        Controls.Label {
                            text: "When auto-protection is active and GPU power drops below threshold, wait this many seconds before removing the CPU limit. This prevents rapid on/off cycling when GPU power fluctuates around the threshold. Set to 0 to disable cooldown."
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                            font.pointSize: Kirigami.Theme.smallFont.pointSize
                            color: Kirigami.Theme.disabledTextColor
                        }
                    }
                }
            }

            // Info Card
            Kirigami.Card {
                Layout.fillWidth: true
                Layout.fillHeight: true

                header: Kirigami.Heading {
                    text: "How It Works"
                    level: 3
                }

                contentItem: Item {
                    implicitHeight: infoCardContent.implicitHeight
                    implicitWidth: infoCardContent.implicitWidth

                    ColumnLayout {
                        id: infoCardContent
                        width: parent.width
                        spacing: Kirigami.Units.smallSpacing

                        Controls.Label {
                            text: "This application monitors your GPU power consumption and automatically throttles your CPU when the GPU exceeds the configured power threshold. This helps prevent system crashes due to VRM and temperature issues on the motherboard."
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        Controls.Label {
                            text: "The Uncrash daemon (uncrashd) runs as a system service in the background, providing continuous protection even when this window is closed."
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                            color: Kirigami.Theme.neutralTextColor
                            font.italic: true
                        }
                    }
                }
            }
        }
    }
}
