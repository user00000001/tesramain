import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.3
import QtQuick.Controls.Material.impl 2.3
import QtQuick.Controls 1.4 as Controls_1_4
import QtQuick.Controls.Styles 1.4 as Controls_1_4_style

Controls_1_4.TabView{
    id: root
    anchors.fill: parent
    anchors.leftMargin: 17
    anchors.rightMargin: 17
    anchors.bottomMargin: 17
    anchors.topMargin:10
    property int tab_width



    style: Controls_1_4_style.TabViewStyle {
        frameOverlap: 1
        tab: Rectangle {
            color:  "#FAFAFA"
            //border.color:  styleData.selected ?"#8CBFCA":"#FAFAFA"
            //border.width: styleData.selected ?1:1
            implicitWidth: tab_width==undefined?68:tab_width
            implicitHeight: 28

            radius: 2
            Text {
                id: text
                anchors.centerIn: parent
                text: styleData.title
                color: styleData.selected ? "#0068B7":"#B8BCC3"
                font.pixelSize: 12
                font.weight:Font.Light
            }

            Rectangle
            {
                color: "#FAFAFA"
                height: 1
                width: parent.width

                anchors.bottom: parent.bottom
                anchors.bottomMargin: 1
                visible: !styleData.selected
            }




            Rectangle
            {
                color: "#0068B7"
                height: 2
                width: parent.width
                anchors.bottom: parent.bottom
                visible: styleData.selected
            }
            Rectangle
            {
                color: "#FAFAFA"
                height: 1
                width: 1
                radius:0.5
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 1
                anchors.left: parent.left
                visible: styleData.selected
            }
            Rectangle
            {
                color: "#FAFAFA"
                height: 1
                width: 1
                radius:0.5
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 1
                anchors.right: parent.right
                visible: styleData.selected
            }
        }
        frame: Rectangle { color: "transparent" }
    }
}
