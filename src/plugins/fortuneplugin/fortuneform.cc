#include "fortuneform.hpp"

#include <QComboBox>
#include <QCoreApplication>
#include <QDateEdit>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <algorithm>
#include <array>

namespace Fortune {

namespace {

constexpr std::array<const char *, 12> kHourNames{
    QT_TRANSLATE_NOOP("Fortune::FortuneForm", "Zi"),
    QT_TRANSLATE_NOOP("Fortune::FortuneForm", "Chou"),
    QT_TRANSLATE_NOOP("Fortune::FortuneForm", "Yin"),
    QT_TRANSLATE_NOOP("Fortune::FortuneForm", "Mao"),
    QT_TRANSLATE_NOOP("Fortune::FortuneForm", "Chen"),
    QT_TRANSLATE_NOOP("Fortune::FortuneForm", "Si"),
    QT_TRANSLATE_NOOP("Fortune::FortuneForm", "Wu"),
    QT_TRANSLATE_NOOP("Fortune::FortuneForm", "Wei"),
    QT_TRANSLATE_NOOP("Fortune::FortuneForm", "Shen"),
    QT_TRANSLATE_NOOP("Fortune::FortuneForm", "You"),
    QT_TRANSLATE_NOOP("Fortune::FortuneForm", "Xu"),
    QT_TRANSLATE_NOOP("Fortune::FortuneForm", "Hai"),
};

} // namespace

class FortuneForm::FortuneFormPrivate
{
public:
    QLabel *nameLabel = nullptr;
    QLabel *birthLabel = nullptr;
    QLabel *hourLabel = nullptr;
    QLineEdit *nameEdit = nullptr;
    QDateEdit *birthEdit = nullptr;
    QComboBox *hourBox = nullptr;
    QPushButton *castButton = nullptr;
};

FortuneForm::FortuneForm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<FortuneFormPrivate>())
{
    setupUi();
    retranslateUi();
    setupConnections();
}

FortuneForm::~FortuneForm() = default;

void FortuneForm::setProfile(const Profile &profile)
{
    d_ptr->nameEdit->setText(profile.name);
    d_ptr->birthEdit->setDate(profile.birthDate);
    d_ptr->hourBox->setCurrentIndex(std::clamp(profile.birthHour, 0, 12));
}

void FortuneForm::clear()
{
    d_ptr->nameEdit->clear();
    d_ptr->birthEdit->setDate(QDate(2000, 1, 1));
    d_ptr->hourBox->setCurrentIndex(0);
}

auto FortuneForm::profile() const -> Profile
{
    return Profile{
        d_ptr->nameEdit->text().trimmed(), d_ptr->birthEdit->date(), d_ptr->hourBox->currentIndex()};
}

void FortuneForm::focusName()
{
    d_ptr->nameEdit->setFocus();
}

void FortuneForm::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();

    QWidget::changeEvent(event);
}

void FortuneForm::onSubmitClicked()
{
    emit submitted(profile());
}

void FortuneForm::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 8, 0, 0);
    root->setSpacing(10);

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(10);

    d_ptr->nameLabel = new QLabel(this);
    d_ptr->nameLabel->setObjectName(QStringLiteral("FieldLabel"));
    d_ptr->birthLabel = new QLabel(this);
    d_ptr->birthLabel->setObjectName(QStringLiteral("FieldLabel"));
    d_ptr->hourLabel = new QLabel(this);
    d_ptr->hourLabel->setObjectName(QStringLiteral("FieldLabel"));

    d_ptr->nameEdit = new QLineEdit(this);
    d_ptr->nameEdit->setMaxLength(48);

    d_ptr->birthEdit = new QDateEdit(this);
    d_ptr->birthEdit->setCalendarPopup(true);
    d_ptr->birthEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    d_ptr->birthEdit->setDateRange(QDate(1901, 2, 19), QDate(2100, 12, 31));
    d_ptr->birthEdit->setDate(QDate(2000, 1, 1));

    d_ptr->hourBox = new QComboBox(this);

    grid->addWidget(d_ptr->nameLabel, 0, 0);
    grid->addWidget(d_ptr->nameEdit, 0, 1);
    grid->addWidget(d_ptr->birthLabel, 1, 0);
    grid->addWidget(d_ptr->birthEdit, 1, 1);
    grid->addWidget(d_ptr->hourLabel, 2, 0);
    grid->addWidget(d_ptr->hourBox, 2, 1);
    grid->setColumnStretch(1, 1);

    d_ptr->castButton = new QPushButton(this);
    d_ptr->castButton->setObjectName(QStringLiteral("PrimaryButton"));
    d_ptr->castButton->setDefault(true);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->setContentsMargins(0, 4, 0, 0);
    buttonRow->addWidget(d_ptr->castButton);
    buttonRow->addStretch();

    root->addLayout(grid);
    root->addLayout(buttonRow);
    root->addStretch();
}

void FortuneForm::retranslateUi()
{
    d_ptr->nameLabel->setText(tr("Name"));
    d_ptr->birthLabel->setText(tr("Date of birth"));
    d_ptr->hourLabel->setText(tr("Hour of birth"));
    d_ptr->nameEdit->setPlaceholderText(tr("Leave empty to read as Seeker"));
    d_ptr->castButton->setText(tr("Cast"));

    const int currentHour = d_ptr->hourBox->currentIndex();
    {
        const QSignalBlocker blocker(d_ptr->hourBox);
        d_ptr->hourBox->clear();
        d_ptr->hourBox->addItem(tr("Unknown"));
        for (const char *name : kHourNames) {
            d_ptr->hourBox->addItem(QCoreApplication::translate("Fortune::FortuneForm", name));
        }
        d_ptr->hourBox->setCurrentIndex(std::clamp(currentHour, 0, 12));
    }
}

void FortuneForm::setupConnections()
{
    connect(d_ptr->castButton, &QPushButton::clicked, this, &FortuneForm::onSubmitClicked);
    connect(d_ptr->nameEdit, &QLineEdit::returnPressed, this, &FortuneForm::onSubmitClicked);
}

} // namespace Fortune
