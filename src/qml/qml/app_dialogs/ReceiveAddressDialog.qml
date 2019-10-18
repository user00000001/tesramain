﻿import QtQuick 2.10
import QtQuick.Window 2.3
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.3
import QtQuick.Controls.Material.impl 2.3
import QtQuick.Controls 1.4 as Controls_1_4
import QtQuick.Controls.Styles 1.4 as Controls_1_4_style
import QtQuick.Controls 1.1 as Controls_1_1

import QtGraphicalEffects 1.0
import QtQuick.Dialogs 1.3
import "../app_pages"
import "../app_items"
import "../base_items"


CommonDialog
{
    id:root
    title: qsTr("Receiving Address")

    confrim_btn_visible:false
    cancel_btn_visible:false
    property bool forPicking: false
    signal pickAddress(string address)

    width:800
    height:500

    Item{
        parent:root.background_rec
        anchors.fill: parent
        anchors.topMargin: 35
        anchors.leftMargin: 17
        anchors.rightMargin: 17
        anchors.bottomMargin: 17


        Connections {
            target: walletModel.receivingAddressProxyModel
            onMessage:
            {
                root_window.warningDialog.title = title
                root_window.warningDialog.content_text = message
                root_window.warningDialog.dim_back = false
                root_window.warningDialog.show()
            }

        }

        AddressDialog
        {
            id:addressDialog

            forSending:false
            model:address_table.model
        }


        Label
        {
            id:sign_label
            anchors.top:parent.top
            width:parent.width
            text: qsTr("These are your TESRA addresses for receiving payments. It is recommended to use a new receiving address for each transaction.")
            font.weight: Font.Light
            font.pixelSize: 11
            wrapMode: Text.WrapAnywhere
            lineHeight: 0.8
            font.letterSpacing:0.3
        }


        CommonTableView
        {
            id:address_table
            anchors.top:sign_label.bottom
            anchors.left: parent.left
            anchors.topMargin: 12
            anchors.right: parent.right
            height:300
            roles:  ["label","address"]
            titles: ["Label","Address"]
            widths: [400,300,width-740]

            selectionMode: Controls_1_1.SelectionMode.SingleSelection

            model:walletModel.receivingAddressProxyModel

            onDoubleClicked:
            {
                if(forPicking)
                {
                    if(address_table.currentRow != -1)
                        root.pickAddress(address_table.model.getData("address",address_table.currentRow))
                    root.close()
                    return
                }

                addressDialog.tagTextField.text = address_table.model.getData("label",address_table.currentRow)
                addressDialog.addressTextField.text = address_table.model.getData("address",address_table.currentRow)
                addressDialog.currentRow = address_table.currentRow
                addressDialog.editing = true
                addressDialog.show()
            }

        }





        CommonButton
        {
            id:new_btn
            color: "#0068B7"
            anchors.left: parent.left
            anchors.bottom: parent.bottom

            width: 80
            height: 32
            radius: 3
            text:qsTr("New Address")
            textSize:11
            letterSpacing:0

            onClicked:
            {
                if(!requestUnlock())
                {
                    addressDialog.editing = false
                    addressDialog.show()
                }
            }

        }

        CommonButton
        {
            id:copy_btn
            color:walletModel.isTestNet()?"#202020":"#1E5569"
            anchors.left: new_btn.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 8
            width: 60
            height: 32
            radius: 3
            text:qsTr("Copy")
            textSize:11
            letterSpacing:0

            ToolTip
            {
                id:tooltip_paste
                text:qsTr("Pasted")
            }

            onClicked:
            {
                walletModel.setClipBoard(address_table.model.getData("address",address_table.currentRow))
                tooltip_paste.visible = true
            }
        }

        CommonButton
        {
            id:export_btn
            color: "#718BBC"
            anchors.right: close_btn.left
            anchors.bottom: parent.bottom
            anchors.rightMargin: 8
            width: 60
            height: 32
            radius: 3
            text:qsTr("Export")
            textSize:11
            letterSpacing:0
            visible: !forPicking

            onClicked:
            {
                walletModel.receivingAddressProxyModel.exportClicked()
            }
        }

        CommonButton
        {
            id:close_btn
            color: "#00A0E9"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: 60
            height: 32
            radius: 3
            text:qsTr("Close")
            textSize:11
            visible: !forPicking

            letterSpacing:0
            onClicked: {
                root.close()
            }
        }


        CommonButton
        {
            id:pick_btn
            color: "#718BBC"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: 60
            height: 32
            radius: 3
            text:qsTr("Select")
            textSize:11
            letterSpacing:0
            visible: forPicking
            onClicked: {
                if(address_table.currentRow != -1)
                    root.pickAddress(address_table.model.getData("address",address_table.currentRow))
                root.close()
            }
        }



    }


}
