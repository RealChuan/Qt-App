#include "configwidget.h"

#include <utils/languageconfig.hpp>
#include <utils/utils.h>

#include <QtWidgets>

class ConfigWidget::ConfigWidgetPrivate
{
public:
    ConfigWidgetPrivate(ConfigWidget *q)
        : q_ptr(q)
    {
        languageBox = new QComboBox(q_ptr);
        languageBox->addItem(tr("Chinese"), Utils::LanguageConfig::Language::Chinese);
        languageBox->addItem(tr("English"), Utils::LanguageConfig::Language::English);
    }

    void setupUI()
    {
        auto fromLayout = new QFormLayout(q_ptr);
        fromLayout->addRow(tr("Language(Requires Restart): "), languageBox);
    }

    void setData()
    {
        languageBox->setCurrentIndex(Utils::LanguageConfig::instance()->currentLanguage());
    }

    QWidget *q_ptr;
    QComboBox *languageBox;
};

ConfigWidget::ConfigWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ConfigWidgetPrivate(this))
{
    d_ptr->setupUI();
    buildConnect();
    Utils::setMacComboBoxStyle(this);
    d_ptr->setData();
}

ConfigWidget::~ConfigWidget() {}

void ConfigWidget::onReloadLanguage(int)
{
    Utils::LanguageConfig::instance()->loadLanguage(
        Utils::LanguageConfig::Language(d_ptr->languageBox->currentData().toInt()));
}

void ConfigWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
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
}

void ConfigWidget::setTr()
{
    d_ptr->languageBox->setItemText(0, tr("Chinese"));
    d_ptr->languageBox->setItemText(1, tr("English"));
}
