#pragma once

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

namespace Particles {

// 自绘 + 帧驱动的最小示例：点击处炸开一团粒子，页面隐藏时停表。
class ParticlesWidget final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit ParticlesWidget(QWidget *parent = nullptr);
    ~ParticlesWidget() override;

    auto id() const -> QString override { return QStringLiteral("particlesplugin.page"); }
    auto title() const -> QString override { return tr("Particles"); }
    auto order() const -> int override { return 20; }
    auto widget() -> QWidget * override { return this; }

    void burst(const QPointF &center);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    class ParticlesWidgetPrivate;
    std::unique_ptr<ParticlesWidgetPrivate> d_ptr;
};

} // namespace Particles
