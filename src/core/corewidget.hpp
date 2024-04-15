#ifndef COREWIDGET_HPP
#define COREWIDGET_HPP

#include "core_global.h"

#include <QObject>

class QPushButton;

namespace Core {

class CORE_EXPORT CoreWidget : public QObject
{
    Q_OBJECT
public:
    enum Type { Tool, About };

    explicit CoreWidget(QObject *parent = nullptr);
    ~CoreWidget() override;

    [[nodiscard]] auto button() const -> QPushButton *;
    [[nodiscard]] auto widget() const -> QWidget *;

protected:
    void setWidget(QWidget *widget);
    void setButton(QPushButton *button, Type type);

private:
    class CoreWidgetPrivate;
    QScopedPointer<CoreWidgetPrivate> d_ptr;
};

} // namespace Core

#endif // COREWIDGET_HPP
