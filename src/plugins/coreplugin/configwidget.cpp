#include "configwidget.h"

#include <utils/guiutils.h>
#include <utils/singletonmanager.hpp>
#include <utils/utils.h>

#include <QtWidgets>

class ConfigWidget::ConfigWidgetPrivate
{
public:
    explicit ConfigWidgetPrivate(ConfigWidget *q)
        : q_ptr(q)
    {
        languageBox = new QComboBox(q_ptr);
        languageBox->setView(new QListView(languageBox));
        languageBox->addItem("", Utils::LanguageManager::Language::Chinese);
        languageBox->addItem("", Utils::LanguageManager::Language::English);

        maskCheckBox = new QCheckBox(q_ptr);
        blurCheckBox = new QCheckBox(q_ptr);
    }

    void setupUI()
    {
        auto *testButton = new QToolButton(q_ptr);
        testButton->setText(tr("Test dialog"));
        connect(testButton, &QToolButton::clicked, []() {
            QDialog dialog(Utils::dialogParent());
            dialog.exec();
        });

        auto *blockedLayout = new QHBoxLayout;
        blockedLayout->addWidget(maskCheckBox);
        blockedLayout->addWidget(blurCheckBox);

        fromLayout = new QFormLayout(q_ptr);
        fromLayout->addRow(ConfigWidget::tr("Language(Requires Restart): "), languageBox);
        fromLayout->addRow(testButton, blockedLayout);
    }

    void setData() const
    {
        languageBox->setCurrentIndex(LANGUAGE_MANAGER->currentLanguage());
        maskCheckBox->setChecked(WIDGET_MANAGER->showMask());
        blurCheckBox->setChecked(WIDGET_MANAGER->blurBackground());
    }

    QWidget *q_ptr;

    QComboBox *languageBox;
    QCheckBox *maskCheckBox;
    QCheckBox *blurCheckBox;
    QFormLayout *fromLayout;
};

ConfigWidget::ConfigWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ConfigWidgetPrivate(this))
{
    d_ptr->setupUI();
    buildConnect();
    Utils::setMacComboBoxStyle(this);
    d_ptr->setData();
    setTr();
}

ConfigWidget::~ConfigWidget() = default;

void ConfigWidget::onReloadLanguage(int /*unused*/)
{
    LANGUAGE_MANAGER->loadLanguage(
        Utils::LanguageManager::Language(d_ptr->languageBox->currentData().toInt()));
}

void ConfigWidget::onMaskCheckStateChanged(Qt::CheckState state)
{
    WIDGET_MANAGER->setShowMask(state == Qt::Checked);
}

void ConfigWidget::onBlurCheckStateChanged(Qt::CheckState state)
{
    WIDGET_MANAGER->setBlurBackground(state == Qt::Checked);
}

void ConfigWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange: setTr(); break;
    default: break;
    }
}

void ConfigWidget::buildConnect()
{
    connect(d_ptr->languageBox,
            &QComboBox::currentIndexChanged,
            this,
            &ConfigWidget::onReloadLanguage);

    connect(d_ptr->maskCheckBox,
            &QCheckBox::checkStateChanged,
            this,
            &ConfigWidget::onMaskCheckStateChanged);
    connect(d_ptr->blurCheckBox,
            &QCheckBox::checkStateChanged,
            this,
            &ConfigWidget::onBlurCheckStateChanged);
}

void ConfigWidget::setTr()
{
    d_ptr->languageBox->setItemText(0, tr("Chinese"));
    d_ptr->languageBox->setItemText(1, tr("English"));

    d_ptr->maskCheckBox->setText(tr("Show mask when window is blocked"));
    d_ptr->blurCheckBox->setText(tr("Blur background when window is blocked"));
}
