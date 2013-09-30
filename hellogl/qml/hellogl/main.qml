import QtQuick 2.0

import MM2013 1.0

Item {
     width: 400
     height: 800
     Rectangle{
         anchors.top: parent.top
         anchors.left: parent.left
         anchors.right: parent.right
         height: 100
         radius: 20
         color: "white"
         Text {
             text: qsTr("Hello QML on OpenGL")
             anchors.centerIn: parent
         }
         MouseArea{ id: moveButton
             anchors.fill: parent
             onClicked: {
                 console.log("moveButton clicked")
                 glItem.toggleMove();
             }
         }


     }

    GlItem{id: glItem
        anchors.fill: parent
    }

    Rectangle{
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 100
        radius: 20
        color: Qt.rgba(1, 1, 1, 0.7)
        Text {
            text: qsTr("Close")
            anchors.centerIn: parent
        }
         MouseArea {
            anchors.fill: parent
            onClicked: {
                Qt.quit();
            }
        }
    }
}
