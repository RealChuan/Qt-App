#pragma once

#include "fortuneengine.hpp"

#include <QWidget>

#include <memory>

namespace Fortune {

// The profile form: name, birth date and birth hour. It owns no persistence and
// no validation beyond what the widgets guarantee; the page does both.
class FortuneForm final : public QWidget
{
    Q_OBJECT

public:
    explicit FortuneForm(QWidget *parent = nullptr);
    ~FortuneForm() override;

    void setProfile(const Profile &profile);
    void clear();
    [[nodiscard]] auto profile() const -> Profile;
    void focusName();
    // 编辑已保存的档案时才露出返回和删除：首次填写既没有可退回的页面，
    // 也没有可删的档案，两个按钮都没有意义。
    void setEditing(bool editing);

signals:
    void submitted(const Profile &profile);
    void backRequested();
    void deleteRequested();

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onSubmitClicked();

private:
    void setupUi();
    void retranslateUi();
    void setupConnections();

    class FortuneFormPrivate;
    std::unique_ptr<FortuneFormPrivate> d_ptr;
};

} // namespace Fortune
