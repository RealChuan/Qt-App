#pragma once

#include "tetrisgame.hpp"

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

namespace Tetris {

// 一局的宿主：持有模型与 60 Hz 帧循环，输入连发（DAS/ARR）也在这里。
//
// 和贪吃蛇同一套规则：离开页面不停表，方块堆到顶自然结束；想安全离开先按暂停
// （P / Esc）。所以这里同样没有 hideEvent。
class TetrisPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit TetrisPage(QWidget *parent = nullptr);
    ~TetrisPage() override;

    [[nodiscard]] auto id() const -> QString override
    {
        return QStringLiteral("tetrisplugin.page");
    }
    [[nodiscard]] auto title() const -> QString override { return tr("Tetris"); }
    [[nodiscard]] auto subtitle() const -> QString override
    {
        return tr("Full-feel Tetris: SRS kicks, DAS/ARR, lock delay, hold and a 7-bag.");
    }
    [[nodiscard]] auto area() const -> Shell::PageArea override { return Shell::PageArea::Games; }
    // 排在贪吃蛇之后：档位之间留 100 的间隔，插件插队时不必改别人。
    [[nodiscard]] auto order() const -> int override { return Shell::PageOrder::kFirst + 100; }
    [[nodiscard]] auto widget() -> QWidget * override { return this; }

    void setBestScore(int score);
    void startGame();

signals:
    void gameFinished(int score);

private slots:
    void onFrame();
    void onPrimaryAction();
    void onPauseToggled();
    void onKeyPressed(int key);
    void onKeyReleased(int key);

protected:
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void setupConnections();
    void updateBestLabel();
    void updateLabels();

    class TetrisPagePrivate;
    std::unique_ptr<TetrisPagePrivate> d_ptr;
};

} // namespace Tetris
