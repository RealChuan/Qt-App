#include "hashwidget.hpp"
#include "hashthread.hpp"

#include <gui/messbox.h>

#include <QtWidgets>

namespace Plugin {

class HashWidget::HashWidgetPrivate
{
public:
    explicit HashWidgetPrivate(HashWidget *q)
        : q_ptr(q)
    {
        hashComboBox = new QComboBox(q_ptr);
        auto metaEnums = QMetaEnum::fromType<QCryptographicHash::Algorithm>();
        for (int i = 0; i < metaEnums.keyCount(); ++i) {
            auto value = metaEnums.value(i);
            if (value == QCryptographicHash::NumAlgorithms) {
                continue;
            }
            hashComboBox->addItem(metaEnums.key(i), value);
            if (value == QCryptographicHash::Md5) {
                hashComboBox->setCurrentText(metaEnums.key(i));
            }
        }

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
        layout->addWidget(descriptionLabel);
        layout->addWidget(new QLabel(HashWidget::tr("Input:"), q_ptr));
        layout->addWidget(inputEdit);
        layout->addLayout(buttonLayout);
        layout->addWidget(new QLabel(HashWidget::tr("Output:"), q_ptr));
        layout->addWidget(outputEdit);
    }

    HashWidget *q_ptr;

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
        GUI::MessBox::Warning(this, tr("Input is empty!"), GUI::MessBox::CloseButton);
        return;
    }
    auto hash = d_ptr->hashComboBox->currentData().toInt();
    auto started = d_ptr->hashThread->startHash(input,
                                                static_cast<QCryptographicHash::Algorithm>(hash));
    if (!started) {
        GUI::MessBox::Warning(this, tr("Hash thread is running!"), GUI::MessBox::CloseButton);
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
    connect(d_ptr->selectFileButton, &QPushButton::clicked, this, &HashWidget::onSelectFile);
    connect(d_ptr->calculateButton, &QPushButton::clicked, this, &HashWidget::onCalculate);
    connect(d_ptr->hashThread, &HashThread::hashFinished, this, &HashWidget::onHashFinished);
}

} // namespace Plugin
