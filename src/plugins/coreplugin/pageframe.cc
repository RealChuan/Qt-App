#include "pageframe.hpp"

#include <QLabel>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace Core {

class PageFrame::PageFramePrivate
{
public:
    QLabel *title = nullptr;
    QLabel *subtitle = nullptr;
};

PageFrame::PageFrame(QWidget *content, QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<PageFramePrivate>())
{
    setObjectName(u"PageFrame"_s);

    d_ptr->title = new QLabel(this);
    d_ptr->title->setObjectName(u"PageTitle"_s);

    d_ptr->subtitle = new QLabel(this);
    d_ptr->subtitle->setObjectName(u"PageSubtitle"_s);
    d_ptr->subtitle->setWordWrap(true);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 22, 28, 22);
    layout->setSpacing(4);
    layout->addWidget(d_ptr->title);
    layout->addWidget(d_ptr->subtitle);
    layout->addSpacing(16);
    layout->addWidget(content, 1);
}

PageFrame::~PageFrame() = default;

void PageFrame::setTexts(const QString &title, const QString &subtitle)
{
    d_ptr->title->setText(title);
    d_ptr->subtitle->setText(subtitle);
    d_ptr->subtitle->setVisible(!subtitle.isEmpty());
}

} // namespace Core
