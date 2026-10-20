#include "configpage.hpp"

#include <utils/singletonmanager.hpp>
#include <utils/utils.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QToolButton>

namespace Core {

class ConfigPage::ConfigPagePrivate
{
public:
    explicit ConfigPagePrivate(ConfigPage *q)
        : q_ptr(q)
    {}

    void setupUi();
    void setTr();

    ConfigPage *q_ptr = nullptr;
    QComboBox *language = nullptr;
    QCheckBox *mask = nullptr;
    QCheckBox *blur = nullptr;
};

void ConfigPage::ConfigPagePrivate::setupUi()
{
    language = new QComboBox(q_ptr);
    language->setView(new QListView(language));
    language->addItem(QString(), Utils::LanguageManager::Language::Chinese);
    language->addItem(QString(), Utils::LanguageManager::Language::English);
    language->setCurrentIndex(LANGUAGE_MANAGER->currentLanguage());

    mask = new QCheckBox(q_ptr);
    mask->setChecked(WIDGET_MANAGER->showMask());
    blur = new QCheckBox(q_ptr);
    blur->setChecked(WIDGET_MANAGER->blurBackground());

    auto *testButton = new QToolButton(q_ptr);
    testButton->setText(QObject::tr("Test dialog"));
    QObject::connect(testButton, &QToolButton::clicked, q_ptr, [] {
        QDialog dialog(Utils::dialogParent());
        dialog.exec();
    });

    auto *checks = new QHBoxLayout;
    checks->addWidget(mask);
    checks->addWidget(blur);

    auto *form = new QFormLayout(q_ptr);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    form->setLabelAlignment(Qt::AlignLeft);
    form->addRow(QObject::tr("Language (restart required):"), language);
    form->addRow(testButton, checks);

    QObject::connect(language, &QComboBox::currentIndexChanged, language, [this](int) {
        LANGUAGE_MANAGER->loadLanguage(
            Utils::LanguageManager::Language(language->currentData().toInt()));
    });
    QObject::connect(mask, &QCheckBox::checkStateChanged, q_ptr, [this](Qt::CheckState state) {
        WIDGET_MANAGER->setShowMask(state == Qt::Checked);
    });
    QObject::connect(blur, &QCheckBox::checkStateChanged, q_ptr, [this](Qt::CheckState state) {
        WIDGET_MANAGER->setBlurBackground(state == Qt::Checked);
    });

    Utils::setMacComboBoxStyle(q_ptr);
    setTr();
}

void ConfigPage::ConfigPagePrivate::setTr()
{
    language->setItemText(0, QObject::tr("Chinese"));
    language->setItemText(1, QObject::tr("English"));
    mask->setText(QObject::tr("Show mask when window is blocked"));
    blur->setText(QObject::tr("Blur background when window is blocked"));
}

ConfigPage::ConfigPage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<ConfigPagePrivate>(this))
{
    d_ptr->setupUi();
}

ConfigPage::~ConfigPage() = default;

void ConfigPage::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        d_ptr->setTr();
}

} // namespace Core
