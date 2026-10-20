#include "particleswidget.hpp"

#include <QElapsedTimer>
#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QTimer>

#include <algorithm>
#include <numbers>

namespace Particles {

namespace {
constexpr int kTimerIntervalMs = 16;
constexpr qreal kGravity = 520.0; // px / s^2
constexpr qreal kLifeLoss = 0.9;  // 每秒衰减
constexpr int kParticlesPerBurst = 70;
} // namespace

class ParticlesWidget::ParticlesWidgetPrivate
{
public:
    struct Particle
    {
        QPointF position;
        QPointF velocity;
        QColor color;
        qreal life = 1.0;
    };

    explicit ParticlesWidgetPrivate(ParticlesWidget *q)
        : q_ptr(q)
    {}

    void advance();
    void paint(QPainter &painter);

    ParticlesWidget *q_ptr = nullptr;
    QList<Particle> particles;
    QTimer timer;
    QElapsedTimer clock;
};

void ParticlesWidget::ParticlesWidgetPrivate::advance()
{
    const qreal dt = std::clamp(clock.restart() / 1000.0, 0.001, 0.1);

    for (Particle &particle : particles) {
        particle.velocity.setY(particle.velocity.y() + kGravity * dt);
        particle.position += particle.velocity * dt;
        particle.life -= kLifeLoss * dt;
    }

    particles.removeIf([](const Particle &particle) { return particle.life <= 0.0; });
    if (particles.isEmpty())
        timer.stop();

    q_ptr->update();
}

void ParticlesWidget::ParticlesWidgetPrivate::paint(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(q_ptr->rect(), QColor(24, 26, 32));

    for (const Particle &particle : std::as_const(particles)) {
        QColor color = particle.color;
        color.setAlphaF(std::clamp(particle.life, 0.0, 1.0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        const qreal radius = 1.5 + particle.life * 2.5;
        painter.drawEllipse(particle.position, radius, radius);
    }

    if (particles.isEmpty()) {
        painter.setPen(QColor(120, 126, 140));
        painter.drawText(q_ptr->rect(), Qt::AlignCenter, QObject::tr("Click anywhere to launch particles"));
    }
}

ParticlesWidget::ParticlesWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<ParticlesWidgetPrivate>(this))
{
    setMinimumSize(320, 240);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setCursor(Qt::CrossCursor);

    d_ptr->timer.setInterval(kTimerIntervalMs);
    connect(&d_ptr->timer, &QTimer::timeout, this, [this] { d_ptr->advance(); });
}

ParticlesWidget::~ParticlesWidget() = default;

void ParticlesWidget::burst(const QPointF &center)
{
    auto *random = QRandomGenerator::global();
    for (int i = 0; i < kParticlesPerBurst; ++i) {
        const qreal angle = random->generateDouble() * 2.0 * std::numbers::pi;
        const qreal speed = 60.0 + random->generateDouble() * 260.0;

        ParticlesWidgetPrivate::Particle particle;
        particle.position = center;
        particle.velocity = QPointF(std::cos(angle) * speed, std::sin(angle) * speed);
        particle.color = QColor::fromHsvF(random->generateDouble(), 0.85, 1.0);
        d_ptr->particles.append(particle);
    }

    if (!d_ptr->timer.isActive()) {
        d_ptr->clock.restart();
        d_ptr->timer.start();
    }
    update();
}

void ParticlesWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    d_ptr->paint(painter);
}

void ParticlesWidget::mousePressEvent(QMouseEvent *event)
{
    burst(event->position());
}

void ParticlesWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    d_ptr->clock.restart();
    if (!d_ptr->particles.isEmpty())
        d_ptr->timer.start();
}

void ParticlesWidget::hideEvent(QHideEvent *event)
{
    d_ptr->timer.stop();
    QWidget::hideEvent(event);
}

} // namespace Particles
