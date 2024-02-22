#include "tcpwidget.h"
#include "tcpclient.h"
#include "tcpserver.h"

#include <extensionsystem/pluginmanager.h>
#include <gui/messbox.h>
#include <utils/utils.h>
#include <utils/validator.hpp>

#include <QHostAddress>
#include <QNetworkInterface>
#include <QtWidgets>

namespace Plugin {

auto formatHex(const QByteArray &msg) -> QString
{
    QString temp;
    auto hex = QString::fromLocal8Bit(msg.toHex().toUpper());
    for (int i = 0; i < hex.length(); i = i + 2) {
        temp += hex.mid(i, 2) + " "; //两个字符+空格（例子：7e ）
    }
    return temp;
}

class TcpWidget::TcpWidgetPrivate
{
public:
    explicit TcpWidgetPrivate(TcpWidget *q)
        : q_ptr(q)
    {
        displayTextEdit = new QTextEdit(q_ptr);
        displayTextEdit->document()->setMaximumBlockCount(1000);
        displayTextEdit->setReadOnly(true);

        sendTextEdit = new QTextEdit(q_ptr);
        sendButton = new QPushButton(QCoreApplication::translate("TcpWidgetPrivate", "Send"), q_ptr);
        sendButton->setObjectName("BlueButton");
        sendButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

        modelBox = new QComboBox(q_ptr);
        modelBox->addItems({"TcpServer", "TcpClient"});
        ipLabel = new QLabel(QCoreApplication::translate("TcpWidgetPrivate", "Local IP List: "),
                             q_ptr);
        localIPBox = new QComboBox(q_ptr);
        serverIPEdit = new QLineEdit(q_ptr);
        serverIPEdit->setPlaceholderText(
            QCoreApplication::translate("TcpWidgetPrivate", "Please enter the server IP address."));
        QRegularExpression regExp(
            "^((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)$");
        auto *validator = new QRegularExpressionValidator(regExp, serverIPEdit);
        serverIPEdit->setValidator(validator);
        portLabel = new QLabel(QCoreApplication::translate("TcpWidgetPrivate", "Local Port: "),
                               q_ptr);
        portEdit = new QLineEdit(q_ptr);
        portEdit->setPlaceholderText(
            QCoreApplication::translate("TcpWidgetPrivate", "Please enter the port number."));
        portEdit->setValidator(new Utils::IntValidator(0, 65536, portEdit));
        listenOrConnectButton = new QPushButton(q_ptr);
        listenOrConnectButton->setCheckable(true);
        listenOrConnectButton->setObjectName("GrayButton");

        hexBox = new QCheckBox(QCoreApplication::translate("TcpWidgetPrivate", "Hex"), q_ptr);
        autoSendBox = new QCheckBox(QCoreApplication::translate("TcpWidgetPrivate", "Auto Delivery"),
                                    q_ptr);
        autoSendTimeBox = new QSpinBox(q_ptr);
        autoSendTimeBox->setSuffix(QCoreApplication::translate("TcpWidgetPrivate", " ms"));
        autoSendTimeBox->setRange(0, 10000);
        autoSendTimeBox->setValue(1000);
        autoSendTimeBox->setSingleStep(50);

        allConnectBox = new QComboBox(q_ptr);
        allConnectBox->addItem(QCoreApplication::translate("TcpWidgetPrivate", "Connect All"));
        autoConnectBox = new QCheckBox(QCoreApplication::translate("TcpWidgetPrivate",
                                                                   "Auto Reconnect"),
                                       q_ptr);
        autoConnectTimeBox = new QSpinBox(q_ptr);
        autoConnectTimeBox->setSuffix(QCoreApplication::translate("TcpWidgetPrivate", " ms"));
        autoConnectTimeBox->setRange(1000, 100000);
        autoConnectTimeBox->setValue(1000);
        autoConnectTimeBox->setSingleStep(50);

        sendConutButton = new QPushButton(q_ptr);
        recvConutButton = new QPushButton(q_ptr);
        saveButton = new QPushButton(QCoreApplication::translate("TcpWidgetPrivate", "Save Data"),
                                     q_ptr);
        clearButton = new QPushButton(QCoreApplication::translate("TcpWidgetPrivate",
                                                                  "Clear Screen"),
                                      q_ptr);

        setWidget = new QWidget(q_ptr);
    }

    void setupUI()
    {
        auto *displayBox = createDisplayWidget();
        auto *sendBox = createSendWidget();

        auto *splitter1 = new QSplitter(Qt::Vertical, q_ptr);
        splitter1->addWidget(displayBox);
        splitter1->addWidget(sendBox);
        splitter1->setHandleWidth(0);
        splitter1->setSizes(QList<int>{400, 1});

        auto *settingBox = createSettingsBox();

        auto *splitter2 = new QSplitter(Qt::Horizontal, q_ptr);
        splitter2->addWidget(splitter1);
        splitter2->addWidget(settingBox);
        splitter2->setHandleWidth(10);
        splitter2->setSizes(QList<int>{400, 1});

        auto *layout = new QHBoxLayout(q_ptr);
        layout->addWidget(splitter2);
    }

    void initWindow() const
    {
        localIPBox->clear();
        auto ipList = QNetworkInterface::allAddresses();
        for (const auto &address : std::as_const(ipList)) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol) {
                localIPBox->addItem(address.toString());
            }
        }
        localIPBox->setCurrentIndex(localIPBox->count() - 1);
    }

    void setSendCount() const
    {
        sendConutButton->setText(
            QCoreApplication::translate("TcpWidgetPrivate", "Send: %1 Bytes").arg(sendCount));
    }

    void setRecvCount() const
    {
        recvConutButton->setText(
            QCoreApplication::translate("TcpWidgetPrivate", "Recv: %1 Bytes").arg(recvCount));
    }

    void clearCount()
    {
        sendCount = 0;
        recvCount = 0;
        setSendCount();
        setRecvCount();
    }

    void appendDisplay(TcpWidget::MessageType type, const QString &message) const
    {
        if (message.isEmpty()) {
            return;
        }
        QString display;
        switch (type) {
        case Send:
            display = QCoreApplication::translate("TcpWidgetPrivate", " >> Network Send: ");
            displayTextEdit->setTextColor(Qt::black);
            break;
        case Recv:
            display = QCoreApplication::translate("TcpWidgetPrivate", " >> Network Recv: ");
            displayTextEdit->setTextColor(QColor("dodgerblue"));
            break;
        case SuccessInfo:
            display = QCoreApplication::translate("TcpWidgetPrivate", " >> Prompt Message: ");
            displayTextEdit->setTextColor(Qt::green);
            break;
        case ErrorInfo:
            display = QCoreApplication::translate("TcpWidgetPrivate", " >> Prompt Message: ");
            displayTextEdit->setTextColor(Qt::red);
            break;
        default: return;
        }
        displayTextEdit->append(
            QCoreApplication::translate("TcpWidgetPrivate", "Time [%1] %2 %3")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"),
                     display,
                     message));
    }

    void loadSetting() const
    {
        auto *setting = ExtensionSystem::PluginManager::settings();
        if (setting == nullptr) {
            return;
        }
        setting->beginGroup("tcp_config");
        modelBox->setCurrentIndex(setting->value("CommunicationMode").toInt());
        serverIPEdit->setText(setting->value("ClientIP", "127.0.0.1").toString());
        portEdit->setText(setting->value("Port", "80").toString());

        hexBox->setChecked(setting->value("Hex").toBool());
        autoSendTimeBox->setValue(setting->value("SendTime", 1000).toInt());
        autoConnectTimeBox->setValue(setting->value("ConnectTime", 3000).toInt());
        sendTextEdit->setText(setting->value("SendData").toString());
        setting->endGroup();
    }

    void saveSetting() const
    {
        auto *setting = ExtensionSystem::PluginManager::settings();
        if (setting == nullptr) {
            return;
        }
        setting->beginGroup("tcp_config");
        setting->setValue("CommunicationMode", modelBox->currentIndex());
        setting->setValue("ClientIP", serverIPEdit->text());
        setting->setValue("Port", portEdit->text());

        setting->setValue("Hex", hexBox->isChecked());
        setting->setValue("SendTime", autoSendTimeBox->value());
        setting->setValue("ConnectTime", autoConnectTimeBox->value());
        setting->setValue("SendData", sendTextEdit->toPlainText().toUtf8());
        setting->endGroup();
    }

    TcpWidget *q_ptr;

    QTextEdit *displayTextEdit;
    QTextEdit *sendTextEdit;
    QPushButton *sendButton;

    QComboBox *modelBox;
    QLabel *ipLabel;
    QComboBox *localIPBox;
    QLineEdit *serverIPEdit;
    QLabel *portLabel;
    QLineEdit *portEdit;
    QPushButton *listenOrConnectButton;

    QCheckBox *hexBox;
    QCheckBox *autoSendBox;
    QSpinBox *autoSendTimeBox;
    QComboBox *allConnectBox;
    QCheckBox *autoConnectBox;
    QSpinBox *autoConnectTimeBox;

    QPushButton *sendConutButton;
    QPushButton *recvConutButton;
    QPushButton *saveButton;
    QPushButton *clearButton;

    QWidget *setWidget;

    QScopedPointer<TcpClient> tcpClientPtr;
    QScopedPointer<TcpServer> tcpServerPtr;

    QTimer sendTime;
    QTimer autoConnectTime;
    int sendCount = 0;
    int recvCount = 0;

private:
    [[nodiscard]] auto createDisplayWidget() const -> QWidget *
    {
        auto *box = new QGroupBox(QCoreApplication::translate("TcpWidgetPrivate", "Data Display"),
                                  q_ptr);
        auto *layout = new QHBoxLayout(box);
        layout->addWidget(displayTextEdit);
        return box;
    }

    [[nodiscard]] auto createSendWidget() const -> QWidget *
    {
        auto *box = new QGroupBox(QCoreApplication::translate("TcpWidgetPrivate", "Data Send"),
                                  q_ptr);
        auto *layout = new QHBoxLayout(box);
        layout->addWidget(sendTextEdit);
        layout->addWidget(sendButton);
        return box;
    }

    [[nodiscard]] auto createSettingsBox() const -> QWidget *
    {
        auto *setLayout = new QVBoxLayout(setWidget);
        setLayout->setContentsMargins(0, 0, 0, 0);
        setLayout->addWidget(
            new QLabel(QCoreApplication::translate("TcpWidgetPrivate", "Mode: "), q_ptr));
        setLayout->addWidget(modelBox);
        setLayout->addWidget(ipLabel);
        setLayout->addWidget(localIPBox);
        setLayout->addWidget(serverIPEdit);
        setLayout->addWidget(portLabel);
        setLayout->addWidget(portEdit);
        setLayout->addWidget(listenOrConnectButton);

        auto *box = new QGroupBox(QCoreApplication::translate("TcpWidgetPrivate", "Settings"),
                                  q_ptr);
        auto *layout = new QVBoxLayout(box);
        layout->addWidget(setWidget);
        layout->addWidget(hexBox);
        layout->addWidget(autoSendBox);
        layout->addWidget(autoSendTimeBox);
        layout->addWidget(allConnectBox);
        layout->addWidget(autoConnectBox);
        layout->addWidget(autoConnectTimeBox);
        layout->addStretch();
        layout->addWidget(sendConutButton);
        layout->addWidget(recvConutButton);
        layout->addWidget(saveButton);
        layout->addWidget(clearButton);
        return box;
    }
};

TcpWidget::TcpWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new TcpWidgetPrivate(this))
{
    d_ptr->setupUI();
    d_ptr->initWindow();
    buildConnect();
    Utils::setMacComboBoxStyle(this);
    d_ptr->loadSetting();
    QMetaObject::invokeMethod(
        this,
        [this] {
            onModelChange(d_ptr->modelBox->currentText());
            d_ptr->displayTextEdit->clear();
        },
        Qt::QueuedConnection);
}

TcpWidget::~TcpWidget()
{
    d_ptr->saveSetting();
}

void TcpWidget::onModelChange(const QString &text)
{
    if (text == tr("TcpServer")) {
        d_ptr->ipLabel->setText(tr("Local IP List: "));
        d_ptr->localIPBox->show();
        d_ptr->serverIPEdit->hide();
        d_ptr->portLabel->setText(tr("Local Port: "));
        d_ptr->listenOrConnectButton->setText(tr("Listen"));
        d_ptr->allConnectBox->show();
        d_ptr->autoConnectBox->hide();
        d_ptr->autoConnectTimeBox->hide();
        onServerOnline(false);
    } else if (text == tr("TcpClient")) {
        d_ptr->ipLabel->setText(tr("Server IP: "));
        d_ptr->localIPBox->hide();
        d_ptr->serverIPEdit->show();
        d_ptr->listenOrConnectButton->setText(tr("Connect"));
        d_ptr->portLabel->setText(tr("Server Port: "));
        d_ptr->allConnectBox->hide();
        d_ptr->autoConnectBox->show();
        d_ptr->autoConnectTimeBox->show();
        onClientStateChanged(tr("The socket is not connected."), false);
    }
    d_ptr->clearCount();
    d_ptr->displayTextEdit->textCursor().removeSelectedText();
}

void TcpWidget::onListenOrConnect(bool state)
{
    d_ptr->listenOrConnectButton->setChecked(!state);

    onServerOnline(false);
    d_ptr->tcpServerPtr.reset();
    d_ptr->tcpClientPtr.reset();
    if (d_ptr->modelBox->currentText() == tr("TcpServer")) {
        if (state) {
            resetTcpServer();
        } else {
            for (int i = 1; i < d_ptr->allConnectBox->count(); i++) {
                onServerDisconnectClient(d_ptr->allConnectBox->itemText(i));
                d_ptr->allConnectBox->removeItem(i);
            }
        }
    } else if (d_ptr->modelBox->currentText() == tr("TcpClient")) {
        if (state) {
            resetTcpClient();
        }
    }
}

void TcpWidget::onSendData()
{
    auto text = d_ptr->sendTextEdit->toPlainText();
    if (text.isEmpty()) {
        return;
    }

    QByteArray bytes;
    if (d_ptr->hexBox->isChecked()) {
        bytes = QByteArray::fromHex(text.toLocal8Bit()).toUpper();
        text = formatHex(bytes);
    } else {
        bytes = text.toUtf8();
    }

    if (d_ptr->tcpServerPtr.isNull()) {
        d_ptr->tcpClientPtr->write(bytes);
        d_ptr->appendDisplay(Send, text);
        d_ptr->sendCount += bytes.size();
        d_ptr->setSendCount();
        return;
    }
    if (d_ptr->allConnectBox->count() == 1) {
        auto error = tr("No client is currently online, please stop sending invalid!");
        d_ptr->appendDisplay(ErrorInfo, error);
        return;
    }
    auto clientInfo = d_ptr->allConnectBox->currentText();
    if (clientInfo == tr("Connect All")) {
        d_ptr->appendDisplay(Send, tr("Send To All Online Clients: %1.").arg(text));
        clientInfo.clear();
    } else {
        d_ptr->appendDisplay(Send, tr("Send To Clients [%1] : %2.").arg(clientInfo, text));
    }
    d_ptr->tcpServerPtr->sendMessage(bytes, clientInfo);
    d_ptr->sendCount += text.size();
    d_ptr->setSendCount();
}

void TcpWidget::onServerOnline(bool state)
{
    d_ptr->modelBox->setEnabled(!state);
    d_ptr->localIPBox->setEnabled(!state);
    d_ptr->portEdit->setEnabled(!state);
    d_ptr->listenOrConnectButton->setChecked(state);
    d_ptr->listenOrConnectButton->setText(state ? tr("Stop Listen") : tr("Listen"));
    if (!state) {
        d_ptr->autoSendBox->setChecked(state);
        d_ptr->sendTime.stop();
    }
    d_ptr->autoSendBox->setEnabled(state);
    d_ptr->sendButton->setEnabled(state);

    if (state) {
        d_ptr->appendDisplay(SuccessInfo, tr("Server Online!"));
    } else {
        d_ptr->appendDisplay(ErrorInfo, tr("Server Offline!"));
    }
}

void TcpWidget::onServerNewClient(const QString &clientInfo)
{
    d_ptr->allConnectBox->addItem(clientInfo);
    auto str = clientInfo + tr(" Online.");
    d_ptr->appendDisplay(SuccessInfo, str);
}

void TcpWidget::onServerDisconnectClient(const QString &clientInfo)
{
    d_ptr->allConnectBox->removeItem(d_ptr->allConnectBox->findText(clientInfo));
    auto str = clientInfo + tr(" Offline.");
    d_ptr->appendDisplay(ErrorInfo, str);
}

void TcpWidget::onServerRecvMessage(const QString &clientInfo, const QByteArray &bytes)
{
    if (bytes.isEmpty()) {
        return;
    }
    d_ptr->recvCount += bytes.size();
    d_ptr->setRecvCount();
    auto str = clientInfo;
    if (d_ptr->hexBox->isChecked()) {
        str += formatHex(bytes);
    } else {
        str += QString::fromUtf8(bytes);
    }
    d_ptr->appendDisplay(Recv, str);
}

void TcpWidget::onClientStateChanged(const QString &text, bool onLine)
{
    d_ptr->modelBox->setEnabled(!onLine);
    d_ptr->serverIPEdit->setEnabled(!onLine);
    d_ptr->portEdit->setEnabled(!onLine);
    d_ptr->listenOrConnectButton->setChecked(onLine);
    d_ptr->listenOrConnectButton->setText(onLine ? tr("Disconnect") : tr("Connect"));
    if (!onLine) {
        d_ptr->autoSendBox->setChecked(onLine);
        d_ptr->sendTime.stop();
    }
    d_ptr->autoSendBox->setEnabled(onLine);
    d_ptr->sendButton->setEnabled(onLine);

    //    if (!onLine && !d_ptr->autoConnectBox->isChecked()) {
    //        d_ptr->tcpClientPtr.reset();
    //    }
    if (onLine) {
        d_ptr->appendDisplay(SuccessInfo, text);
    } else {
        d_ptr->appendDisplay(ErrorInfo, text);
    }
}

void TcpWidget::onClientRecvMessage(const QByteArray &bytes)
{
    if (bytes.isEmpty()) {
        return;
    }
    d_ptr->recvCount += bytes.size();
    d_ptr->setRecvCount();
    QString str;
    if (d_ptr->hexBox->isChecked()) {
        str = formatHex(bytes);
    } else {
        str = bytes;
    }
    d_ptr->appendDisplay(Recv, str);
}

void TcpWidget::onAutoReconnectStartOrStop(bool state)
{
    d_ptr->setWidget->setEnabled(!state);
    if (state) {
        resetTcpClient();
        d_ptr->autoConnectTime.start(d_ptr->autoConnectTimeBox->value());
    } else {
        d_ptr->autoConnectTime.stop();
        if (!d_ptr->listenOrConnectButton->isChecked()) {
            d_ptr->tcpServerPtr.reset();
            d_ptr->tcpClientPtr.reset();
        }
    }
}

void TcpWidget::onAutoConnect()
{
    if (!d_ptr->tcpClientPtr.isNull() && d_ptr->tcpClientPtr->isConnected()) {
        return;
    }

    resetTcpClient();
}

void TcpWidget::onAutoSend(bool state)
{
    d_ptr->autoSendTimeBox->setEnabled(!state);
    if (state) {
        d_ptr->sendTime.start(d_ptr->autoSendTimeBox->value());
    } else {
        d_ptr->sendTime.stop();
    }
}

void TcpWidget::onSave()
{
    auto text = d_ptr->displayTextEdit->toPlainText();
    if (text.isEmpty()) {
        return;
    }
    const auto openPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)
                              .value(0, QDir::homePath());
    const auto path
        = QFileDialog::getSaveFileName(this,
                                       tr("Open File"),
                                       QString("%1/%2").arg(openPath,
                                                            QDateTime::currentDateTime().toString(
                                                                "yyyy-MM-dd-HH-mm-ss")),
                                       tr("Text Files(*.txt)"));
    if (path.isEmpty()) {
        d_ptr->appendDisplay(ErrorInfo, tr("No file saved."));
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        GUI::MessBox::Warning(this,
                              tr("Write File: Can't open file:\n %1 !").arg(path),
                              GUI::MessBox::CloseButton);
        return;
    }
    file.write(text.toUtf8());
    file.flush();
    file.close();
    d_ptr->appendDisplay(SuccessInfo, tr("The file was saved successfully."));
}

void TcpWidget::onAppendError(const QString &error)
{
    d_ptr->appendDisplay(ErrorInfo, error);
}

void TcpWidget::buildConnect()
{
    connect(d_ptr->modelBox, &QComboBox::currentTextChanged, this, &TcpWidget::onModelChange);
    connect(d_ptr->listenOrConnectButton,
            &QPushButton::clicked,
            this,
            &TcpWidget::onListenOrConnect);

    auto *sendShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this);
    connect(sendShortcut, &QShortcut::activated, this, &TcpWidget::onSendData);
    connect(d_ptr->sendButton, &QPushButton::clicked, this, &TcpWidget::onSendData);

    connect(d_ptr->autoSendBox, &QCheckBox::clicked, this, &TcpWidget::onAutoSend);
    connect(&d_ptr->sendTime, &QTimer::timeout, this, &TcpWidget::onSendData);

    connect(d_ptr->autoConnectBox,
            &QCheckBox::clicked,
            this,
            &TcpWidget::onAutoReconnectStartOrStop);
    connect(&d_ptr->autoConnectTime, &QTimer::timeout, this, &TcpWidget::onAutoConnect);

    connect(d_ptr->sendConutButton, &QPushButton::clicked, this, [this] {
        d_ptr->sendCount = 0;
        d_ptr->setSendCount();
    });
    connect(d_ptr->recvConutButton, &QPushButton::clicked, this, [this] {
        d_ptr->recvCount = 0;
        d_ptr->setRecvCount();
    });
    connect(d_ptr->saveButton, &QPushButton::clicked, this, &TcpWidget::onSave);
    connect(d_ptr->clearButton, &QPushButton::clicked, d_ptr->displayTextEdit, &QTextEdit::clear);
}

void TcpWidget::resetTcpServer()
{
    auto port = d_ptr->portEdit->text();
    if (port.isEmpty()) {
        GUI::MessBox::Warning(this, tr("Please enter the port number!"), GUI::MessBox::CloseButton);
        d_ptr->portEdit->setFocus();
        return;
    }
    d_ptr->tcpServerPtr.reset(new TcpServer);
    connect(d_ptr->tcpServerPtr.data(), &TcpServer::errorMessage, this, &TcpWidget::onAppendError);
    connect(d_ptr->tcpServerPtr.data(),
            &TcpServer::newClientInfo,
            this,
            &TcpWidget::onServerNewClient);
    connect(d_ptr->tcpServerPtr.data(),
            &TcpServer::disconnectClientInfo,
            this,
            &TcpWidget::onServerDisconnectClient);
    connect(d_ptr->tcpServerPtr.data(),
            &TcpServer::clientMessage,
            this,
            &TcpWidget::onServerRecvMessage);
    auto ok = d_ptr->tcpServerPtr->listen(QHostAddress(d_ptr->localIPBox->currentText()),
                                          static_cast<quint16>(port.toUInt()));
    onServerOnline(ok);
}

void TcpWidget::resetTcpClient()
{
    auto port = d_ptr->portEdit->text();
    if (port.isEmpty()) {
        GUI::MessBox::Warning(this, tr("Please enter the port number!"), GUI::MessBox::CloseButton);
        d_ptr->portEdit->setFocus();
        d_ptr->autoConnectBox->setChecked(false);
        onAutoReconnectStartOrStop(false);
        return;
    }
    auto ip = d_ptr->serverIPEdit->text().trimmed();
    if (ip.isEmpty()) {
        GUI::MessBox::Warning(this, tr("Please enter the ip address!"), GUI::MessBox::CloseButton);
        d_ptr->serverIPEdit->setFocus();
        d_ptr->autoConnectBox->setChecked(false);
        onAutoReconnectStartOrStop(false);
        return;
    }
    d_ptr->tcpClientPtr.reset(new TcpClient(ip, static_cast<quint16>(port.toUInt())));
    connect(d_ptr->tcpClientPtr.data(), &TcpClient::errorMessage, this, &TcpWidget::onAppendError);
    connect(d_ptr->tcpClientPtr.data(),
            &TcpClient::serverMessage,
            this,
            &TcpWidget::onClientRecvMessage);
    connect(d_ptr->tcpClientPtr.data(),
            &TcpClient::socketStateChanged,
            this,
            &TcpWidget::onClientStateChanged);
    d_ptr->tcpClientPtr->connectToServer();
}

} // namespace Plugin
