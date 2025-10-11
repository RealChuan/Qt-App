#pragma once

#include "core_global.h"

#include <QList>
#include <QObject>

class QPushButton;
class QWidget;

namespace Core {

class CORE_EXPORT CoreWidget : public QObject
{
    Q_OBJECT
public:
    enum Type : int { Main, Help };

    explicit CoreWidget(QObject *parent = nullptr);
    ~CoreWidget() override;

    [[nodiscard]] auto button() const -> QPushButton *;
    [[nodiscard]] auto widget() const -> QWidget *;
    [[nodiscard]] auto type() const -> Type;

protected:
    void setWidget(QWidget *widget);
    void setButton(QPushButton *button, Type type);

private:
    class CoreWidgetPrivate;
    QScopedPointer<CoreWidgetPrivate> d_ptr;
};

using CoreWidgetList = QList<CoreWidget *>;

CORE_EXPORT void addCoreWidget(CoreWidget *widget);
CORE_EXPORT auto getCoreWidgets() -> CoreWidgetList &;

} // namespace Core
