#ifndef STRETCHPARAMSSETTINGDAILOG_HPP
#define STRETCHPARAMSSETTINGDAILOG_HPP

#include <controls/dialog.hpp>

using namespace Control;

class StretchParamsSettingDailog : public Dialog
{
    Q_OBJECT
public:
    struct StretchParams
    {
        QSize size = QSize(0, 0);
        Qt::AspectRatioMode mode = Qt::KeepAspectRatio;
    };

    explicit StretchParamsSettingDailog(QWidget *parent = nullptr);
    ~StretchParamsSettingDailog();

    void setParams(const StretchParams &params);
    StretchParams params() const;

private slots:
    void onReset();
    void onApply();

private:
    void setupUI();

    class StretchParamsSettingDailogPrivate;
    QScopedPointer<StretchParamsSettingDailogPrivate> d_ptr;
};

#endif // STRETCHPARAMSSETTINGDAILOG_HPP
