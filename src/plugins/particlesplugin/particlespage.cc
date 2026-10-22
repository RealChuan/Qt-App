#include "particlespage.hpp"

#include <QElapsedTimer>
#include <QEvent>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QTimer>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace Particles {

namespace {

constexpr int kTimerIntervalMs = 16;
constexpr qreal kGravity = 520.0; // px / s^2
constexpr qreal kLifeLoss = 0.9;  // 每秒衰减
constexpr int kParticlesPerBurst = 64;
constexpr int kMaxParticles = 900;

// 卡片外观对齐 pages.css 的 #Card。
constexpr qreal kCardRadius = 12.0;
constexpr qreal kBorderWidth = 1.0;

// 粒子只围绕主题强调色做小幅色相偏移，避免一屏彩虹和蓝灰色的界面打架。
constexpr qreal kHueJitter = 0.05;
constexpr qreal kFallbackHue = 0.58; // 主题色是灰色时的冷蓝
constexpr qreal kMinSpeed = 60.0;
constexpr qreal kSpeedRange = 260.0;

constexpr qreal kCoreRadius = 1.2;
constexpr qreal kCoreRadiusPerLife = 2.0;
constexpr qreal kGlowScale = 3.0;
constexpr qreal kGlowAlpha = 0.45;
constexpr int kHintFontSize = 13; // 与 pages.css 的 #Placeholder 同级

auto hueOf(const QColor &color) -> qreal
{
    const qreal hue = color.hsvHueF();
    return hue < 0.0 ? kFallbackHue : hue;
}

auto isDarkPalette(const QPalette &palette) -> bool
{
    return palette.color(QPalette::Window).lightnessF() < 0.5;
}

} // namespace

class ParticlesPage::ParticlesPagePrivate
{
public:
    struct Particle
    {
        QPointF position;
        QPointF velocity;
        QColor color;
        qreal life = 1.0;
    };

    explicit ParticlesPagePrivate(ParticlesPage *q)
        : q_ptr(q)
    {}

    void advance();

    void paint(QPainter &painter);

    ParticlesPage *q_ptr = nullptr;
    QList<Particle> particles;
    QTimer timer;
    QElapsedTimer clock;
    QString emptyHint;
};

void ParticlesPage::ParticlesPagePrivate::advance()
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

void ParticlesPage::ParticlesPagePrivate::paint(QPainter &painter)
{
    const QRectF area
        = QRectF(q_ptr->rect()).adjusted(kBorderWidth, kBorderWidth, -kBorderWidth, -kBorderWidth);

    if (particles.isEmpty()) {
        // 空状态用占位色，和 #Placeholder 一个语汇：告诉用户这里可以点。
        QFont font = painter.font();
        font.setPixelSize(kHintFontSize);
        painter.setFont(font);
        painter.setPen(QGuiApplication::palette().color(QPalette::PlaceholderText));
        painter.drawText(area, Qt::AlignCenter, emptyHint);
        return;
    }

    painter.setPen(Qt::NoPen);
    for (const Particle &particle : std::as_const(particles)) {
        const qreal alpha = std::clamp(particle.life, 0.0, 1.0);
        const qreal radius = kCoreRadius + alpha * kCoreRadiusPerLife;

        // 光晕用 ObjectBoundingMode，一个渐变就跟着每颗粒子的外接框走。
        QColor glow = particle.color;
        glow.setAlphaF(alpha * kGlowAlpha);
        QColor fade = glow;
        fade.setAlphaF(0.0);

        QRadialGradient halo(QPointF(0.5, 0.5), 0.5);
        halo.setCoordinateMode(QGradient::ObjectBoundingMode);
        halo.setColorAt(0.0, glow);
        halo.setColorAt(1.0, fade);

        painter.setBrush(halo);
        painter.drawEllipse(particle.position, radius * kGlowScale, radius * kGlowScale);

        QColor core = particle.color;
        core.setAlphaF(alpha);
        painter.setBrush(core);
        painter.drawEllipse(particle.position, radius, radius);
    }
}

ParticlesPage::ParticlesPage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<ParticlesPagePrivate>(this))
{
    setMinimumSize(320, 240);
    setCursor(Qt::CrossCursor);

    d_ptr->timer.setInterval(kTimerIntervalMs);
    retranslateUi();
    setupConnections();
}

ParticlesPage::~ParticlesPage() = default;

void ParticlesPage::setupConnections()
{
    connect(&d_ptr->timer, &QTimer::timeout, this, &ParticlesPage::onFrameTimeout);
}

void ParticlesPage::retranslateUi()
{
    d_ptr->emptyHint = tr("Click anywhere to launch a burst");
}

void ParticlesPage::onFrameTimeout()
{
    d_ptr->advance();
}

void ParticlesPage::burst(const QPointF &center)
{
    auto *random = QRandomGenerator::global();
    // 主题色取应用调色板：控件自己的 palette() 在换肤过程中是分批到位的。
    const QPalette pal = QGuiApplication::palette();
    const qreal baseHue = hueOf(pal.color(QPalette::Highlight));
    // 浅色底上要把亮度压下来，否则浅色粒子在白卡片上几乎看不见。
    const bool isDark = isDarkPalette(pal);
    const qreal minValue = isDark ? 0.80 : 0.62;
    const qreal valueRange = isDark ? 0.20 : 0.26;

    for (int i = 0; i < kParticlesPerBurst; ++i) {
        const qreal angle = random->generateDouble() * 2.0 * std::numbers::pi;
        const qreal speed = kMinSpeed + random->generateDouble() * kSpeedRange;
        const qreal hue
            = std::fmod(baseHue + (random->generateDouble() * 2.0 - 1.0) * kHueJitter + 1.0, 1.0);

        d_ptr->particles.append(
            ParticlesPagePrivate::Particle{
                .position = center,
                .velocity = QPointF(std::cos(angle) * speed, std::sin(angle) * speed),
                .color = QColor::fromHsvF(
                    hue,
                    0.55 + random->generateDouble() * 0.40,
                    minValue + random->generateDouble() * valueRange),
            });
    }

    // 连点也不能让粒子无限堆积，超出的先丢最老的。
    if (d_ptr->particles.size() > kMaxParticles)
        d_ptr->particles.remove(0, d_ptr->particles.size() - kMaxParticles);

    if (!d_ptr->timer.isActive()) {
        d_ptr->clock.restart();
        d_ptr->timer.start();
    }
    update();
}

void ParticlesPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 卡片底板自己画，尺寸与配色同 QSS 的 #Card。
    const QPalette pal = QGuiApplication::palette();
    const qreal inset = kBorderWidth / 2.0;
    painter.setPen(QPen(pal.color(QPalette::Mid), kBorderWidth));
    painter.setBrush(pal.color(QPalette::Base));
    painter.drawRoundedRect(
        QRectF(rect()).adjusted(inset, inset, -inset, -inset), kCardRadius, kCardRadius);

    QPainterPath clip;
    clip.addRoundedRect(
        QRectF(rect()).adjusted(kBorderWidth, kBorderWidth, -kBorderWidth, -kBorderWidth),
        kCardRadius - kBorderWidth,
        kCardRadius - kBorderWidth);

    painter.save();
    painter.setClipPath(clip);
    d_ptr->paint(painter);
    painter.restore();
}

void ParticlesPage::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    burst(event->position());
}

void ParticlesPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    d_ptr->clock.restart();
    if (!d_ptr->particles.isEmpty())
        d_ptr->timer.start();
}

void ParticlesPage::hideEvent(QHideEvent *event)
{
    d_ptr->timer.stop();
    QWidget::hideEvent(event);
}

void ParticlesPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}

} // namespace Particles
