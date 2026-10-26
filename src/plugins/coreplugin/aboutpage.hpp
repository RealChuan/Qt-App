#pragma once

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

class QEvent;

using namespace Qt::StringLiterals;

namespace Core {

class AboutPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit AboutPage(QWidget *parent = nullptr);
    ~AboutPage() override;

    [[nodiscard]] auto id() const -> QString override { return u"coreplugin.about"_s; }
    [[nodiscard]] auto title() const -> QString override { return tr("About"); }
    [[nodiscard]] auto subtitle() const -> QString override
    {
        return tr("Version information and the environment this build runs on.");
    }
    [[nodiscard]] auto area() const -> Shell::PageArea override { return Shell::PageArea::Help; }
    [[nodiscard]] auto order() const -> int override { return Shell::PageOrder::kFirst; }
    [[nodiscard]] auto widget() -> QWidget * override { return this; }

protected:
    void changeEvent(QEvent *event) override;

private:
    void setupUi();
    void retranslateUi();
    void setupConnections();

    class AboutPagePrivate;
    std::unique_ptr<AboutPagePrivate> d_ptr;
};

} // namespace Core
