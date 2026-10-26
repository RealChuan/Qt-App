#pragma once

#include "fortuneengine.hpp"

#include <shell/page.hpp>

#include <QList>
#include <QWidget>

#include <memory>

using namespace Qt::StringLiterals;

namespace Fortune {

class FortuneForm;
class FortuneReadingView;

class Fortunepage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit Fortunepage(QWidget *parent = nullptr);
    ~Fortunepage() override;

    [[nodiscard]] auto id() const -> QString override { return u"fortuneplugin.page"_s; }
    [[nodiscard]] auto title() const -> QString override { return tr("Cyber Fortune"); }
    [[nodiscard]] auto subtitle() const -> QString override;
    [[nodiscard]] auto area() const -> Shell::PageArea override
    {
        return Shell::PageArea::Workspace;
    }
    [[nodiscard]] auto order() const -> int override { return Shell::PageOrder::kFifth + 100; }
    [[nodiscard]] auto widget() -> QWidget * override { return this; }

    void setProfile(const Profile &profile);
    void setHistory(const QList<HistoryEntry> &history);

    [[nodiscard]] auto hasProfile() const -> bool;
    [[nodiscard]] auto history() const -> QList<HistoryEntry>;

    void castNow();
    void focusProfileForm();

signals:
    void profileSaved(const Profile &profile);
    void profileCleared();
    void fortuneCasted(const HistoryEntry &entry);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onProfileSubmitted(const Profile &profile);
    void onCastAgainClicked();
    void onEditClicked();
    void onBackClicked();
    void onDeleteClicked();

private:
    void setupUi();
    void retranslateUi();
    void setupConnections();
    void showProfileState();
    void showIdleState();
    void showReadingState(const Reading &reading);
    void updateHistoryLabel();
    void showError(const QString &message);

    class FortunepagePrivate;
    std::unique_ptr<FortunepagePrivate> d_ptr;
};

} // namespace Fortune
