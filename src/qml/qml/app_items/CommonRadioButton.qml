/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.3
import QtQuick.Controls.Material.impl 2.3

RadioButton {
     id: control
     text: qsTr("RadioButton")
     checked: true
     spacing: 2
     indicator: Rectangle {
         implicitWidth: 14
         implicitHeight: 14
         x: 0
         y: parent.height / 2 - height / 2
         radius: 7
         border.color: control.down ? (walletModel.isTestNet()?"#202020":"#3285C4"):(walletModel.isTestNet()?"#202020":"#3285C4")
         color: "transparent"

         Rectangle {
             width: 10
             height: 10
             anchors.centerIn: parent
             radius: 5
             color: control.down ? (walletModel.isTestNet()?"#202020":"#3285C4"):(walletModel.isTestNet()?"#202020":"#3285C4")
             visible: control.checked
         }

         Ripple {
             x: (parent.width - width) / 2
             y: (parent.height - height) / 2
             width: 18; height: 18

             z: -1
             anchor: control
             pressed: control.pressed
             active: control.down || control.visualFocus || control.hovered
             color: control.checked ? control.Material.highlightedRippleColor : control.Material.rippleColor
         }
     }

     contentItem: Text {
         text: control.text
         font: control.font
         opacity: enabled ? 1.0 : 0.3
         color: "#3D4966"
         verticalAlignment: Text.AlignVCenter
         leftPadding: control.indicator.width + control.spacing
     }
 }
