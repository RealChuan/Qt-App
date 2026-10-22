#pragma once

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

namespace Particles {

// 自绘 + 帧驱动的最小示例：点击处炸开一团粒子，页面隐藏时停表。
class ParticlesPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit ParticlesPage(QWidget *parent = nullptr);
    ~ParticlesPage() override;

    [[nodiscard]] auto id() const -> QString override
    {
        return QStringLiteral("particlesplugin.page");
    }
    [[nodiscard]] auto title() const -> QString override { return tr("Particles"); }
    [[nodiscard]] auto subtitle() const -> QString override
    {
        return tr("Click anywhere on the canvas to launch a burst of particles.");
    }
    [[nodiscard]] auto order() const -> int override { return Shell::PageOrder::kSecond; }
    [[nodiscard]] auto widget() -> QWidget * override { return this; }

    void burst(const QPointF &center);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void onFrameTimeout();
    void retranslateUi();
    void setupConnections();

    class ParticlesPagePrivate;
    std::unique_ptr<ParticlesPagePrivate> d_ptr;
};

} // namespace Particles
