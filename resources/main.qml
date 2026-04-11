import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import GraphConstructor 1.0

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    title: "Graph Constructor (EnTT + Qt Quick)"

    menuBar: MenuBar {
        Menu {
            title: "Main"
            MenuItem { text: "Clean Workspace (Ctrl+N)"; onTriggered: canvas.clearWorkspace() }
            MenuSeparator {}
            MenuItem { text: "Undo (Ctrl+Z)"; onTriggered: canvas.undo() }
            MenuItem { text: "Redo (Ctrl+Shift+Z)"; onTriggered: canvas.redo() }
            MenuSeparator {}
            MenuItem { text: "Exit"; onTriggered: Qt.quit() }
        }
        Menu {
            title: "Export"
            MenuItem { 
                text: "SVG"
                onTriggered: {
                    svgSaveDialog.open();
                }
            }
        }
        Menu {
            title: "Import (Requires Console Binding logic)"
            MenuItem { text: "SVG"; onTriggered: console.log("Import not yet wired to file loaders") }
            MenuItem { text: "JSON"; onTriggered: console.log("Import not yet wired to file loaders") }
            MenuItem { text: "CSV"; onTriggered: console.log("Import not yet wired to file loaders") }
        }
        Menu {
            title: "Examples"
            MenuItem { text: "3-to-2 Expander"; onTriggered: canvas.pasteExample(0) }
            MenuItem { text: "Neural Network"; onTriggered: canvas.pasteExample(1) }
            MenuItem { text: "Binary Tree (H:4)"; onTriggered: canvas.pasteExample(2) }
            MenuItem { text: "Fully Connected (K-6)"; onTriggered: canvas.pasteExample(3) }
        }
        Menu {
            title: "View"
            MenuItem {
                text: "Toggle Code Panel"
                onTriggered: {
                    codeViewPanel.visible = !codeViewPanel.visible;
                    if (codeViewPanel.visible) updateCodeView();
                }
            }
        }
        Menu {
            title: "Page"
            Menu {
                title: "Auto-Align"
                MenuItem { text: "Radial (Orbital)"; onTriggered: canvas.autoAlign(0) }
                MenuItem { text: "Row-wise"; onTriggered: canvas.autoAlign(1) }
                MenuItem { text: "Columnar"; onTriggered: canvas.autoAlign(2) }
            }
        }
    }

    Menu {
        id: nodeContextMenu
        property int targetEntity: -1
        property string currentText: ""
        MenuItem { text: "Toggle Borderless"; onTriggered: canvas.toggleNodeBorderless(nodeContextMenu.targetEntity) }
        MenuItem {
            text: "Edit Caption..."
            onTriggered: {
                textEditDialog.targetEntity = nodeContextMenu.targetEntity;
                captionInput.text = nodeContextMenu.currentText;
                positionDropdown.currentIndex = canvas.getNodeLabelPosition(nodeContextMenu.targetEntity);
                textEditDialog.open();
            }
        }
        MenuSeparator {}
        MenuItem { text: "Copy Node"; onTriggered: canvas.copy() }
    }

    Menu {
        id: canvasContextMenu
        MenuItem { text: "Paste"; onTriggered: canvas.paste() }
        MenuSeparator {}
        MenuItem { text: "Select All"; onTriggered: canvas.selectAll() }
    }

    Dialog {
        id: textEditDialog
        title: "Edit Caption"
        standardButtons: Dialog.Ok | Dialog.Cancel
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        property int targetEntity: -1
        Column {
            spacing: 10
            TextField {
                id: captionInput
                placeholderText: "Enter node caption..."
                width: 200
            }
            ComboBox {
                id: positionDropdown
                model: ["Center", "Top", "Bottom", "Left", "Right"]
                width: 200
            }
        }
        onAccepted: {
            canvas.setNodeText(targetEntity, captionInput.text);
            canvas.setNodeLabelPosition(targetEntity, positionDropdown.currentIndex);
        }
        onOpened: captionInput.forceActiveFocus()
    }

    FileDialog {
        id: svgSaveDialog
        title: "Save SVG"
        fileMode: FileDialog.SaveFile
        nameFilters: ["SVG files (*.svg)"]
        onAccepted: {
            canvas.saveStringToFile(canvas.exportToSVG(), svgSaveDialog.selectedFile);
        }
    }

    FileDialog {
        id: codeSaveDialog
        title: "Save Output"
        fileMode: FileDialog.SaveFile
        onAccepted: {
            canvas.saveStringToFile(codeTextArea.text, codeSaveDialog.selectedFile);
        }
    }

    Shortcut {
        sequence: "Ctrl+C"
        onActivated: canvas.copy()
    }
    Shortcut {
        sequence: "Ctrl+V"
        onActivated: canvas.paste()
    }
    Shortcut {
        sequence: "Ctrl+A"
        onActivated: canvas.selectAll()
    }
    Shortcut {
        sequence: "Ctrl+Z"
        onActivated: canvas.undo()
    }
    Shortcut {
        sequence: "Ctrl+Shift+Z"
        onActivated: canvas.redo()
    }
    Shortcut {
        sequence: "Ctrl+N"
        onActivated: canvas.clearWorkspace()
    }
    Shortcut {
        sequence: "Ctrl++"
        onActivated: canvas.zoomIn()
    }
    Shortcut {
        sequence: "Ctrl+=" // Common standard override mapping natively handling Plus keystrokes
        onActivated: canvas.zoomIn()
    }
    Shortcut {
        sequence: "Ctrl+-"
        onActivated: canvas.zoomOut()
    }

    SplitView {
        anchors.fill: parent

        Rectangle {
            id: palettePanel
            SplitView.preferredWidth: 80
            SplitView.minimumWidth: 80
            color: "#f0f0f0"
            z: (maBlack.drag.active || maGrey.drag.active || maWhite.drag.active) ? 100 : 0

            Column {
                anchors.centerIn: parent
                spacing: 20

                Item { width: 30; height: 30
                     Rectangle {
                         id: dragBlack
                         x: 5; y: 5
                         width: 20; height: 20; radius: 10; color: "black"; border.color: "black"; border.width: 1
                         Drag.active: maBlack.drag.active
                         Drag.hotSpot.x: 10; Drag.hotSpot.y: 10
                         property int nodeShade: 0
                         MouseArea { id: maBlack; anchors.fill: parent; drag.target: parent; onReleased: { parent.Drag.drop(); parent.x = 5; parent.y = 5; } }
                     }
                }
                
                // Grey
                Item { width: 30; height: 30
                     Rectangle {
                         id: dragGrey
                         x: 5; y: 5
                         width: 20; height: 20; radius: 10; color: "gray"; border.color: "black"; border.width: 1
                         Drag.active: maGrey.drag.active
                         Drag.hotSpot.x: 10; Drag.hotSpot.y: 10
                         property int nodeShade: 128
                         MouseArea { id: maGrey; anchors.fill: parent; drag.target: parent; onReleased: { parent.Drag.drop(); parent.x = 5; parent.y = 5; } }
                     }
                }
                
                // White
                Item { width: 30; height: 30
                     Rectangle {
                         id: dragWhite
                         x: 5; y: 5
                         width: 20; height: 20; radius: 10; color: "white"; border.color: "black"; border.width: 1
                         Drag.active: maWhite.drag.active
                         Drag.hotSpot.x: 10; Drag.hotSpot.y: 10
                         property int nodeShade: 255
                         MouseArea { id: maWhite; anchors.fill: parent; drag.target: parent; onReleased: { parent.Drag.drop(); parent.x = 5; parent.y = 5; } }
                     }
                }
            }
        }

        Item {
            SplitView.fillWidth: true
            clip: true
            
            GraphCanvas {
                id: canvas
                anchors.fill: parent

                onNodeRightClicked: function(entityId, currentText, clickX, clickY) {
                    nodeContextMenu.targetEntity = entityId;
                    nodeContextMenu.currentText = currentText;
                    nodeContextMenu.popup(); // displays at mouse coordinates intrinsically
                }

                onCanvasRightClicked: function(clickX, clickY) {
                    canvasContextMenu.popup();
                }

                DropArea {
                    anchors.fill: parent
                    onDropped: function(drop) {
                        console.log("Drop event detected!");
                        if (drop.source && drop.source.nodeShade !== undefined) {
                            console.log("Binding success! Adding node at", drop.x, drop.y);
                            canvas.addNode(drop.x, drop.y, drop.source.nodeShade, 10.0)
                            drop.accept()
                        } else {
                            console.log("Binding failed or unauthorized item dropped.");
                        }
                    }
                }

                HoverHandler {
                    id: canvasHover
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.margins: 20
                    width: zoomInput.width + 20
                    height: zoomInput.height + 10
                    radius: 6
                    color: "#80000000"
                    opacity: canvasHover.hovered ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: 150 } }

                    TextInput {
                        id: zoomInput
                        anchors.centerIn: parent
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                        text: Math.round(canvas.zoom * 100) + "%"
                        
                        onEditingFinished: {
                            var val = parseInt(text.replace("%", ""));
                            if (!isNaN(val) && val > 0) {
                                canvas.setZoomScale(val);
                            }
                            focus = false;
                            text = Math.round(canvas.zoom * 100) + "%"; // refresh bounds strictly
                        }

                        onActiveFocusChanged: {
                            if (activeFocus) text = text.replace("%", "");
                            else text = Math.round(canvas.zoom * 100) + "%";
                        }
                    }
                }
            }
        }

        Rectangle {
            id: codeViewPanel
            SplitView.preferredWidth: 300
            SplitView.minimumWidth: 200
            color: "#2b2b2b"
            visible: false

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                TabBar {
                    id: codeTypeBar
                    Layout.fillWidth: true
                    background: Rectangle { color: "#3c3f41" }
                    
                    TabButton { 
                        text: "TikZ" 
                        contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        background: Rectangle { color: codeTypeBar.currentIndex === 0 ? "#4b6eaf" : "transparent" }
                    }
                    TabButton { 
                        text: "JSON"
                        contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        background: Rectangle { color: codeTypeBar.currentIndex === 1 ? "#4b6eaf" : "transparent" }
                    }
                    TabButton { 
                        text: "CSV"
                        contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        background: Rectangle { color: codeTypeBar.currentIndex === 2 ? "#4b6eaf" : "transparent" }
                    }
                    
                    onCurrentIndexChanged: updateCodeView()
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.margins: 5
                    Button {
                        text: "Copy"
                        onClicked: {
                            canvas.copyToClipboard(codeTextArea.text);
                        }
                    }
                    Button {
                        text: "Save..."
                        onClicked: {
                            codeSaveDialog.open()
                        }
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    TextArea {
                        id: codeTextArea
                        color: "#a9b7c6"
                        font.family: "Monospace"
                        readOnly: true
                        wrapMode: TextEdit.NoWrap
                        background: Rectangle { color: "#2b2b2b" }
                    }
                }
            }
        }
    }

    Connections {
        target: canvas
        function onGraphChanged() {
            if (codeViewPanel.visible) {
                updateCodeView();
            }
        }
    }

    function updateCodeView() {
        if (!codeViewPanel.visible) return;
        if (codeTypeBar.currentIndex === 0) {
            codeTextArea.text = canvas.exportToTikZ();
        } else if (codeTypeBar.currentIndex === 1) {
            codeTextArea.text = canvas.exportToJSON();
        } else if (codeTypeBar.currentIndex === 2) {
            codeTextArea.text = canvas.exportToCSV();
        }
    }
}