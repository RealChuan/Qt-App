#include "appearancepage.hpp"

#include <utils/singletonmanager.hpp>
#include <utils/theme.hpp>
#include <utils/utils.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QSignalBlocker>
#include <QToolButton>
#include <QVBoxLayout>

namespace Core {

namespace {

// 卡片 = 标题 + 内容；页面里所有分组都用同一个结构，视觉上才是一致的。
struct Card
{
    QFrame *widget = nullptr;
    QVBoxLayout *layout = nullptr;
    QLabel *title = nullptr;
};

constexpr int kFieldWidth = 88;
constexpr int kCardWidth = 620;

auto createCard(QWidget *parent) -> Card
{
    Card card;
    card.widget = new QFrame(parent);
    card.widget->setObjectName("Card");

    card.layout = new QVBoxLayout(card.widget);
    card.layout->setContentsMargins(20, 16, 20, 18);
    card.layout->setSpacing(10);

    card.title = new QLabel(card.widget);
    card.title->setObjectName("CardTitle");
    card.layout->addWidget(card.title);
    return card;
}

// 一行“字段名 + 控件”
auto createRow(QWidget *parent, QLabel **labelOut) -> QHBoxLayout *
{
    auto *row = new QHBoxLayout;
    row->setContentsMargins({});
    row->setSpacing(16);

    auto *label = new QLabel(parent);
    label->setObjectName("FieldLabel");
    label->setFixedWidth(kFieldWidth);
    row->addWidget(label);

    *labelOut = label;
    return row;
}

} // namespace

class AppearancePage::AppearancePagePrivate
{
public:
    explicit AppearancePagePrivate(AppearancePage *q)
        : q_ptr(q)
    {}

    void setupUi();

    AppearancePage *q_ptr = nullptr;

    Card themeCard;
    Card languageCard;
    Card windowCard;
    QLabel *themeLabel = nullptr;
    QLabel *languageLabel = nullptr;
    QLabel *themeHint = nullptr;
    QLabel *languageHint = nullptr;
    QComboBox *theme = nullptr;
    QComboBox *language = nullptr;
    QCheckBox *mask = nullptr;
    QCheckBox *blur = nullptr;
    QToolButton *testButton = nullptr;
};

// 只建控件与布局；文案在 retranslateUi()，信号在 setupConnections()。
void AppearancePage::AppearancePagePrivate::setupUi()
{
    theme = new QComboBox(q_ptr);
    theme->setView(new QListView(theme));
    theme->addItem(QString(), static_cast<int>(Utils::ThemeMode::System));
    theme->addItem(QString(), static_cast<int>(Utils::ThemeMode::Light));
    theme->addItem(QString(), static_cast<int>(Utils::ThemeMode::Dark));
    theme->setCurrentIndex(static_cast<int>(Utils::theme()->mode()));

    language = new QComboBox(q_ptr);
    language->setView(new QListView(language));
    language->addItem(QString(), Utils::LanguageManager::Language::Chinese);
    language->addItem(QString(), Utils::LanguageManager::Language::English);
    language->setCurrentIndex(Utils::languageManager()->currentLanguage());

    mask = new QCheckBox(q_ptr);
    mask->setChecked(Utils::widgetManager()->showMask());
    blur = new QCheckBox(q_ptr);
    blur->setChecked(Utils::widgetManager()->blurBackground());

    testButton = new QToolButton(q_ptr);
    testButton->setCursor(Qt::PointingHandCursor);

    themeCard = createCard(q_ptr);
    auto *themeRow = createRow(q_ptr, &themeLabel);
    themeRow->addWidget(theme, 1);
    themeCard.layout->addLayout(themeRow);
    themeHint = new QLabel(q_ptr);
    themeHint->setObjectName("CardHint");
    themeHint->setWordWrap(true);
    themeCard.layout->addWidget(themeHint);

    languageCard = createCard(q_ptr);
    auto *languageRow = createRow(q_ptr, &languageLabel);
    languageRow->addWidget(language, 1);
    languageCard.layout->addLayout(languageRow);
    languageHint = new QLabel(q_ptr);
    languageHint->setObjectName("CardHint");
    languageHint->setWordWrap(true);
    languageCard.layout->addWidget(languageHint);

    windowCard = createCard(q_ptr);
    windowCard.layout->addWidget(mask);
    windowCard.layout->addWidget(blur);
    auto *testRow = new QHBoxLayout;
    testRow->setContentsMargins({});
    testRow->addStretch();
    testRow->addWidget(testButton);
    windowCard.layout->addSpacing(4);
    windowCard.layout->addLayout(testRow);

    // 卡片放进固定宽度的列：宽度确定后，卡片里的换行文本才能算出正确高度，
    // 否则 QLabel 的 heightForWidth 会按一行高度布局，文本被裁掉。
    auto *column = new QWidget(q_ptr);
    column->setFixedWidth(kCardWidth);
    auto *columnLayout = new QVBoxLayout(column);
    columnLayout->setContentsMargins({});
    columnLayout->setSpacing(16);
    columnLayout->addWidget(themeCard.widget);
    columnLayout->addWidget(languageCard.widget);
    columnLayout->addWidget(windowCard.widget);
    columnLayout->addStretch();

    auto *root = new QHBoxLayout(q_ptr);
    root->setContentsMargins({});
    root->setSpacing(0);
    root->addWidget(column);
    root->addStretch();
}

AppearancePage::AppearancePage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<AppearancePagePrivate>(this))
{
    d_ptr->setupUi();
    retranslateUi();
    setupConnections();
    Utils::setMacComboBoxStyle(this);
}

AppearancePage::~AppearancePage() = default;

void AppearancePage::retranslateUi()
{
    d_ptr->themeCard.title->setText(tr("Theme"));
    d_ptr->themeLabel->setText(tr("Color scheme"));
    d_ptr->theme->setItemText(0, tr("Follow system"));
    d_ptr->theme->setItemText(1, tr("Light"));
    d_ptr->theme->setItemText(2, tr("Dark"));
    d_ptr->themeHint->setText(tr("Follow system switches together with the Windows color scheme."));

    d_ptr->languageCard.title->setText(tr("Language"));
    d_ptr->languageLabel->setText(tr("Interface"));
    d_ptr->language->setItemText(0, tr("Chinese"));
    d_ptr->language->setItemText(1, tr("English"));
    d_ptr->languageHint->setText(tr("Applies immediately, no restart needed."));

    d_ptr->windowCard.title->setText(tr("Window"));
    d_ptr->mask->setText(tr("Dim the window while a dialog is open"));
    d_ptr->blur->setText(tr("Blur the window while a dialog is open"));
    d_ptr->testButton->setText(tr("Test dialog"));
}

void AppearancePage::setupConnections()
{
    connect(d_ptr->testButton, &QToolButton::clicked, this, &AppearancePage::showTestDialog);
    connect(d_ptr->theme, &QComboBox::currentIndexChanged, this, &AppearancePage::onThemeChanged);
    connect(Utils::theme(), &Utils::Theme::themeApplied, this, &AppearancePage::onThemeApplied);
    connect(
        d_ptr->language, &QComboBox::currentIndexChanged, this, &AppearancePage::onLanguageChanged);
    connect(d_ptr->mask, &QCheckBox::checkStateChanged, this, &AppearancePage::onMaskToggled);
    connect(d_ptr->blur, &QCheckBox::checkStateChanged, this, &AppearancePage::onBlurToggled);
}

void AppearancePage::showTestDialog()
{
    QDialog dialog(Utils::dialogParent());
    dialog.exec();
}

void AppearancePage::onThemeChanged()
{
    Utils::theme()->setMode(static_cast<Utils::ThemeMode>(d_ptr->theme->currentData().toInt()));
}

void AppearancePage::onThemeApplied(bool dark)
{
    Q_UNUSED(dark)
    // 主题可能来自标题栏按钮或系统，回填下拉框时不要再触发一次切换。
    const QSignalBlocker blocker(d_ptr->theme);
    d_ptr->theme->setCurrentIndex(static_cast<int>(Utils::theme()->mode()));
}

void AppearancePage::onLanguageChanged()
{
    Utils::languageManager()->loadLanguage(
        Utils::LanguageManager::Language(d_ptr->language->currentData().toInt()));
}

void AppearancePage::onMaskToggled(Qt::CheckState state)
{
    Utils::widgetManager()->setShowMask(state == Qt::Checked);
}

void AppearancePage::onBlurToggled(Qt::CheckState state)
{
    Utils::widgetManager()->setBlurBackground(state == Qt::Checked);
}

void AppearancePage::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        // 语言也可能从标题栏按钮切换，回填下拉框。
        const QSignalBlocker blocker(d_ptr->language);
        d_ptr->language->setCurrentIndex(Utils::languageManager()->currentLanguage());
    }
}

} // namespace Core
