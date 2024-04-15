#ifndef LANGUAGECONFIG_HPP
#define LANGUAGECONFIG_HPP

#include "singleton.hpp"
#include "utils_global.h"

#include <QtCore/QObject>

class QTranslator;

namespace Utils {

class UTILS_EXPORT LanguageConfig : public QObject
{
    Q_OBJECT
public:
    enum Language { Chinese, English };

    auto currentLanguage() -> Language;

    void loadLanguage(Language language);
    void loadLanguage();

private:
    explicit LanguageConfig(QObject *parent = nullptr);
    ~LanguageConfig() override;

    void getConfig();
    void saveConfig();

    Language m_currentLanguage = Chinese;
    QScopedPointer<QTranslator> m_translatorPtr;

    SINGLETON(LanguageConfig)
};

} // namespace Utils

#endif // LANGUAGECONFIG_HPP
