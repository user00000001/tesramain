import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.3
import QtQuick.Controls.Material.impl 2.3


Rectangle{
    id:root
    color: "#FFFFFF"
    height: 33
    width: 100
    property alias source: contact_img.source

    signal clicked()

    RowLayout{
        anchors.centerIn: parent

        spacing: 12
        Image{
            id:contact_img
            source: "qrc:/images/icons/contact.png"
            fillMode: Image.PreserveAspectFit
            Layout.preferredHeight:18
            Layout.preferredWidth:18
            property bool pressed
            property bool down
            property bool hovered

            Ripple {
                x: (parent.width - width) / 2
                y: (parent.height - height) / 2
                width: 22; height: 22

                z: -1
                anchor: contact_img
                pressed: contact_img.pressed
                active: contact_img.down || contact_img.visualFocus || contact_img.hovered
                color: contact_img.Material.rippleColor
            }

            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true
                onEntered:
                {
                    contact_img.hovered = true
                }

                onExited:
                {
                    contact_img.hovered = false
                }

                onReleased:
                {
                    contact_img.pressed = false
                }

                onPressed:
                {
                    contact_img.pressed = true
                }
                onClicked:
                {
                    root.clicked()
                }
            }
        }
        Image{
            id:edit_img
            visible: false
            source: "qrc:/images/icons/edit.png"
            fillMode: Image.PreserveAspectFit
            Layout.preferredHeight:15
            Layout.preferredWidth:15
            property bool pressed
            property bool down
            property bool hovered
            Ripple {
                x: (parent.width - width) / 2
                y: (parent.height - height) / 2
                width: 19; height: 19

                z: -1
                anchor: edit_img
                pressed: edit_img.pressed
                active: edit_img.down || edit_img.visualFocus || edit_img.hovered
                color: edit_img.Material.rippleColor
            }

            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true
                onEntered:
                {
                    edit_img.hovered = true
                }

                onExited:
                {
                    edit_img.hovered = false
                }

                onReleased:
                {
                    edit_img.pressed = false
                }

                onPressed:
                {
                    edit_img.pressed = true
                }
            }

        }
        Image{
            id:delete_img
            visible: false

            source: "qrc:/images/icons/delete.png"
            fillMode: Image.PreserveAspectFit
            Layout.preferredHeight:16
            Layout.preferredWidth:16
            property bool pressed
            property bool down
            property bool hovered
            Ripple {
                x: (parent.width - width) / 2
                y: (parent.height - height) / 2
                width: 20; height: 20

                z: -1
                anchor: delete_img
                pressed: delete_img.pressed
                active: delete_img.down || delete_img.visualFocus || delete_img.hovered
                color: delete_img.Material.rippleColor
            }

            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true
                onEntered:
                {
                    delete_img.hovered = true
                }

                onExited:
                {
                    delete_img.hovered = false
                }

                onReleased:
                {
                    delete_img.pressed = false
                }

                onPressed:
                {
                    delete_img.pressed = true
                }
            }

        }

    }

}
