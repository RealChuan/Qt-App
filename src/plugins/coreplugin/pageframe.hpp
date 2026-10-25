#pragma once

#include <QString>
#include <QWidget>

#include <memory>

namespace Core {

// 每个页面外层套一层壳：标题与说明由外壳统一排版，插件只交出内容。
class PageFrame final : public QWidget
{
public:
    explicit PageFrame(QWidget *content, QWidget *parent = nullptr);
    ~PageFrame() override;

    void setTexts(const QString &title, const QString &subtitle);

private:
    class PageFramePrivate;
    std::unique_ptr<PageFramePrivate> d_ptr;
};

} // namespace Core
