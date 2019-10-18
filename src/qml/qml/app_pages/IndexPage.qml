import QtQuick 2.10
import QtQuick.Window 2.3
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.3
import QtQuick.Controls.Material.impl 2.3
import QtQuick.Controls 1.4 as Controls_1_4

import QtGraphicalEffects 1.0

import "../app_items"

Controls_1_4.Tab {

    property string balance_label_str: qsTr("Balance:  ------  ")
    property string tsr_blance_can_get_label_str: " ------  "
    property string tsr_blance_pending_amount_label_str: " ------  "
    property string tsr_blance_immature_amount_label_str: " ------  "
    property string tsr_blance_total_amount_label_str: " ------  "

    property string zero_blance_mature_amount_label_str: " ------  "
    property string zero_blance_unconfirmed_amount_label_str: " ------  "
    property string zero_blance_immature_amount_label_str: " ------  "
    property string zero_blance_total_amount_label_str: " ------  "


    property string combined_blance_TSR_amount_label_str: " ------  "
    property string combined_blance_locked_amount_label_str: " ------  "
    property string combined_blance_unlocked_amount_label_str: " ------  "
    property string combined_blance_zTSR_amount_label_str: " ------  "
    property string combined_blance_total_amount_label_str: " ------  "


    Component.onCompleted:
    {
        walletModel.balanceChanged.connect(setBalance)
    }


    function setBalance(balance,
                        unconfirmedBalance,
                        immatureBalance,
                        zerocoinBalance,
                        unconfirmedZerocoinBalance,
                        immatureZerocoinBalance,
                        watchOnlyBalance,
                        watchUnconfBalance,
                        watchImmatureBalance)
    {
        balance_label_str =  qsTr("Balance: ") + walletModel.formatAmount(balance) + unitName
        tsr_blance_can_get_label_str =  walletModel.formatAmount(balance-unconfirmedBalance-immatureBalance) + unitName
        tsr_blance_pending_amount_label_str = walletModel.formatAmount(unconfirmedBalance) + unitName
        tsr_blance_immature_amount_label_str = walletModel.formatAmount(immatureBalance) + unitName
        tsr_blance_total_amount_label_str = walletModel.formatAmount(balance) + unitName


        zero_blance_mature_amount_label_str =  walletModel.formatAmount(zerocoinBalance - immatureZerocoinBalance) + unitName
        zero_blance_unconfirmed_amount_label_str = walletModel.formatAmount(unconfirmedZerocoinBalance) + unitName
        zero_blance_immature_amount_label_str = walletModel.formatAmount(immatureZerocoinBalance) + unitName
        zero_blance_total_amount_label_str = walletModel.formatAmount(zerocoinBalance) + unitName

        combined_blance_TSR_amount_label_str =  walletModel.formatAmount(balance + unconfirmedBalance) + unitName
        combined_blance_locked_amount_label_str = walletModel.formatAmount(walletModel.getlockedCoins()) + unitName
        combined_blance_unlocked_amount_label_str = walletModel.formatAmount(balance + unconfirmedBalance - walletModel.getlockedCoins()) + unitName
        combined_blance_zTSR_amount_label_str = walletModel.formatAmount(zerocoinBalance) + unitName
        combined_blance_total_amount_label_str = walletModel.formatAmount(balance + unconfirmedBalance + zerocoinBalance) + unitName
    }



    Rectangle {
        anchors.fill:parent

        radius: 0
        color: "#FAFAFA"



        CommonSelectableLabel {
            id:balance_label
            font.weight: Font.Normal
            font.pixelSize:26
            anchors.top:parent.top
            anchors.left: parent.left
            anchors.topMargin: 25
            anchors.leftMargin: 22
            text:balance_label_str
        }

        Label{
            id:status_label
            font.weight: Font.Light
            font.pixelSize:14
            anchors.top:balance_label.bottom
            anchors.left: balance_label.left
            anchors.topMargin: 18
            color: "#3D4966"
            text:qsTr("MasterNode:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\<font color=\"#3285C4\">Unknown\<\/font\>")
        }

        CommonButton{
            id:receive_btn
            color: "#3285C4"
            anchors.left: status_label.left
            anchors.top: status_label.bottom
            anchors.topMargin: 115
            width: 60
            height: 40
            radius: 2

            Image
            {
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                source: "../../images/icons/receive_btn.png"
                width:24
            }

            onClicked:
            {
                gotoReceivepage()
            }

        }

        Label {
            id:receive_label
            font.weight: Font.Normal
            font.pixelSize:16
            anchors.left: receive_btn.right
            anchors.leftMargin: 11
            anchors.verticalCenter: receive_btn.verticalCenter
            color: "#3D4966"
            text:qsTr("Receiving Address")
        }

        Label{
            id:transaction_label
            font.weight: Font.Normal
            font.pixelSize:18
            anchors.left: balance_label.left
            anchors.top: receive_label.bottom
            anchors.topMargin: 82
            color: walletModel.isTestNet()?"#202020":"#3285C4"
            text:qsTr("Recent transactions")

        }

        //TODO:dynamic uptate will exceed the limit of rowCount() fix this!

        ListView {
            id: recent_transaction_info_listView
            clip:true
            focus: true
            currentIndex: -1
            width:550
            boundsBehavior:Flickable.StopAtBounds
            anchors.top:transaction_label.bottom
            anchors.topMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.left: balance_label.left
            model: walletModel.transaction_ProxyModelOverView
            cacheBuffer: 1000
            spacing: 10
            delegate: ItemDelegate {
                width: 550
                height:35
                hoverEnabled: false


                Image
                {
                    id:typeImg
                    width:25
                    height:25
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    fillMode: Image.PreserveAspectFit
                    verticalAlignment: Image.AlignVCenter
                    horizontalAlignment: Image.AlignTop
                    source: address__
                }

                Label {
                    id:timeLabel
                    anchors.top: parent.top
                    anchors.left: typeImg.right
                    anchors.leftMargin: 10
                    font.pixelSize: 14
                    lineHeight: 0.8
                    text:date
                    color:"#3D4966"
                    font.weight: Font.Light
                    font.letterSpacing: 0.5
                }

                Label {
                    id:amountLabel
                    anchors.top: parent.top
                    anchors.right: parent.right
                    font.pixelSize: 14
                    lineHeight: 0.8
                    text:amount__ + unitName
                    color:amount__ < 0 ?"#FF0000":(confirmed?"#3285C4":"#808080")
                    font.weight: Font.Light
                    font.letterSpacing: 0.5
                }

                Label {
                    id:addressLabel
                    anchors.top: timeLabel.bottom
                    anchors.bottomMargin: 10
                    anchors.left: timeLabel.left

                    font.pixelSize: 14
                    lineHeight: 0.8
                    text:address___
                    color:amount__ < 0 ?"#808080":"#3D4966"
                    font.weight: Font.Light
                    font.letterSpacing: 0.5
                }

                onClicked:
                {
                    //tab_change(3)
                    gotoTransactionPage(recent_transaction_info_listView.model.mapToSource(recent_transaction_info_listView.model.index(index,0)))
                }




            }





        }

        Label {
            id:transaction_status_label
            font.weight: Font.Normal
            font.pixelSize:18
            anchors.left: transaction_label.right
            anchors.leftMargin: 73
            anchors.top: transaction_label.top
            color: "#EF5653"
            text:walletModel.syncStatus

        }





       //--------------TSR Balance
        Label
        {
            id:tsr_blance_label
            font.weight: Font.Normal
            font.pixelSize:18
            anchors.right: parent.right
            anchors.rightMargin: 400
            anchors.top: parent.top
            anchors.topMargin: 35
            color: walletModel.isTestNet()?"#202020":"#3285C4"
            text:qsTr("TSR Balances")
        }


        Label {
            id:tsr_blance_can_get_label
            anchors.top: tsr_blance_label.bottom
            anchors.topMargin: 10
            anchors.left: tsr_blance_label.left
            font.pixelSize: 14
            text:qsTr("Available:")
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:tsr_blance_can_get_amount_label
            anchors.verticalCenter: tsr_blance_can_get_label.verticalCenter
            anchors.left: tsr_blance_can_get_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:tsr_blance_can_get_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }


        Label {
            id:tsr_blance_pending_label
            anchors.top: tsr_blance_can_get_label.bottom
            anchors.topMargin: 13
            anchors.left: tsr_blance_can_get_label.left
            font.pixelSize: 14
            text:qsTr("Pending:")
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:tsr_blance_pending_amount_label
            anchors.verticalCenter: tsr_blance_pending_label.verticalCenter
            anchors.left: tsr_blance_pending_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:tsr_blance_pending_amount_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }

        Label {
            id:tsr_blance_immature_label
            anchors.top: tsr_blance_pending_label.bottom
            anchors.topMargin: 13
            anchors.left: tsr_blance_pending_label.left
            font.pixelSize: 14
            text:qsTr("Immature:")
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:tsr_blance_immature_amount_label
            anchors.verticalCenter: tsr_blance_immature_label.verticalCenter
            anchors.left: tsr_blance_immature_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:tsr_blance_immature_amount_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }


        Label {
            id:tsr_blance_total_label
            anchors.top: tsr_blance_immature_label.bottom
            anchors.topMargin: 13
            anchors.left: tsr_blance_immature_label.left
            font.pixelSize: 14
            text:qsTr("Total:")
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:tsr_blance_total_amount_label
            anchors.verticalCenter: tsr_blance_total_label.verticalCenter
            anchors.left: tsr_blance_total_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:tsr_blance_total_amount_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }

        //--------------ZeroCoin Balance
        Label
        {
            id:zero_blance_label
            font.weight: Font.Normal
            font.pixelSize:18
            anchors.left: tsr_blance_total_label.left
            anchors.top: transaction_label.top

            color: walletModel.isTestNet()?"#202020":"#3285C4"
            text:qsTr("Zerocoin Balance")
        }


        Label {
            id:zero_blance_total_label
            anchors.top: zero_blance_label.bottom
            anchors.topMargin: 10
            anchors.left: zero_blance_label.left
            font.pixelSize: 14
            text:qsTr("Total:")
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:zero_blance_total_amount_label
            anchors.verticalCenter: zero_blance_total_label.verticalCenter
            anchors.left: zero_blance_total_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:zero_blance_total_amount_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }

        Label {
            id:zero_blance_unconfirmed_label
            anchors.top: zero_blance_total_label.bottom
            anchors.topMargin: 13
            anchors.left: zero_blance_label.left
            font.pixelSize: 14
            text:qsTr("Unconfirmed:")
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:zero_blance_unconfirmed_amount_label
            anchors.verticalCenter: zero_blance_unconfirmed_label.verticalCenter
            anchors.left: zero_blance_unconfirmed_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:zero_blance_unconfirmed_amount_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }


        Label {
            id:zero_blance_immature_label
            anchors.top: zero_blance_unconfirmed_label.bottom
            anchors.topMargin: 13
            anchors.left: zero_blance_label.left
            font.pixelSize: 14
            text:qsTr("Immature:")
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:zero_blance_immature_amount_label
            anchors.verticalCenter: zero_blance_immature_label.verticalCenter
            anchors.left: zero_blance_immature_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:zero_blance_immature_amount_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }



        Label {
            id:zero_blance_mature_label
            anchors.top: zero_blance_immature_label.bottom
            anchors.topMargin: 13
            anchors.left: zero_blance_label.left
            font.pixelSize: 14
            text:qsTr("Mature:")
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:zero_blance_mature_amount_label
            anchors.verticalCenter: zero_blance_mature_label.verticalCenter
            anchors.left: zero_blance_mature_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:zero_blance_mature_amount_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }



        //--------------Combined Balances
        Label
        {
            id:combined_blance_label
            font.weight: Font.Normal
            font.pixelSize:18
            anchors.left: tsr_blance_total_label.left
            anchors.top: zero_blance_mature_amount_label.bottom
            anchors.topMargin: 46
            color: walletModel.isTestNet()?"#202020":"#3285C4"
            text:qsTr("Combined Balances")
        }


        Label {
            id:combined_tsr_label
            anchors.top: combined_blance_label.bottom
            anchors.topMargin: 11
            anchors.left: combined_blance_label.left
            font.pixelSize: 14
            text:qsTr("TSR:")
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:combined_tsr_label_amount_label
            anchors.verticalCenter: combined_tsr_label.verticalCenter
            anchors.left: combined_tsr_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:combined_blance_TSR_amount_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }


        Label {
            id:combined_locked_label
            anchors.top: combined_tsr_label.bottom
            anchors.topMargin: 11
            anchors.left: combined_tsr_label.left
            font.pixelSize: 14
            text:qsTr("Locked:")
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:combined_locked_amount_label
            anchors.verticalCenter: combined_locked_label.verticalCenter
            anchors.left: combined_locked_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:combined_blance_locked_amount_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }


        Label {
            id:combined_unlocked_label
            anchors.top: combined_locked_label.bottom
            anchors.topMargin: 11
            anchors.left: combined_locked_label.left
            font.pixelSize: 14
            text:"Unlocked:"
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:combined_unlocked_amount_label
            anchors.verticalCenter: combined_unlocked_label.verticalCenter
            anchors.left: combined_unlocked_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:combined_blance_unlocked_amount_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }


        Label {
            id:combined_ztsr_label
            anchors.top: combined_unlocked_label.bottom
            anchors.topMargin: 11
            anchors.left: combined_unlocked_label.left
            font.pixelSize: 14
            text:qsTr("zTSR:")
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:combined_ztsr_amount_label
            anchors.verticalCenter: combined_ztsr_label.verticalCenter
            anchors.left: combined_ztsr_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:combined_blance_zTSR_amount_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }

        Label {
            id:combined_total_label
            anchors.top: combined_ztsr_label.bottom
            anchors.topMargin: 11
            anchors.left: combined_ztsr_label.left
            font.pixelSize: 14
            text:qsTr("Total:")
            color:"#3D4966"
            font.weight: Font.Light
            font.letterSpacing: 0.5

        }

        CommonSelectableLabel {
            id:combined_total_amount_label
            anchors.verticalCenter: combined_total_label.verticalCenter
            anchors.left: combined_total_label.left
            anchors.leftMargin: 140
            font.pixelSize: 14
            text:combined_blance_total_amount_label_str
            textColor:"#3D4966"
            font.weight: Font.Normal
            font.letterSpacing: 0.5

        }

    }

}
