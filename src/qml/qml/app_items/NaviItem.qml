import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.3
import QtGraphicalEffects 1.0

Rectangle{
    id:root
    implicitHeight: 60
    implicitWidth: 180
    color: walletModel.isTestNet()?"#3F6D2A":"#FAFAFA"
    property string title
    property string icon
    property bool picked
    property bool hovered

    property int index_
    signal clicked()

    LinearGradient {
        source: parent;
        visible: picked || hovered

        width:parent.width
        height:parent.height
        gradient: Gradient {
           GradientStop{ position: 0.0; color: (picked||hovered)?(walletModel.isTestNet()?"#3F6D2A":"#0068B7"):(walletModel.isTestNet()?"#3F6D2A":"#FAFAFA");}
            GradientStop{ position: 1.0; color: (picked||hovered)?(walletModel.isTestNet()?"#9B3e2020":"#0068B7"):(walletModel.isTestNet()?"#3e2020":"#FAFAFA");}


        }
        start: Qt.point(0,parent.height/2);
        end: Qt.point(parent.width, parent.height/2);


    }


    Item{
        anchors.fill: parent

        Image{
            id:navi_img
            source: icon
            fillMode: Image.PreserveAspectFit
            anchors.left:parent.left
            anchors.leftMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            width:33
            height:33

        }

        Label{
            id:navi_label
            text:title
            color: (picked||hovered)?"#FFFFFF":"#3D4966"
            anchors.left:navi_img.right
            anchors.leftMargin: 10
            font.pixelSize: 15
            anchors.verticalCenter: parent.verticalCenter
            font.weight: Font.Normal
        }
    }

    MouseArea
    {
        hoverEnabled: true
        anchors.fill: parent

        onEntered:
        {
//            if(!picked)
//            {
              hovered = true
//           }
        }

        onExited:
        {
            hovered = false
        }

        onClicked:
        {
            if(!picked)
            {
                reset_all_items()

                if(index_ != 6)
                {
                    picked = true
                    tab_change(index_)
                }


            }
            root.clicked()

        }

    }

}
