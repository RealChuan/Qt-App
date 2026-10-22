#include "plugindetailsview.hpp"

#include <extensionsystem/pluginspec.hpp>

#include <QEvent>
#include <QGridLayout>
#include <QHash>
#include <QLabel>
#include <QVBoxLayout>

namespace Core {

class PluginDetailsView::PluginDetailsViewPrivate
{
public:
    explicit PluginDetailsViewPrivate(PluginDetailsView *q)
        : q_ptr(q)
    {}

    void setupUi();

    PluginDetailsView *q_ptr = nullptr;
    ExtensionSystem::PluginSpec *spec = nullptr;

    QLabel *placeholder = nullptr;
    QWidget *content = nullptr;
    QLabel *name = nullptr;
    QGridLayout *grid = nullptr;
    QLabel *state = nullptr;
    QLabel *id = nullptr;
    QLabel *version = nullptr;
    QLabel *vendor = nullptr;
    QLabel *category = nullptr;
    QLabel *location = nullptr;
    QLabel *dependencies = nullptr;
    QLabel *description = nullptr;
    QLabel *errorTitle = nullptr;
    QLabel *errorText = nullptr;

    // 值控件 -> 字段名控件，语言切换时按值控件找回标题
    QHash<QWidget *, QLabel *> fieldLabels;
};

// 只建控件；文案在 retranslateUi()，内容在 updateContents()。
void PluginDetailsView::PluginDetailsViewPrivate::setupUi()
{
    name = new QLabel(q_ptr);
    name->setObjectName("DetailsTitle");
    name->setWordWrap(true);
    name->setTextInteractionFlags(Qt::TextSelectableByMouse);

    state = new QLabel(q_ptr);
    id = new QLabel(q_ptr);
    version = new QLabel(q_ptr);
    vendor = new QLabel(q_ptr);
    category = new QLabel(q_ptr);
    location = new QLabel(q_ptr);
    dependencies = new QLabel(q_ptr);
    description = new QLabel(q_ptr);
    errorText = new QLabel(q_ptr);
    for (QLabel *label :
         {state, id, version, vendor, category, location, dependencies, description, errorText}) {
        label->setObjectName("DetailsValue");
        label->setWordWrap(true);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }
    errorText->setObjectName("ErrorLabel");

    // 用 QGridLayout 而不是 QFormLayout：字段名给固定列宽，值列独占剩余宽度，
    // 路径、描述这类长文本才会在正确宽度上换行（QFormLayout 在滚动区里会让
    // 折行标签算出过宽的宽度，文字被卡片边缘裁掉）。
    grid = new QGridLayout;
    grid->setContentsMargins({});
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(8);
    // 96 是 "Dependencies:" 在 13px 下的宽度，再窄就和值贴在一起了。
    grid->setColumnMinimumWidth(0, 96);
    grid->setColumnStretch(1, 1);

    int row = 0;
    const auto addField = [this, &row](QLabel *field) {
        auto *label = new QLabel(content);
        label->setObjectName("FieldLabel");
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        // Ignored：不按“最长单词”要最小宽度，宽度完全由列宽决定。
        // 否则路径、描述这类长文本会把内容撑得比滚动区还宽，右侧被裁掉。
        field->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
        grid->addWidget(label, row, 0, Qt::AlignTop | Qt::AlignLeft);
        grid->addWidget(field, row, 1);
        fieldLabels.insert(field, label);
        ++row;
    };

    content = new QWidget(q_ptr);
    addField(state);
    addField(id);
    addField(version);
    addField(vendor);
    addField(category);
    addField(location);
    addField(dependencies);
    addField(description);
    addField(errorText);
    errorTitle = fieldLabels.value(errorText);

    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins({});
    contentLayout->setSpacing(14);
    contentLayout->addWidget(name);
    contentLayout->addLayout(grid);
    contentLayout->addStretch();

    placeholder = new QLabel(q_ptr);
    placeholder->setObjectName("Placeholder");
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setWordWrap(true);

    auto *layout = new QVBoxLayout(q_ptr);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(placeholder);
    layout->addWidget(content);
}

PluginDetailsView::PluginDetailsView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<PluginDetailsViewPrivate>(this))
{
    d_ptr->setupUi();
    retranslateUi();
    updateContents();
}

PluginDetailsView::~PluginDetailsView() = default;

void PluginDetailsView::setSpec(ExtensionSystem::PluginSpec *spec)
{
    d_ptr->spec = spec;
    updateContents();
}

void PluginDetailsView::retranslateUi()
{
    d_ptr->placeholder->setText(tr("Select a plugin to view its details."));
    setFieldLabel(d_ptr->state, tr("State:"));
    setFieldLabel(d_ptr->id, tr("Id:"));
    setFieldLabel(d_ptr->version, tr("Version:"));
    setFieldLabel(d_ptr->vendor, tr("Vendor:"));
    setFieldLabel(d_ptr->category, tr("Category:"));
    setFieldLabel(d_ptr->location, tr("Location:"));
    setFieldLabel(d_ptr->dependencies, tr("Dependencies:"));
    setFieldLabel(d_ptr->description, tr("Description:"));
    setFieldLabel(d_ptr->errorText, tr("Error:"));
}

void PluginDetailsView::updateContents()
{
    ExtensionSystem::PluginSpec *spec = d_ptr->spec;
    const bool hasSpec = spec != nullptr;
    d_ptr->placeholder->setVisible(!hasSpec);
    d_ptr->content->setVisible(hasSpec);
    if (!hasSpec)
        return;

    d_ptr->name->setText(spec->displayName());
    d_ptr->state->setText(spec->stateText());
    d_ptr->id->setText(spec->id());
    d_ptr->version->setText(
        spec->version() == spec->compatVersion()
            ? spec->version()
            : QStringLiteral("%1 (%2)").arg(spec->version(), spec->compatVersion()));
    d_ptr->vendor->setText(spec->vendor());
    d_ptr->category->setText(spec->category());
    d_ptr->location->setText(spec->filePath().toUserOutput());

    QStringList dependencies;
    for (const ExtensionSystem::PluginDependency &dependency : spec->dependencies()) {
        QString text = dependency.id;
        if (!dependency.version.isEmpty())
            text += QStringLiteral(" >= %1").arg(dependency.version);
        if (dependency.type == ExtensionSystem::PluginDependency::Type::Optional)
            text += QStringLiteral(" (%1)").arg(tr("optional"));
        dependencies.append(text);
    }
    d_ptr->dependencies->setText(
        dependencies.isEmpty() ? tr("None") : dependencies.join(QStringLiteral(", ")));

    d_ptr->description->setText(spec->longDescription());

    const bool hasError = spec->hasError();
    d_ptr->errorText->setText(spec->errorString());
    d_ptr->errorText->setVisible(hasError);
    if (d_ptr->errorTitle)
        d_ptr->errorTitle->setVisible(hasError);
}

void PluginDetailsView::setFieldLabel(QWidget *field, const QString &text)
{
    if (auto *label = d_ptr->fieldLabels.value(field))
        label->setText(text);
}

void PluginDetailsView::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        // stateText() 与依赖里的"optional"都来自翻译，需要重算一次。
        updateContents();
    }
}

} // namespace Core
