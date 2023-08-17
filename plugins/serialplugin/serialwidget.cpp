#include "serialwidget.h"
#include "serialport.h"
#include "serialsettings.hpp"

#include <extensionsystem/pluginmanager.h>
#include <gui/messbox.h>
#include <utils/utils.h>

#include <QSerialPortInfo>
#include <QtWidgets>

namespace Plugin {

template<typename T>
void initComboBox(QComboBox *comboBox)
{
    comboBox->clear();
    auto metaEnum = QMetaEnum::fromType<T>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        comboBox->addItem(metaEnum.key(i), metaEnum.value(i));
    }
}

void setComboxCurrentText(QComboBox *box, const QVariant &value)
{
    box->setCurrentIndex(box->findData(value));
}

auto formatHex(const QByteArray &msg) -> QString
{
    QString temp;
    auto hex = QString::fromLocal8Bit(msg.toHex().toUpper());
    for (int i = 0; i < hex.length(); i = i + 2) {
        temp += hex.mid(i, 2) + " "; //两个字符+空格（例子：7e ）
    }
    return temp;
}

struct WidgetSettings
{
    SerialSettings serialSettings;
    bool hex = false;
    int sendTime = 1000;
    QString sendData;
};

class SerialWidget::SerialWidgetPrivate
{
public:
    SerialWidgetPrivate(QWidget *q)
        : q_ptr(q)
    {
        displayTextEdit = new QTextEdit(q_ptr);
        displayTextEdit->document()->setMaximumBlockCount(1000);
        displayTextEdit->setReadOnly(true);

        sendTextEdit = new QTextEdit(q_ptr);
        sendButton = new QPushButton(QObject::tr("Send"), q_ptr);
        sendButton->setObjectName("BlueButton");
        sendButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

        searchSerialButton = new QPushButton(QObject::tr("Search Available Serial"), q_ptr);
        searchSerialButton->setObjectName("BlueButton");
        portNameBox = new QComboBox(q_ptr);
        baudRateBox = new QComboBox(q_ptr);
        dataBitsBox = new QComboBox(q_ptr);
        stopBitsBox = new QComboBox(q_ptr);
        parityBox = new QComboBox(q_ptr);
        flowControlBox = new QComboBox(q_ptr);
        openOrCloseButton = new QPushButton(q_ptr);
        openOrCloseButton->setObjectName("GrayButton");
        openOrCloseButton->setCheckable(true);

        hexBox = new QCheckBox(QObject::tr("Hex"), q_ptr);
        autoSendBox = new QCheckBox(QObject::tr("Auto Delivery"), q_ptr);
        autoSendTimeBox = new QSpinBox(q_ptr);
        autoSendTimeBox->setSuffix(QObject::tr(" ms"));
        autoSendTimeBox->setRange(0, 10000);
        autoSendTimeBox->setValue(1000);
        autoSendTimeBox->setSingleStep(50);

        sendConutButton = new QPushButton(QObject::tr("Send: 0 Bytes"), q_ptr);
        recvConutButton = new QPushButton(QObject::tr("Receive: 0 Bytes"), q_ptr);
        saveButton = new QPushButton(QObject::tr("Save Data"), q_ptr);
        clearButton = new QPushButton(QObject::tr("Clear Screen"), q_ptr);

        sendTimer = new QTimer(q_ptr);
    }

    void setupUI()
    {
        auto displayBox = createDisplayWidget();
        auto sendBox = createSendWidget();

        auto splitter1 = new QSplitter(Qt::Vertical, q_ptr);
        splitter1->addWidget(displayBox);
        splitter1->addWidget(sendBox);
        splitter1->setHandleWidth(0);
        splitter1->setSizes(QList<int>() << 400 << 1);

        auto settingBox = createSettingsBox();

        auto splitter2 = new QSplitter(Qt::Horizontal, q_ptr);
        splitter2->addWidget(splitter1);
        splitter2->addWidget(settingBox);
        splitter2->setHandleWidth(10);
        splitter2->setSizes(QList<int>() << 400 << 1);

        auto layout = new QHBoxLayout(q_ptr);
        layout->addWidget(splitter2);
    }

    void initWindow()
    {
        searchPort();
        auto baudList = QSerialPortInfo::standardBaudRates();
        for (const auto baudrate : baudList) {
            baudRateBox->addItem(QString::number(baudrate), baudrate);
        }

        initComboBox<QSerialPort::DataBits>(dataBitsBox);
        initComboBox<QSerialPort::Parity>(parityBox);
        initComboBox<QSerialPort::FlowControl>(flowControlBox);

        stopBitsBox->addItem("1", QSerialPort::OneStop);
#ifdef Q_OS_WIN
        stopBitsBox->addItem("1.5", QSerialPort::OneAndHalfStop);
#endif
        stopBitsBox->addItem("2", QSerialPort::TwoStop);

        QMetaObject::invokeMethod(
            q_ptr, [this] { searchPort(); }, Qt::QueuedConnection);
    }

    void searchPort()
    {
        portNameBox->clear();
        auto availablePorts = QSerialPortInfo::availablePorts();
        for (const auto &info : qAsConst(availablePorts)) {
            QSerialPort port;
            port.setPort(info);
            if (port.open(QIODevice::ReadWrite)) {
                portNameBox->addItem(port.portName());
                port.close();
            }
        }
    }

    void setWindowSettings()
    {
        auto serialSettings = &widgetSettings.serialSettings;
        setComboxCurrentText(baudRateBox, serialSettings->baudRate);
        setComboxCurrentText(dataBitsBox, serialSettings->dataBits);
        setComboxCurrentText(stopBitsBox, serialSettings->stopBits);
        setComboxCurrentText(parityBox, serialSettings->parity);
        setComboxCurrentText(flowControlBox, serialSettings->flowControl);

        hexBox->setChecked(widgetSettings.hex);
        autoSendTimeBox->setValue(widgetSettings.sendTime);
        sendTextEdit->setText(widgetSettings.sendData);
    }

    void setSerialSettings()
    {
        auto serialParam = &widgetSettings.serialSettings;
        serialParam->portName = portNameBox->currentText();
        serialParam->baudRate = QSerialPort::BaudRate(baudRateBox->currentData().toInt());
        serialParam->dataBits = QSerialPort::DataBits(dataBitsBox->currentData().toInt());
        serialParam->stopBits = QSerialPort::StopBits(stopBitsBox->currentData().toInt());
        serialParam->parity = QSerialPort::Parity(parityBox->currentData().toInt());
        serialParam->flowControl = QSerialPort::FlowControl(flowControlBox->currentData().toInt());
    }

    void appendDisplay(SerialWidget::MessageType type, const QString &message)
    {
        if (message.isEmpty()) {
            return;
        }
        QString display;
        switch (type) {
        case Send:
            display = tr(" >> Serial Send: ");
            displayTextEdit->setTextColor(Qt::black);
            break;
        case Recv:
            display = tr(" >> Serial Recv: ");
            displayTextEdit->setTextColor(QColor("dodgerblue"));
            break;
        case SuccessInfo:
            display = tr(" >> Prompt Message: ");
            displayTextEdit->setTextColor(Qt::green);
            break;
        case ErrorInfo:
            display = tr(" >> Prompt Message: ");
            displayTextEdit->setTextColor(Qt::red);
            break;
        default: return;
        }
        displayTextEdit->append(
            tr("Time [%1] %2 %3")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"),
                     display,
                     message));
    }

    void setSendCount() { sendConutButton->setText(tr("Send: %1 Bytes").arg(sendCount)); }

    void setRecvCount() { recvConutButton->setText(tr("Recv: %1 Bytes").arg(recvCount)); }

    void loadSetting()
    {
        auto setting = ExtensionSystem::PluginManager::settings();
        if (!setting) {
            return;
        }
        setting->beginGroup("serial_config");
        auto serialParam = &widgetSettings.serialSettings;
        serialParam->baudRate = QSerialPort::BaudRate(
            setting->value("BaudRate", serialParam->baudRate).toInt());
        serialParam->dataBits = QSerialPort::DataBits(
            setting->value("DataBits", serialParam->dataBits).toInt());
        serialParam->stopBits = QSerialPort::StopBits(
            setting->value("StopBits", serialParam->stopBits).toInt());
        serialParam->parity = QSerialPort::Parity(
            setting->value("Parity", serialParam->parity).toInt());
        serialParam->flowControl = QSerialPort::FlowControl(
            setting->value("FlowControl", serialParam->flowControl).toInt());

        widgetSettings.hex = setting->value("Hex", widgetSettings.hex).toBool();
        widgetSettings.sendTime = setting->value("SendTime", widgetSettings.sendTime).toInt();
        widgetSettings.sendData = setting->value("SendData", widgetSettings.sendData).toString();
        setting->endGroup();
    }

    void saveSetting()
    {
        auto setting = ExtensionSystem::PluginManager::settings();
        if (!setting) {
            return;
        }
        auto serialParam = &widgetSettings.serialSettings;
        setting->beginGroup("serial_config");
        setting->setValue("BaudRate", serialParam->baudRate);
        setting->setValue("DataBits", serialParam->dataBits);
        setting->setValue("StopBits", serialParam->stopBits);
        setting->setValue("Parity", serialParam->parity);
        setting->setValue("FlowControl", serialParam->flowControl);

        setting->setValue("Hex", hexBox->isChecked());
        setting->setValue("SendTime", autoSendTimeBox->value());
        setting->setValue("SendData", sendTextEdit->toPlainText().toUtf8());
        setting->endGroup();
    }

    QWidget *q_ptr;

    QTextEdit *displayTextEdit;
    QTextEdit *sendTextEdit;
    QPushButton *sendButton;

    QPushButton *searchSerialButton;
    QComboBox *portNameBox;
    QComboBox *baudRateBox;
    QComboBox *dataBitsBox;
    QComboBox *stopBitsBox;
    QComboBox *parityBox;
    QComboBox *flowControlBox;
    QPushButton *openOrCloseButton;

    QCheckBox *hexBox;
    QCheckBox *autoSendBox;
    QSpinBox *autoSendTimeBox;
    QPushButton *sendConutButton;
    QPushButton *recvConutButton;
    QPushButton *saveButton;
    QPushButton *clearButton;

    QScopedPointer<SerialPort> serialPortPtr;
    WidgetSettings widgetSettings;

    QTimer *sendTimer;
    int sendCount = 0;
    int recvCount = 0;

private:
    QWidget *createDisplayWidget()
    {
        auto box = new QGroupBox(QObject::tr("Data Display"), q_ptr);
        auto layout = new QHBoxLayout(box);
        layout->addWidget(displayTextEdit);
        return box;
    }

    QWidget *createSendWidget()
    {
        auto box = new QGroupBox(QObject::tr("Data Send"), q_ptr);
        auto layout = new QHBoxLayout(box);
        layout->addWidget(sendTextEdit);
        layout->addWidget(sendButton);
        return box;
    }

    QWidget *createSettingsBox()
    {
        auto formLayout = new QFormLayout;
        formLayout->setContentsMargins(0, 0, 0, 0);
        formLayout->addRow(QObject::tr("Port: "), portNameBox);
        formLayout->addRow(QObject::tr("Baud Rate: "), baudRateBox);
        formLayout->addRow(QObject::tr("Data Bits: "), dataBitsBox);
        formLayout->addRow(QObject::tr("Stop Bits: "), stopBitsBox);
        formLayout->addRow(QObject::tr("Parity: "), parityBox);
        formLayout->addRow(QObject::tr("Flow Control: "), flowControlBox);
#ifdef Q_OS_MACOS
        formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
        formLayout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
        formLayout->setLabelAlignment(Qt::AlignLeft);
#endif

        auto box = new QGroupBox(tr("Settings"), q_ptr);
        auto layout = new QVBoxLayout(box);
        layout->addWidget(searchSerialButton);
        layout->addLayout(formLayout);
        layout->addWidget(openOrCloseButton);
        layout->addWidget(hexBox);
        layout->addWidget(autoSendBox);
        layout->addWidget(autoSendTimeBox);
        layout->addStretch();
        layout->addWidget(sendConutButton);
        layout->addWidget(recvConutButton);
        layout->addWidget(saveButton);
        layout->addWidget(clearButton);
        return box;
    }
};

SerialWidget::SerialWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new SerialWidgetPrivate(this))
{
    d_ptr->setupUI();
    d_ptr->initWindow();
    d_ptr->loadSetting();
    d_ptr->setWindowSettings();
    buildConnect();
    Utils::setMacComboBoxStyle(this);
    onLine(false);
    d_ptr->displayTextEdit->clear();
}

SerialWidget::~SerialWidget()
{
    d_ptr->saveSetting();
}

void SerialWidget::onSendData()
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
        bytes = text.toLatin1();
    }

    if (d_ptr->serialPortPtr.isNull()) {
        return;
    }
    d_ptr->serialPortPtr->write(bytes);
    d_ptr->appendDisplay(Send, text);
    d_ptr->sendCount += bytes.size();
    d_ptr->setSendCount();
}

void SerialWidget::onParamChanged(const QString &)
{
    if (d_ptr->serialPortPtr.isNull()) {
        return;
    }
    d_ptr->serialPortPtr->close();
    d_ptr->serialPortPtr->openSerialPort(d_ptr->widgetSettings.serialSettings);
}

void SerialWidget::onOpenOrCloseSerial(bool state)
{
    d_ptr->openOrCloseButton->setChecked(!state);
    if (state) {
        d_ptr->setSerialSettings();
        d_ptr->serialPortPtr.reset(new SerialPort);
        connect(d_ptr->serialPortPtr.data(),
                &SerialPort::onLine,
                this,
                &SerialWidget::onLine,
                Qt::UniqueConnection);
        connect(d_ptr->serialPortPtr.data(),
                &SerialPort::errorMessage,
                this,
                &SerialWidget::onAppendError,
                Qt::UniqueConnection);
        connect(d_ptr->serialPortPtr.data(),
                &SerialPort::message,
                this,
                &SerialWidget::onSerialRecvMessage,
                Qt::UniqueConnection);
        d_ptr->serialPortPtr->openSerialPort(d_ptr->widgetSettings.serialSettings);
        return;
    }
    d_ptr->serialPortPtr.reset();
}

void SerialWidget::onLine(bool state)
{
    d_ptr->searchSerialButton->setEnabled(!state);
    d_ptr->openOrCloseButton->setChecked(state);
    d_ptr->openOrCloseButton->setText(state ? tr("Close") : tr("Open"));

    if (!state) {
        d_ptr->autoSendBox->setChecked(state);
        d_ptr->sendTimer->stop();
    }
    d_ptr->autoSendBox->setEnabled(state);
    d_ptr->sendButton->setEnabled(state);

    if (state) {
        d_ptr->appendDisplay(SuccessInfo, tr("Serial Open!"));
    } else {
        d_ptr->appendDisplay(ErrorInfo, tr("Serial Close!"));
    }
}

void SerialWidget::onAppendError(const QString &error)
{
    d_ptr->appendDisplay(ErrorInfo, error);
}

void SerialWidget::onSerialRecvMessage(const QByteArray &bytes)
{
    //qDebug() << "onSerialRecvMessage: " << bytes;
    if (bytes.isEmpty()) {
        return;
    }
    d_ptr->recvCount += bytes.size();
    d_ptr->setRecvCount();
    QString str;
    if (d_ptr->hexBox->isChecked()) {
        str = formatHex(bytes);
    } else {
        str = QString::fromLatin1(bytes);
    }
    d_ptr->appendDisplay(Recv, str);
}

void SerialWidget::onAutoSend(bool state)
{
    d_ptr->autoSendTimeBox->setEnabled(!state);
    if (state) {
        d_ptr->sendTimer->start(d_ptr->autoSendTimeBox->value());
    } else {
        d_ptr->sendTimer->stop();
    }
}

void SerialWidget::onSave()
{
    const auto data = d_ptr->displayTextEdit->toPlainText();
    if (data.isEmpty()) {
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
    QTextStream stream(&file);
    stream << data;
    file.close();
    d_ptr->appendDisplay(SuccessInfo, tr("The file was saved successfully."));
}

void SerialWidget::buildConnect()
{
    connect(d_ptr->searchSerialButton, &QPushButton::clicked, this, [this] { d_ptr->searchPort(); });

    connect(d_ptr->portNameBox, &QComboBox::currentTextChanged, this, &SerialWidget::onParamChanged);
    connect(d_ptr->baudRateBox, &QComboBox::currentTextChanged, this, &SerialWidget::onParamChanged);
    connect(d_ptr->dataBitsBox, &QComboBox::currentTextChanged, this, &SerialWidget::onParamChanged);
    connect(d_ptr->stopBitsBox, &QComboBox::currentTextChanged, this, &SerialWidget::onParamChanged);
    connect(d_ptr->parityBox, &QComboBox::currentTextChanged, this, &SerialWidget::onParamChanged);
    connect(d_ptr->flowControlBox,
            &QComboBox::currentTextChanged,
            this,
            &SerialWidget::onParamChanged);

    connect(d_ptr->openOrCloseButton,
            &QPushButton::clicked,
            this,
            &SerialWidget::onOpenOrCloseSerial);

    auto sendShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this);
    connect(sendShortcut, &QShortcut::activated, this, &SerialWidget::onSendData);
    connect(d_ptr->sendButton, &QPushButton::clicked, this, &SerialWidget::onSendData);

    connect(d_ptr->autoSendBox, &QCheckBox::clicked, this, &SerialWidget::onAutoSend);
    connect(d_ptr->sendTimer, &QTimer::timeout, this, &SerialWidget::onSendData);

    connect(d_ptr->sendConutButton, &QPushButton::clicked, this, [this] {
        d_ptr->sendCount = 0;
        d_ptr->setSendCount();
    });
    connect(d_ptr->recvConutButton, &QPushButton::clicked, this, [this] {
        d_ptr->recvCount = 0;
        d_ptr->setRecvCount();
    });
    connect(d_ptr->saveButton, &QPushButton::clicked, this, &SerialWidget::onSave);
    connect(d_ptr->clearButton, &QPushButton::clicked, d_ptr->displayTextEdit, &QTextEdit::clear);
}

} // namespace Plugin
