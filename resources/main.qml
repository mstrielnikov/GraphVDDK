import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
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
            MenuItem { text: "SVG"; onTriggered: console.log("SVG:\n", canvas.exportToSVG()) }
            MenuItem { text: "JSON"; onTriggered: console.log("JSON:\n", canvas.exportToJSON()) }
            MenuItem { text: "TikZ"; onTriggered: console.log("TikZ:\n", canvas.exportToTikZ()) }
            MenuItem { text: "CSV"; onTriggered: console.log("CSV:\n", canvas.exportToCSV()) }
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
    }
}