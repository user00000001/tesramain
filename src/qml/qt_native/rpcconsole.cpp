// Copyright (c) 2011-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcconsole.h"

#include "clientmodel.h"
#include "guiutil.h"
#include "peertablemodel.h"

#include "chainparams.h"
#include "main.h"
#include "rpcclient.h"
#include "rpcserver.h"
#include "util.h"
#include "init.h"

#include <openssl/crypto.h>

#include <univalue.h>

#ifdef ENABLE_WALLET
#include <db_cxx.h>
#endif

#include <QDir>
#include <QKeyEvent>
#include <QScrollBar>
#include <QThread>
#include <QTime>
#include <QStringList>
#include <QApplication>

#if QT_VERSION < 0x050000
#include <QUrl>
#endif

// TODO: add a scrollback limit, as there is currently none
// TODO: make it possible to filter out categories (esp debug messages when implemented)
// TODO: receive errors and debug messages through ClientModel

const int CONSOLE_HISTORY = 50;
const QSize ICON_SIZE(24, 24);

const int INITIAL_TRAFFIC_GRAPH_MINS = 30;

// Repair parameters
const QString SALVAGEWALLET("-salvagewallet");
const QString RESCAN("-rescan");
const QString ZAPTXES1("-zapwallettxes=1");
const QString ZAPTXES2("-zapwallettxes=2");
const QString UPGRADEWALLET("-upgradewallet");
const QString REINDEX("-reindex");
const QString RESYNC("-resync");

const struct {
    const char* url;
    const char* source;
} ICON_MAPPING[] = {
{"cmd-request", ":/icons/tx_input"},
{"cmd-reply", ":/icons/tx_output"},
{"cmd-error", ":/icons/tx_output"},
{"misc", ":/icons/tx_inout"},
{NULL, NULL}};

/* Object for executing console RPC commands in a separate thread.
*/
class RPCExecutor : public QObject
{
    Q_OBJECT

public slots:
    void request(const QString& command);

signals:
    void reply(int category, const QString& command);
};

#include "rpcconsole.moc"

/**
 * Split shell command line into a list of arguments. Aims to emulate \c bash and friends.
 *
 * - Arguments are delimited with whitespace
 * - Extra whitespace at the beginning and end and between arguments will be ignored
 * - Text can be "double" or 'single' quoted
 * - The backslash \c \ is used as escape character
 *   - Outside quotes, any character can be escaped
 *   - Within double quotes, only escape \c " and backslashes before a \c " or another backslash
 *   - Within single quotes, no escaping is possible and no special interpretation takes place
 *
 * @param[out]   args        Parsed arguments will be appended to this list
 * @param[in]    strCommand  Command line to split
 */
bool RPCConsole::parseCommandLine(std::vector<std::string>& args, const std::string& strCommand)
{
    enum CmdParseState {
        STATE_EATING_SPACES,
        STATE_ARGUMENT,
        STATE_SINGLEQUOTED,
        STATE_DOUBLEQUOTED,
        STATE_ESCAPE_OUTER,
        STATE_ESCAPE_DOUBLEQUOTED
    } state = STATE_EATING_SPACES;
    std::string curarg;
    foreach (char ch, strCommand) {
        switch (state) {
        case STATE_ARGUMENT:      // In or after argument
        case STATE_EATING_SPACES: // Handle runs of whitespace
            switch (ch) {
            case '"':
                state = STATE_DOUBLEQUOTED;
                break;
            case '\'':
                state = STATE_SINGLEQUOTED;
                break;
            case '\\':
                state = STATE_ESCAPE_OUTER;
                break;
            case ' ':
            case '\n':
            case '\t':
                if (state == STATE_ARGUMENT) // Space ends argument
                {
                    args.push_back(curarg);
                    curarg.clear();
                }
                state = STATE_EATING_SPACES;
                break;
            default:
                curarg += ch;
                state = STATE_ARGUMENT;
            }
            break;
        case STATE_SINGLEQUOTED: // Single-quoted string
            switch (ch) {
            case '\'':
                state = STATE_ARGUMENT;
                break;
            default:
                curarg += ch;
            }
            break;
        case STATE_DOUBLEQUOTED: // Double-quoted string
            switch (ch) {
            case '"':
                state = STATE_ARGUMENT;
                break;
            case '\\':
                state = STATE_ESCAPE_DOUBLEQUOTED;
                break;
            default:
                curarg += ch;
            }
            break;
        case STATE_ESCAPE_OUTER: // '\' outside quotes
            curarg += ch;
            state = STATE_ARGUMENT;
            break;
        case STATE_ESCAPE_DOUBLEQUOTED:                  // '\' in double-quoted text
            if (ch != '"' && ch != '\\') curarg += '\\'; // keep '\' for everything but the quote and '\' itself
            curarg += ch;
            state = STATE_DOUBLEQUOTED;
            break;
        }
    }
    switch (state) // final state
    {
    case STATE_EATING_SPACES:
        return true;
    case STATE_ARGUMENT:
        args.push_back(curarg);
        return true;
    default: // ERROR to end in one of the other states
        return false;
    }
}

void RPCExecutor::request(const QString& command)
{
    std::vector<std::string> args;
    if (!RPCConsole::parseCommandLine(args, command.toStdString())) {
        emit reply(RPCConsole::CMD_ERROR, QString("Parse error: unbalanced ' or \""));
        return;
    }
    if (args.empty())
        return; // Nothing to do
    try {
        std::string strPrint;
        // Convert argument list to JSON objects in method-dependent way,
        // and pass it along with the method name to the dispatcher.
        UniValue result = tableRPC.execute(
                    args[0],
                RPCConvertValues(args[0], std::vector<std::string>(args.begin() + 1, args.end())));

        // Format result reply
        if (result.isNull())
            strPrint = "";
        else if (result.isStr())
            strPrint = result.get_str();
        else
            strPrint = result.write(2);

        emit reply(RPCConsole::CMD_REPLY, QString::fromStdString(strPrint));
    } catch (UniValue& objError) {
        try // Nice formatting for standard-format error
        {
            int code = find_value(objError, "code").get_int();
            std::string message = find_value(objError, "message").get_str();
            emit reply(RPCConsole::CMD_ERROR, QString::fromStdString(message) + " (code " + QString::number(code) + ")");
        } catch (std::runtime_error&) // raised when converting to invalid type, i.e. missing code or message
        {                             // Show raw JSON object
            emit reply(RPCConsole::CMD_ERROR, QString::fromStdString(objError.write()));
        }
    } catch (std::exception& e) {
        emit reply(RPCConsole::CMD_ERROR, QString("Error: ") + QString::fromStdString(e.what()));
    }
}


RPCConsole::RPCConsole(QObject *parent) : QObject(parent)
{
    //  ui->setupUi(this);
    //   GUIUtil::restoreWindowGeometry("nRPCConsoleWindow", this->size(), this);

    //#ifndef Q_OS_MAC
    //    ui->openDebugLogfileButton->setIcon(QIcon(":/icons/export"));
    //#endif

    //   // Install event filter for up and down arrow
    //   ui->lineEdit->installEventFilter(this);
    //   ui->messagesWidget->installEventFilter(this);

    //   connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    //   connect(ui->btnClearTrafficGraph, SIGNAL(clicked()), ui->trafficGraph, SLOT(clear()));

    // Wallet Repair Buttons
    //   connect(ui->btn_salvagewallet, SIGNAL(clicked()), this, SLOT(walletSalvage()));
    //   connect(ui->btn_rescan, SIGNAL(clicked()), this, SLOT(walletRescan()));
    //   connect(ui->btn_zapwallettxes1, SIGNAL(clicked()), this, SLOT(walletZaptxes1()));
    //   connect(ui->btn_zapwallettxes2, SIGNAL(clicked()), this, SLOT(walletZaptxes2()));
    //   connect(ui->btn_upgradewallet, SIGNAL(clicked()), this, SLOT(walletUpgrade()));
    //   connect(ui->btn_reindex, SIGNAL(clicked()), this, SLOT(walletReindex()));
    //   connect(ui->btn_resync, SIGNAL(clicked()), this, SLOT(walletResync()));

    // set library version labels
    //    ui->openSSLVersion->setText(SSLeay_version(SSLEAY_VERSION));
    //#ifdef ENABLE_WALLET
    //    ui->berkeleyDBVersion->setText(DbEnv::version(0, 0, 0));
    //    ui->wallet_path->setText(QString::fromStdString(GetDataDir().string() + QDir::separator().toLatin1() + GetArg("-wallet", "wallet.dat")));
    //#else
    //    ui->label_berkeleyDBVersion->hide();
    //    ui->berkeleyDBVersion->hide();
    //#endif

    startExecutor();
    //    setTrafficGraphRange(INITIAL_TRAFFIC_GRAPH_MINS);

    //    ui->peerHeading->setText(tr("Select a peer to view detailed information."));

}

QString RPCConsole::getWalletPath()
{
    return QString::fromStdString(GetDataDir().string() + QDir::separator().toLatin1() + GetArg("-wallet", "wallet.dat"));
}

void RPCConsole::startFromQML()
{
    clear();
}

QString RPCConsole::getStartUpInfo()
{
    QString str = "<br>";

    clientName = clientModel->clientName();
    clientversion = clientModel->formatFullVersion();
    openSSLVersion = SSLeay_version(SSLEAY_VERSION);
    berkeleyDBVersion = DbEnv::version(0, 0, 0);
    buildDate = clientModel->formatBuildDate();
    startupTime = clientModel->formatClientStartupTime();

    networkName = QString::fromStdString(Params().NetworkIDString());
    connections = QString::number(clientModel->getNumConnections()) + " (";
    connections += tr("In:") + " " + QString::number(clientModel->getNumConnections(CONNECTIONS_IN)) + " / ";
    connections += tr("Out:") + " " + QString::number(clientModel->getNumConnections(CONNECTIONS_OUT)) + ")";
    MasternodeCount =  clientModel->getMasternodeCountString();

    NumBlocks = QString::number(clientModel->getNumBlocks());
    LastBlockDate = clientModel->getLastBlockDate().toString();

    str += clientName + "<br>" + clientversion + "<br>" + openSSLVersion + "<br>" + berkeleyDBVersion + "<br>"
            + buildDate + "<br>" + startupTime + "<br><br>" + networkName + "<br>" + connections + "<br>" + MasternodeCount + "<br><br>"
            + NumBlocks + "<br>" + LastBlockDate;

    return str;
}


QString RPCConsole::getStartUpTraffic()
{
    QString str;
    str = "Totals\n  Received:\t" + FormatBytes(clientModel->getTotalBytesRecv()) + "\n  Sent:\t" + FormatBytes(clientModel->getTotalBytesSent());
    return str;
}

RPCConsole::~RPCConsole()
{
    // GUIUtil::saveWindowGeometry("nRPCConsoleWindow", this);
    emit stopExecutor();
    //delete ui;
}

ClientModel *RPCConsole::getClientModel()
{
    return clientModel;
}







void RPCConsole::setClientModel(ClientModel* model)
{
    clientModel = model;

    // peerTableModel  = new QSortFilterProxyModel(this);
    //  peerTableModel->setSourceModel(clientModel->getPeerTableModel());
    //  peerTableModel->setSortRole(PeerTableModel::AddressRole);

    //  peerTableModel->sort(0, Qt::DescendingOrder);
    peerTableModel = clientModel->getPeerTableModel();



    peerTableproxy = new PeerTableProxy(this);
    peerTableproxy->setSourceModel(peerTableModel);
    peerTableproxy->setSortRole(Qt::EditRole);
    peerTableproxy->setDynamicSortFilter(true);
    peerTableproxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    peerTableproxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    peerTableproxy->sort(PeerTableModel::Ping,Qt::DescendingOrder);

    //ui->trafficGraph->setClientModel(model);
    if (model) {
        // Keep up to date with client
        //        setNumConnections(model->getNumConnections());
        connect(model, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        //        setNumBlocks(model->getNumBlocks());
        connect(model, SIGNAL(numBlocksChanged(int)), this, SLOT(setNumBlocks(int)));

        //        setMasternodeCount(model->getMasternodeCountString());
        connect(model, SIGNAL(strMasternodesChanged(QString)), this, SLOT(setMasternodeCount(QString)));

        //        updateTrafficStats(model->getTotalBytesRecv(), model->getTotalBytesSent());
        connect(model, SIGNAL(bytesChanged(quint64, quint64)), this, SLOT(updateTrafficStats(quint64, quint64)));

        //        // set up peer table
        //        ui->peerWidget->setModel(model->getPeerTableModel());
        //        ui->peerWidget->verticalHeader()->hide();
        //        ui->peerWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        //        ui->peerWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        //        ui->peerWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        //        ui->peerWidget->setColumnWidth(PeerTableModel::Address, ADDRESS_COLUMN_WIDTH);
        //        ui->peerWidget->setColumnWidth(PeerTableModel::Subversion, SUBVERSION_COLUMN_WIDTH);
        //        ui->peerWidget->setColumnWidth(PeerTableModel::Ping, PING_COLUMN_WIDTH);

        // connect the peerWidget selection model to our peerSelected() handler
        //        connect(ui->peerWidget->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        //            this, SLOT(peerSelected(const QItemSelection&, const QItemSelection&)));
        //        connect(model->getPeerTableModel(), SIGNAL(layoutChanged()), this, SLOT(peerLayoutChanged()));

        // Provide initial values
        //        ui->clientVersion->setText(model->formatFullVersion());
        //        ui->clientName->setText(model->clientName());
        //        ui->buildDate->setText(model->formatBuildDate());
        //        ui->startupTime->setText(model->formatClientStartupTime());
        //        ui->networkName->setText(QString::fromStdString(Params().NetworkIDString()));

        //Setup autocomplete and attach it
        std::vector<std::string> commandList = tableRPC.listCommands();
        for (size_t i = 0; i < commandList.size(); ++i)
        {
            wordList << commandList[i].c_str();
        }

        m_hintModel = new SingleItemModel(this);

        // autoCompleter = new QCompleter(wordList, this);
        //ui->lineEdit->setCompleter(autoCompleter);

        // clear the lineEdit after activating from QCompleter
        // autoCompleter->popup()->installEventFilter(this);
    }
}


void RPCConsole::findhint(QString text)
{

    m_hintModel->ClearSingleItem();

    QRegExp rx("^(" + text + ".*)");

    int size = wordList.size();
    int pos;
    for(int i = 0 ; i < size;i++){

        pos = 0;
        if ((pos = rx.indexIn(wordList.at(i), pos)) != -1) {
            m_hintModel->addSingleItem(rx.cap(1));
        }
    }

}

int RPCConsole::hintRowCount()
{
    m_hintModel->rowCount();
}


static QString categoryIcon(int category)
{
    switch (category) {
    case RPCConsole::CMD_REQUEST:
        return "qrc:/images/icons/tx_input.png";
        break;
    case RPCConsole::CMD_REPLY:
        return "qrc:/images/icons/tx_output.png";
        break;
    case RPCConsole::CMD_ERROR:
        return "qrc:/images/icons/tx_output.png";
        break;
    default:
        return "qrc:/images/icons/tx_input.png";
    }
}

static QString categoryColor(int category)
{
    switch (category) {
    case RPCConsole::CMD_REQUEST:
        return "#006060";
        break;
    case RPCConsole::CMD_REPLY:
        return "#3B3B3B";
        break;
    case RPCConsole::CMD_ERROR:
        return "red";
        break;
    default:
        return "#3B3B3B";
    }
}

/** Restart wallet with "-salvagewallet" */
void RPCConsole::walletSalvage()
{
     buildParameterlist(SALVAGEWALLET);
}

/** Restart wallet with "-rescan" */
void RPCConsole::walletRescan()
{
     buildParameterlist(RESCAN);
}

/** Restart wallet with "-zapwallettxes=1" */
void RPCConsole::walletZaptxes1()
{
     buildParameterlist(ZAPTXES1);
}

/** Restart wallet with "-zapwallettxes=2" */
void RPCConsole::walletZaptxes2()
{
     buildParameterlist(ZAPTXES2);
}

/** Restart wallet with "-upgradewallet" */
void RPCConsole::walletUpgrade()
{
      buildParameterlist(UPGRADEWALLET);
}

/** Restart wallet with "-reindex" */
void RPCConsole::walletReindex()
{
      buildParameterlist(REINDEX);
}

/** Restart wallet with "-resync" */
void RPCConsole::walletResync()
{

        // Restart and resync
        buildParameterlist(RESYNC);
}

/** Build command-line parameter list for restart */
void RPCConsole::buildParameterlist(QString arg)
{
   //  Get command-line arguments and remove the application name
        QStringList args = QApplication::arguments();
        args.removeFirst();

        // Remove existing repair-options
        args.removeAll(SALVAGEWALLET);
        args.removeAll(RESCAN);
        args.removeAll(ZAPTXES1);
        args.removeAll(ZAPTXES2);
        args.removeAll(UPGRADEWALLET);
        args.removeAll(REINDEX);

        // Append repair parameter to command line.
        args.append(arg);

    //    // Send command-line arguments to BitcoinGUI::handleRestart()
      //  emit handleRestart(args);

        if (!ShutdownRequested())
            emit requestedRestart(args);
}

void RPCConsole::changeNetWork()
{
    boost::filesystem::path pathConfig = GetConfigFile();
    if (!boost::filesystem::exists(pathConfig))
        return;

    QString pathStr = GUIUtil::boostPathToQString(pathConfig);

    QFile f(pathStr);
    f.open(QIODevice::ReadOnly);
    QString array = QString(f.readAll());
    f.close();

    if(GetBoolArg("-testnet", false))
        array = array.replace(QRegExp("testnet.*=.*1?\\n"),"");
    else
        array += "testnet=1\n";

    f.open(QIODevice::WriteOnly);
    f.write(array.toUtf8());
    f.close();

    if (!ShutdownRequested())
        emit requestedRestart(QStringList());

}

void RPCConsole::clear()
{
    // ui->messagesWidget->clear();
    history.clear();
    historyPtr = 0;
    //  ui->lineEdit->clear();
    // ui->lineEdit->setFocus();
    //
    // Add smoothly scaled icon images.
    // (when using width/height on an img, Qt uses nearest instead of linear interpolation)

    //    // Set default style sheet
    //    ui->messagesWidget->document()->setDefaultStyleSheet(
    //        "table { }"
    //        "td.time { color: #808080; padding-top: 3px; } "
    //        "td.message { font-family: Courier, Courier New, Lucida Console, monospace; font-size: 12px; } " // Todo: Remove fixed font-size
    //        "td.cmd-request { color: #006060; } "
    //        "td.cmd-error { color: red; } "
    //        "b { color: #006060; } ");

    message(CMD_REPLY, (tr("Welcome to the TESRA RPC console.") + "<br>" +
                        tr("Use up and down arrows to navigate history, and <b>Ctrl-L</b> to clear screen.") + "<br>" +
                        tr("Type <b>help</b> for an overview of available commands.")),
            true);
}

void RPCConsole::reject()
{
    // Ignore escape keypress if this is not a seperate window
    //   if (windowType() != Qt::Widget)
    //        QDialog::reject();
}

void RPCConsole::message(int category, const QString& message, bool html)
{
    QTime time = QTime::currentTime();
    QString timeString = time.toString();
    QString out;
    out += "<table><tr><td class=\"time\" width=\"65\">" + timeString + "</td>";
    out += "<td class=\"icon\" width=\"32\"><img src=\"" + categoryIcon(category) + "\"></td>";
    out += "<td class=\"message\" valign=\"middle\"><font color=\"" + categoryColor(category) +  "\">";
    if (html)
        out += message;
    else
        out += GUIUtil::HtmlEscape(message, true);
    out += "</td></tr></table>";
    emit newMessage(out);
}

void RPCConsole::setNumConnections(int count)
{
    connections = QString::number(count) + " (";
    connections += tr("In:") + " " + QString::number(clientModel->getNumConnections(CONNECTIONS_IN)) + " / ";
    connections += tr("Out:") + " " + QString::number(clientModel->getNumConnections(CONNECTIONS_OUT)) + ")";
    emittinginfoUpdate();
}

void RPCConsole::emittinginfoUpdate()
{
    QString str = "<br>";
    str += clientName + "<br>" + clientversion + "<br>" + openSSLVersion + "<br>" + berkeleyDBVersion + "<br>"
            + buildDate + "<br>" + startupTime + "<br><br>" + networkName + "<br>" + connections + "<br>" + MasternodeCount + "<br><br>"
            + NumBlocks + "<br>" + LastBlockDate;

    emit infoUpdate(str);

}

void RPCConsole::setNumBlocks(int count)
{
    NumBlocks = QString::number(count);
    emittinginfoUpdate();
}

void RPCConsole::setMasternodeCount(const QString& strMasternodes)
{
    MasternodeCount =  clientModel->getMasternodeCountString();
    emittinginfoUpdate();
}

void RPCConsole::on_lineEdit_returnPressed(QString cmd)
{

    if (!cmd.isEmpty()) {
        message(CMD_REQUEST, cmd);
        emit cmdRequest(cmd);
        // Remove command, if already in history
        history.removeOne(cmd);
        // Append command to history
        history.append(cmd);
        // Enforce maximum history size
        while (history.size() > CONSOLE_HISTORY)
            history.removeFirst();
        // Set pointer to end of history
        historyPtr = history.size();
        // Scroll console view to end
        //scrollToEnd();
    }
}

QString RPCConsole::browseHistory(int offset)
{

    historyPtr += offset;
    if (historyPtr < 0)
        historyPtr = 0;
    if (historyPtr > history.size())
        historyPtr = history.size();
    QString cmd="";
    if (historyPtr < history.size())
        cmd = history.at(historyPtr);

    return cmd;
}


void RPCConsole::startExecutor()
{
    QThread* thread = new QThread;
    RPCExecutor* executor = new RPCExecutor();
    executor->moveToThread(thread);

    // Replies from executor object must go to this object
    connect(executor, SIGNAL(reply(int, QString)), this, SLOT(message(int, QString)));
    // Requests from this object must go to executor
    connect(this, SIGNAL(cmdRequest(QString)), executor, SLOT(request(QString)));

    // On stopExecutor signal
    // - queue executor for deletion (in execution thread)
    // - quit the Qt event loop in the execution thread
    connect(this, SIGNAL(stopExecutor()), executor, SLOT(deleteLater()));
    connect(this, SIGNAL(stopExecutor()), thread, SLOT(quit()));
    // Queue the thread for deletion (in this thread) when it is finished
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    // Default implementation of QThread::run() simply spins up an event loop in the thread,
    // which is what we want.
    thread->start();
}

void RPCConsole::on_tabWidget_currentChanged(int index)
{
    //    if (ui->tabWidget->widget(index) == ui->tab_console) {
    //        ui->lineEdit->setFocus();
    //    }
}

void RPCConsole::openDebugLogFile()
{
    GUIUtil::openDebugLogfile();
}


QString RPCConsole::FormatBytes(quint64 bytes)
{
    if (bytes < 1024)
        return QString(tr("%1 B")).arg(bytes);
    if (bytes < 1024 * 1024)
        return QString(tr("%1 KB")).arg(bytes / 1024);
    if (bytes < 1024 * 1024 * 1024)
        return QString(tr("%1 MB")).arg(bytes / 1024 / 1024);

    return QString(tr("%1 GB")).arg(bytes / 1024 / 1024 / 1024);
}

QString RPCConsole::setTrafficGraphRange(int mins)
{
    //    ui->trafficGraph->setGraphRangeMins(mins);
    return GUIUtil::formatDurationStr(mins * 60);
}

void RPCConsole::updateTrafficStats(quint64 totalBytesIn, quint64 totalBytesOut)
{
    QString str;
    str = "Totals\n  Received:\t" + FormatBytes(totalBytesIn) + "\n  Sent:\t" + FormatBytes(totalBytesOut);

    emit trafficUpdate(str);
}


void RPCConsole::showConfEditor()
{
   GUIUtil::openConfigfile();
}

void RPCConsole::showMNConfEditor()
{
    GUIUtil::openMNConfigfile();
}

void RPCConsole::peerSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    //    Q_UNUSED(deselected);

    //    if (!clientModel || selected.indexes().isEmpty())
    //        return;

    //    const CNodeCombinedStats* stats = clientModel->getPeerTableModel()->getNodeStats(selected.indexes().first().row());
    //    if (stats)
    //        updateNodeDetail(stats);
}

void RPCConsole::peerLayoutChanged()
{
    //    if (!clientModel)
    //        return;

    //    const CNodeCombinedStats* stats = NULL;
    //    bool fUnselect = false;
    //    bool fReselect = false;

    //    if (cachedNodeid == -1) // no node selected yet
    //        return;

    //    // find the currently selected row
    //    int selectedRow;
    //    QModelIndexList selectedModelIndex = ui->peerWidget->selectionModel()->selectedIndexes();
    //    if (selectedModelIndex.isEmpty())
    //        selectedRow = -1;
    //    else
    //        selectedRow = selectedModelIndex.first().row();

    //    // check if our detail node has a row in the table (it may not necessarily
    //    // be at selectedRow since its position can change after a layout change)
    //    int detailNodeRow = clientModel->getPeerTableModel()->getRowByNodeId(cachedNodeid);

    //    if (detailNodeRow < 0) {
    //        // detail node dissapeared from table (node disconnected)
    //        fUnselect = true;
    //        cachedNodeid = -1;
    //        ui->peerHeading->setText(tr("Select a peer to view detailed information."));
    //    } else {
    //        if (detailNodeRow != selectedRow) {
    //            // detail node moved position
    //            fUnselect = true;
    //            fReselect = true;
    //        }

    //        // get fresh stats on the detail node.
    //        stats = clientModel->getPeerTableModel()->getNodeStats(detailNodeRow);
    //    }

    //    if (fUnselect && selectedRow >= 0) {
    //        ui->peerWidget->selectionModel()->select(QItemSelection(selectedModelIndex.first(), selectedModelIndex.last()),
    //            QItemSelectionModel::Deselect);
    //    }

    //    if (fReselect) {
    //        ui->peerWidget->selectRow(detailNodeRow);
    //    }

    //    if (stats)
    //        updateNodeDetail(stats);
}

void RPCConsole::updateNodeDetail(const CNodeCombinedStats* stats)
{
    //    // Update cached nodeid
    //    cachedNodeid = stats->nodeStats.nodeid;

    //    // update the detail ui with latest node information
    //    QString peerAddrDetails(QString::fromStdString(stats->nodeStats.addrName));
    //    if (!stats->nodeStats.addrLocal.empty())
    //        peerAddrDetails += "<br />" + tr("via %1").arg(QString::fromStdString(stats->nodeStats.addrLocal));
    //    ui->peerHeading->setText(peerAddrDetails);
    //    ui->peerServices->setText(GUIUtil::formatServicesStr(stats->nodeStats.nServices));
    //    ui->peerLastSend->setText(stats->nodeStats.nLastSend ? GUIUtil::formatDurationStr(GetTime() - stats->nodeStats.nLastSend) : tr("never"));
    //    ui->peerLastRecv->setText(stats->nodeStats.nLastRecv ? GUIUtil::formatDurationStr(GetTime() - stats->nodeStats.nLastRecv) : tr("never"));
    //    ui->peerBytesSent->setText(FormatBytes(stats->nodeStats.nSendBytes));
    //    ui->peerBytesRecv->setText(FormatBytes(stats->nodeStats.nRecvBytes));
    //    ui->peerConnTime->setText(GUIUtil::formatDurationStr(GetTime() - stats->nodeStats.nTimeConnected));
    //    ui->peerPingTime->setText(GUIUtil::formatPingTime(stats->nodeStats.dPingTime));
    //    ui->peerVersion->setText(QString("%1").arg(stats->nodeStats.nVersion));
    //    ui->peerSubversion->setText(QString::fromStdString(stats->nodeStats.cleanSubVer));
    //    ui->peerDirection->setText(stats->nodeStats.fInbound ? tr("Inbound") : tr("Outbound"));
    //    ui->peerHeight->setText(QString("%1").arg(stats->nodeStats.nStartingHeight));

    //    // This check fails for example if the lock was busy and
    //    // nodeStateStats couldn't be fetched.
    //    if (stats->fNodeStateStatsAvailable) {
    //        // Ban score is init to 0
    //        ui->peerBanScore->setText(QString("%1").arg(stats->nodeStateStats.nMisbehavior));

    //        // Sync height is init to -1
    //        if (stats->nodeStateStats.nSyncHeight > -1)
    //            ui->peerSyncHeight->setText(QString("%1").arg(stats->nodeStateStats.nSyncHeight));
    //        else
    //            ui->peerSyncHeight->setText(tr("Unknown"));
    //    } else {
    //        ui->peerBanScore->setText(tr("Fetching..."));
    //        ui->peerSyncHeight->setText(tr("Fetching..."));
    //    }

    //    ui->detailWidget->show();
}


void RPCConsole::showEvent(QShowEvent* event)
{
    //    QWidget::showEvent(event);

    //    if (!clientModel)
    //        return;

    //    // start PeerTableModel auto refresh
    //    clientModel->getPeerTableModel()->startAutoRefresh();
}

void RPCConsole::hideEvent(QHideEvent* event)
{
    //    QWidget::hideEvent(event);

    //    if (!clientModel)
    //        return;

    //    // stop PeerTableModel auto refresh
    //    clientModel->getPeerTableModel()->stopAutoRefresh();
}

void RPCConsole::showBackups()
{
    GUIUtil::showBackups();
}






