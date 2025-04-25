#include "hashwidget.hpp"
#include "cpubenchthread.hpp"
#include "hashthread.hpp"

#include <gui/messagebox.h>

#include <QtWidgets>

namespace Plugin {

static QComboBox *createHashComboBox(QWidget *parent)
{
    auto *comboBox = new QComboBox(parent);
    auto metaEnums = QMetaEnum::fromType<QCryptographicHash::Algorithm>();
    for (int i = 0; i < metaEnums.keyCount(); ++i) {
        auto value = metaEnums.value(i);
        if (value == QCryptographicHash::NumAlgorithms) {
            continue;
        }
        comboBox->addItem(metaEnums.key(i), value);
        if (value == QCryptographicHash::Md5) {
            comboBox->setCurrentText(metaEnums.key(i));
        }
    }
    return comboBox;
}

class HashWidget::HashWidgetPrivate
{
public:
    explicit HashWidgetPrivate(HashWidget *q)
        : q_ptr(q)
    {
        testHashComboBox = createHashComboBox(q_ptr);
        testIterationsSpinBox = new QSpinBox(q_ptr);
        auto sizePolicy = testIterationsSpinBox->sizePolicy();
        sizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
        testIterationsSpinBox->setSizePolicy(sizePolicy);
        testIterationsSpinBox->setRange(1, INT_MAX);
        testIterationsSpinBox->setValue(10);
        testDurationSpinBox = new QSpinBox(q_ptr);
        testDurationSpinBox->setSizePolicy(sizePolicy);
        testDurationSpinBox->setRange(1, INT_MAX);
        testDurationSpinBox->setValue(1000);
        testDataSizeSpinBox = new QSpinBox(q_ptr);
        testDataSizeSpinBox->setSizePolicy(sizePolicy);
        testDataSizeSpinBox->setRange(1024 * 1024, INT_MAX);
        testResultLabel = new QLabel(HashWidget::tr("Performance:--"), q_ptr);
        testButton = new QPushButton(HashWidget::tr("Test"), q_ptr);
        testButton->setToolTip(HashWidget::tr("Test hash speed."));
        testButton->setObjectName("BlueButton");

        cpuBenchThread = new CpuBenchThread(q_ptr);

        hashComboBox = createHashComboBox(q_ptr);
        selectFileButton = new QPushButton(HashWidget::tr("Select File"), q_ptr);
        selectFileButton->setToolTip(HashWidget::tr("Select file to calculate hash."));
        selectFileButton->setObjectName("BlueButton");
        calculateButton = new QPushButton(HashWidget::tr("Calculate"), q_ptr);
        calculateButton->setToolTip(HashWidget::tr("Calculate hash."));
        calculateButton->setObjectName("BlueButton");
        inputEdit = new QTextEdit(q_ptr);
        outputEdit = new QTextEdit(q_ptr);

        hashThread = new HashThread(q_ptr);
    }

    void setupUI() const
    {
        auto *buttonLayout = new QHBoxLayout;
        buttonLayout->setSpacing(20);
        buttonLayout->addWidget(selectFileButton);
        buttonLayout->addWidget(hashComboBox);
        buttonLayout->addWidget(calculateButton);

        auto *descriptionLabel = new QLabel(
            HashWidget::tr("If Input String is file path and file exists, calculate hash of file. "
                           "Otherwise calculate hash of Input String."),
            q_ptr);
        descriptionLabel->setWordWrap(true);

        auto *layout = new QVBoxLayout(q_ptr);
        layout->setSpacing(10);
        layout->addWidget(createHashTestGroup());
        layout->addWidget(descriptionLabel);
        layout->addWidget(new QLabel(HashWidget::tr("Input:"), q_ptr));
        layout->addWidget(inputEdit);
        layout->addLayout(buttonLayout);
        layout->addWidget(new QLabel(HashWidget::tr("Output:"), q_ptr));
        layout->addWidget(outputEdit);
    }

    QGroupBox *createHashTestGroup() const
    {
        auto *layout1 = new QHBoxLayout;
        layout1->setSpacing(20);
        layout1->addWidget(new QLabel(HashWidget::tr("Iterations:"), q_ptr));
        layout1->addWidget(testIterationsSpinBox);
        layout1->addWidget(new QLabel(HashWidget::tr("Duration (ms):"), q_ptr));
        layout1->addWidget(testDurationSpinBox);
        layout1->addWidget(new QLabel(HashWidget::tr("Data Size (bytes):"), q_ptr));
        layout1->addWidget(testDataSizeSpinBox);

        auto *layout2 = new QHBoxLayout;
        layout2->setSpacing(20);
        layout2->addWidget(new QLabel(HashWidget::tr("Algorithm:"), q_ptr));
        layout2->addWidget(testHashComboBox);
        layout2->addStretch();
        layout2->addWidget(testResultLabel);
        layout2->addWidget(testButton);

        auto *groupBox = new QGroupBox(HashWidget::tr("Hash Test"), q_ptr);
        auto *layout = new QVBoxLayout(groupBox);

        layout->addLayout(layout1);
        layout->addLayout(layout2);
        return groupBox;
    }

    HashWidget *q_ptr;

    QComboBox *testHashComboBox;
    QSpinBox *testIterationsSpinBox;
    QSpinBox *testDurationSpinBox;
    QSpinBox *testDataSizeSpinBox;
    QLabel *testResultLabel;
    QPushButton *testButton;
    CpuBenchThread *cpuBenchThread;

    QComboBox *hashComboBox;
    QPushButton *selectFileButton;
    QPushButton *calculateButton;
    QTextEdit *inputEdit;
    QTextEdit *outputEdit;
    HashThread *hashThread;
};

HashWidget::HashWidget(QWidget *parent)
    : QWidget{parent}
    , d_ptr(new HashWidgetPrivate(this))
{
    d_ptr->setupUI();
    buildConnect();
}

HashWidget::~HashWidget() = default;

void HashWidget::onTestHash()
{
    d_ptr->testButton->setEnabled(false);
    d_ptr->testButton->setText(tr("Testing..."));
    d_ptr->testResultLabel->setText(tr("Performance:--"));
    auto iterations = d_ptr->testIterationsSpinBox->value();
    auto duration = d_ptr->testDurationSpinBox->value();
    auto dataSize = d_ptr->testDataSizeSpinBox->value();
    auto algorithm = static_cast<QCryptographicHash::Algorithm>(
        d_ptr->testHashComboBox->currentData().toInt());
    d_ptr->cpuBenchThread->startBench({
        iterations,
        duration,
        dataSize,
        algorithm,
    });
}

void HashWidget::onTestBenchFinished(double result)
{
    d_ptr->testResultLabel->setText(tr("Performance: %1 MB/s").arg(result, 0, 'f', 2));
    d_ptr->testButton->setText(tr("Test"));
    d_ptr->testButton->setEnabled(true);
}

void HashWidget::onSelectFile()
{
    auto path = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)
                    .value(0, QDir::homePath());
    auto filePath = QFileDialog::getOpenFileName(this, tr("Select File"), path);
    d_ptr->inputEdit->setText(filePath);
}

void HashWidget::onCalculate()
{
    auto input = d_ptr->inputEdit->toPlainText();
    if (input.isEmpty()) {
        GUI::MessageBox::Warning(this, tr("Input is empty!"), GUI::MessageBox::Close);
        return;
    }
    auto algorithm = static_cast<QCryptographicHash::Algorithm>(
        d_ptr->hashComboBox->currentData().toInt());
    auto started = d_ptr->hashThread->startHash(input, algorithm);
    if (!started) {
        GUI::MessageBox::Warning(this, tr("Hash thread is running!"), GUI::MessageBox::Close);
        return;
    }
    d_ptr->calculateButton->setEnabled(false);
    d_ptr->calculateButton->setText(tr("Calculating..."));
}

void HashWidget::onHashFinished(const QString &result)
{
    d_ptr->outputEdit->setText(result);
    d_ptr->calculateButton->setEnabled(true);
    d_ptr->calculateButton->setText(tr("Calculate"));
}

void HashWidget::buildConnect()
{
    connect(d_ptr->testButton, &QPushButton::clicked, this, &HashWidget::onTestHash);
    connect(d_ptr->cpuBenchThread,
            &CpuBenchThread::benchFinished,
            this,
            &HashWidget::onTestBenchFinished);

    connect(d_ptr->selectFileButton, &QPushButton::clicked, this, &HashWidget::onSelectFile);
    connect(d_ptr->calculateButton, &QPushButton::clicked, this, &HashWidget::onCalculate);
    connect(d_ptr->hashThread, &HashThread::hashFinished, this, &HashWidget::onHashFinished);
}

} // namespace Plugin
