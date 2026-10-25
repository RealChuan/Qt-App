#pragma once

#include <QWidget>

#include <memory>

namespace Fortune {

// Six lines drawn bottom-up. revealProgress goes from 0 to 6 and is animated by
// the page, so the widget itself owns no timer.
class HexagramBoard final : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal revealProgress READ revealProgress WRITE setRevealProgress)

public:
    explicit HexagramBoard(QWidget *parent = nullptr);
    ~HexagramBoard() override;

    void setHexagram(int upper, int lower, int movingLine, bool isIdle);
    void clear();

    [[nodiscard]] auto revealProgress() const -> qreal;
    void setRevealProgress(qreal progress);

    [[nodiscard]] auto sizeHint() const -> QSize override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    class HexagramBoardPrivate;
    std::unique_ptr<HexagramBoardPrivate> d_ptr;
};

} // namespace Fortune
