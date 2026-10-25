#include "fortunepage.hpp"

#include "fortunecontent.hpp"
#include "fortuneform.hpp"
#include "fortunelunar.hpp"
#include "fortunereadingview.hpp"

#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace Fortune {

namespace {

constexpr int kHistoryLimit = 10;
constexpr int kHistoryShown = 5;

} // namespace

class Fortunepage::FortunepagePrivate
{
public:
    FortuneForm *form = nullptr;
    FortuneReadingView *view = nullptr;
    QStackedWidget *stack = nullptr;
    QLabel *introLabel = nullptr;
    QLabel *errorLabel = nullptr;
    QLabel *historyLabel = nullptr;
    QLabel *privacyLabel = nullptr;
    QLabel *footerLabel = nullptr;

    Profile profile;
    bool hasProfile = false;
    QList<HistoryEntry> history;
};

Fortunepage::Fortunepage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<FortunepagePrivate>())
{
    setupUi();
    retranslateUi();
    setupConnections();
    showProfileState();
}

Fortunepage::~Fortunepage() = default;

auto Fortunepage::subtitle() const -> QString
{
    return tr(
        "A simplified Plum Blossom reading built from your birth data and the moment you "
        "cast.");
}

void Fortunepage::setProfile(const Profile &profile)
{
    if (!isLunarDateSupported(profile.birthDate) || profile.birthHour < 0
        || profile.birthHour > 12) {
        showProfileState();
        return;
    }

    d_ptr->profile = profile;
    d_ptr->hasProfile = true;
    d_ptr->form->setProfile(profile);
    showIdleState();
}

void Fortunepage::setHistory(const QList<HistoryEntry> &history)
{
    d_ptr->history = history;
    updateHistoryLabel();
}

auto Fortunepage::hasProfile() const -> bool
{
    return d_ptr->hasProfile;
}

auto Fortunepage::history() const -> QList<HistoryEntry>
{
    return d_ptr->history;
}

void Fortunepage::castNow()
{
    if (!d_ptr->hasProfile)
        return;

    const auto reading = cast(d_ptr->profile, QDateTime::currentDateTime());
    if (!reading) {
        showError(reading.error());
        return;
    }

    showError({});

    const HistoryEntry entry{reading->castAt, reading->hexagram, reading->movingLine, reading->grade};
    d_ptr->history.prepend(entry);
    if (d_ptr->history.size() > kHistoryLimit)
        d_ptr->history.removeLast();

    emit fortuneCasted(entry);

    showReadingState(*reading);
    updateHistoryLabel();
}

void Fortunepage::focusProfileForm()
{
    if (!d_ptr->hasProfile) {
        showProfileState();
        d_ptr->form->focusName();
        return;
    }

    d_ptr->view->focusCastAgain();
}

void Fortunepage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();

    QWidget::changeEvent(event);
}

void Fortunepage::onProfileSubmitted(const Profile &profile)
{
    if (!isLunarDateSupported(profile.birthDate)) {
        showError(tr("Birth date must be between 1901-02-19 and 2100-12-31."));
        return;
    }

    d_ptr->profile = profile;
    d_ptr->hasProfile = true;
    showError({});
    emit profileSaved(profile);
    castNow();
}

void Fortunepage::onCastAgainClicked()
{
    castNow();
}

void Fortunepage::onResetClicked()
{
    d_ptr->view->finishReveal();
    d_ptr->profile = {};
    d_ptr->hasProfile = false;
    d_ptr->history.clear();
    d_ptr->form->clear();

    emit profileCleared();

    showProfileState();
    updateHistoryLabel();
    d_ptr->form->focusName();
}

void Fortunepage::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins({});
    root->setSpacing(0);

    auto *card = new QFrame(this);
    card->setObjectName(QStringLiteral("Card"));

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 24, 28, 24);
    cardLayout->setSpacing(10);

    d_ptr->introLabel = new QLabel(card);
    d_ptr->introLabel->setObjectName(QStringLiteral("CardHint"));
    d_ptr->introLabel->setWordWrap(true);

    auto *divider = new QFrame(card);
    divider->setObjectName(QStringLiteral("Divider"));
    divider->setFrameShape(QFrame::NoFrame);

    d_ptr->stack = new QStackedWidget(card);
    d_ptr->form = new FortuneForm(d_ptr->stack);
    d_ptr->view = new FortuneReadingView(d_ptr->stack);
    d_ptr->stack->addWidget(d_ptr->form);
    d_ptr->stack->addWidget(d_ptr->view);

    d_ptr->errorLabel = new QLabel(card);
    d_ptr->errorLabel->setObjectName(QStringLiteral("ErrorLabel"));
    d_ptr->errorLabel->setWordWrap(true);
    d_ptr->errorLabel->setVisible(false);

    d_ptr->historyLabel = new QLabel(card);
    d_ptr->historyLabel->setObjectName(QStringLiteral("HeroMeta"));
    d_ptr->historyLabel->setWordWrap(true);

    d_ptr->privacyLabel = new QLabel(card);
    d_ptr->privacyLabel->setObjectName(QStringLiteral("CardHint"));

    d_ptr->footerLabel = new QLabel(card);
    d_ptr->footerLabel->setObjectName(QStringLiteral("Placeholder"));
    d_ptr->footerLabel->setWordWrap(true);

    cardLayout->addWidget(d_ptr->introLabel);
    cardLayout->addWidget(divider);
    cardLayout->addWidget(d_ptr->stack, 1);
    cardLayout->addWidget(d_ptr->errorLabel);
    cardLayout->addWidget(d_ptr->historyLabel);
    cardLayout->addWidget(d_ptr->privacyLabel);
    cardLayout->addWidget(d_ptr->footerLabel);

    root->addWidget(card);
}

void Fortunepage::retranslateUi()
{
    d_ptr->introLabel->setText(
        tr("Your birth data stays on this device. Each cast uses the moment you press the "
           "button, so asking twice gives two different readings."));
    d_ptr->privacyLabel->setText(tr("Saved on this device only."));
    d_ptr->footerLabel->setText(
        tr("For entertainment only. A simplified Plum Blossom reading, not advice."));

    updateHistoryLabel();
}

void Fortunepage::setupConnections()
{
    connect(d_ptr->form, &FortuneForm::submitted, this, &Fortunepage::onProfileSubmitted);
    connect(
        d_ptr->view,
        &FortuneReadingView::castAgainRequested,
        this,
        &Fortunepage::onCastAgainClicked);
    connect(d_ptr->view, &FortuneReadingView::resetRequested, this, &Fortunepage::onResetClicked);
}

void Fortunepage::showProfileState()
{
    d_ptr->view->clear();
    d_ptr->stack->setCurrentWidget(d_ptr->form);
    d_ptr->historyLabel->setVisible(false);
    showError({});
}

void Fortunepage::showIdleState()
{
    const auto chart = birthChart(d_ptr->profile);
    if (!chart) {
        showProfileState();
        showError(chart.error());
        return;
    }

    d_ptr->view->showIdle(*chart, d_ptr->profile);
    d_ptr->stack->setCurrentWidget(d_ptr->view);
    d_ptr->historyLabel->setVisible(true);
}

void Fortunepage::showReadingState(const Reading &reading)
{
    d_ptr->view->showReading(reading);
    d_ptr->stack->setCurrentWidget(d_ptr->view);
    d_ptr->historyLabel->setVisible(true);
}

void Fortunepage::updateHistoryLabel()
{
    if (d_ptr->history.isEmpty()) {
        d_ptr->historyLabel->setText(tr("No casts yet."));
        return;
    }

    QStringList parts;
    const int count = std::min(int(d_ptr->history.size()), kHistoryShown);
    for (int i = 0; i < count; ++i) {
        const HistoryEntry &entry = d_ptr->history.at(i);
        parts.append(QStringLiteral("%1 · %2 · %3")
                         .arg(
                             entry.castAt.toString(QStringLiteral("MM-dd HH:mm")),
                             QString::number(entry.hexagram),
                             gradeName(entry.grade)));
    }

    d_ptr->historyLabel->setText(tr("Recent: %1").arg(parts.join(QStringLiteral("   "))));
}

void Fortunepage::showError(const QString &message)
{
    d_ptr->errorLabel->setText(message);
    d_ptr->errorLabel->setVisible(!message.isEmpty());
}

} // namespace Fortune
