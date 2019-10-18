import QtQuick 2.6
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.3
import QtQuick.Controls.Material.impl 2.3

Slider {
     id: control
     value: 0.5
     width:420
     background: Rectangle {
         x: control.leftPadding
         y: control.topPadding + control.availableHeight / 2 - height / 2
         implicitWidth: 420
         implicitHeight: 3
         width: control.availableWidth
         height: implicitHeight
         radius: 1
         color: control.enabled?"#3285C4":"#F0F0F0"

         Rectangle {
             width: control.visualPosition * parent.width
             height: parent.height
             color: control.enabled?"#0068B7":"#F0F0F0"
             radius: 1
         }


     }

     handle: Rectangle {
         x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
         y: control.topPadding + control.availableHeight / 2 - height / 2
         implicitWidth: 12
         implicitHeight: 12
         radius: 6
         color: control.enabled?(control.pressed ? "#3285C4" : "#FFFFFF"):"#F0F0F0"
         //border.color: "#bdbebf"
         border.width:0

         Ripple {
             x: (parent.width - width) / 2
             y: (parent.height - height) / 2
             width: 15; height: 15

             z: -1
             anchor: control
             pressed: control.pressed
             active: control.down || control.visualFocus || control.hovered
             color: control.checked ? control.Material.highlightedRippleColor : control.Material.rippleColor
         }
     }
 }
