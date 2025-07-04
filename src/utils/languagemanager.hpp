#pragma once

#include "utils_global.h"

#include <QObject>

class QSettings;

namespace Utils {

class UTILS_EXPORT LanguageManager : public QObject
{
    Q_OBJECT
public:
    enum Language : int { Chinese, English };

    explicit LanguageManager(QObject *parent = nullptr);
    ~LanguageManager();

    Language currentLanguage();

    void loadLanguage(Language language);
    void loadLanguage();

    void saveSettings(QSettings &settings);
    void loadSettings(QSettings &settings);

private:
    class LanguageManagerPrivate;
    QScopedPointer<LanguageManagerPrivate> d_ptr;
};

} // namespace Utils