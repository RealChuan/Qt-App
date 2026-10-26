#pragma once

#include "fortuneengine.hpp"

#include <QWidget>

#include <memory>

class QHideEvent;

namespace Fortune {

// Hexagram board on the left, scrollable text on the right. Only the text
// column scrolls, so the drawing never moves with the reading.
class FortuneReadingView final : public QWidget
{
    Q_OBJECT

public:
    explicit FortuneReadingView(QWidget *parent = nullptr);
    ~FortuneReadingView() override;

    void showIdle(const BirthChart &chart, const Profile &profile);
    void showReading(const Reading &reading);
    void clear();
    void finishReveal();
    void focusCastAgain();

signals:
    void castAgainRequested();
    void editRequested();

protected:
    void changeEvent(QEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void onSkipClicked();
    void onCopyClicked();
    void onSaveImageClicked();
    void onRevealFinished();
    void onThemeApplied();

private:
    void setupUi();
    void retranslateUi();
    void setupConnections();
    void startReveal();
    void updateCastButtonText();

    class FortuneReadingViewPrivate;
    std::unique_ptr<FortuneReadingViewPrivate> d_ptr;
};

} // namespace Fortune
