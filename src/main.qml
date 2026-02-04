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
                text: daemonClient.autoProtection ? "Disable Auto Protection" : "Enable Auto Protection"
                icon.name: daemonClient.autoProtection ? "dialog-ok" : "dialog-cancel"
                enabled: daemonClient.connected
                onTriggered: daemonClient.autoProtection = !daemonClient.autoProtection
            }
        ]

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.largeSpacing

            // Connection Status Banner
            Kirigami.InlineMessage {
                Layout.fillWidth: true
                visible: !daemonClient.connected
                type: Kirigami.MessageType.Warning
                text: "Not connected to Uncrash daemon. Please ensure uncrashd service is running."
            }

            // Status Card
            Kirigami.Card {
                Layout.fillWidth: true

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
                                source: daemonClient.thresholdExceeded ? "data-warning" : "dialog-ok"
                                color: daemonClient.thresholdExceeded ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                                implicitWidth: Kirigami.Units.iconSizes.medium
                                implicitHeight: Kirigami.Units.iconSizes.medium
                            }
                            Controls.Label {
                                text: daemonClient.thresholdExceeded ? "⚠ GPU power threshold exceeded - CPU throttling active" : "✓ System operating normally"
                                font.bold: true
                                color: daemonClient.thresholdExceeded ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
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
                                text: "GPU Power:"
                                font.bold: true
                            }
                            Controls.Label {
                                text: daemonClient.gpuPower.toFixed(2) + " W"
                                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                            }

                            Controls.Label {
                                text: "GPU Threshold:"
                                font.bold: true
                            }
                            Controls.Label {
                                text: daemonClient.gpuPowerThreshold.toFixed(2) + " W"
                                color: Kirigami.Theme.disabledTextColor
                            }

                            Controls.Label {
                                text: "CPU Max Frequency:"
                                font.bold: true
                            }
                            Controls.Label {
                                text: daemonClient.currentMaxFrequency.toFixed(2) + " GHz"
                                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                            }

                            Controls.Label {
                                text: "Auto Protection:"
                                font.bold: true
                            }
                            Controls.Label {
                                text: daemonClient.autoProtection ? "Enabled" : "Disabled"
                                color: daemonClient.autoProtection ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.neutralTextColor
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
                                value: daemonClient.gpuPowerThreshold
                                enabled: daemonClient.connected
                                onMoved: daemonClient.gpuPowerThreshold = value

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
                                value: daemonClient.gpuPowerThreshold
                                enabled: daemonClient.connected
                                onValueModified: daemonClient.gpuPowerThreshold = value
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
                                value: daemonClient.maxFrequency
                                enabled: daemonClient.connected
                                onMoved: daemonClient.maxFrequency = value

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
                                value: daemonClient.maxFrequency * 10
                                enabled: daemonClient.connected
                                onValueModified: daemonClient.maxFrequency = value / 10.0
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
                                enabled: daemonClient.connected && daemonClient.regulationEnabled
                                onClicked: daemonClient.applyFrequencyLimit()
                            }

                            Controls.Button {
                                text: "Remove Limit Now"
                                icon.name: "kt-restore"
                                enabled: daemonClient.connected
                                onClicked: daemonClient.removeFrequencyLimit()
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            Controls.Switch {
                                text: "Enable CPU Regulation"
                                checked: daemonClient.regulationEnabled
                                enabled: daemonClient.connected
                                onToggled: daemonClient.regulationEnabled = checked
                            }
                        }
                    }
                }
            }

            // Info Card
            Kirigami.Card {
                Layout.fillWidth: true

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

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
