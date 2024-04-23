#include "languageconfig.hpp"
#include "utils.h"

namespace Utils {

class LanguageConfig::LanguageConfigPrivate
{
public:
    explicit LanguageConfigPrivate(LanguageConfig *q)
        : q_ptr(q)
    {}
    ~LanguageConfigPrivate() = default;

    LanguageConfig *q_ptr;
    Language currentLanguage = Chinese;
    QScopedPointer<QTranslator> translatorPtr;
    QScopedPointer<QTranslator> qtTranslatorPtr;
};

LanguageConfig::Language LanguageConfig::currentLanguage()
{
    return d_ptr->currentLanguage;
}

void LanguageConfig::loadLanguage(Language language)
{
    d_ptr->currentLanguage = language;
    loadLanguage();
}

void LanguageConfig::loadLanguage()
{
    if (!d_ptr->translatorPtr.isNull()) {
        qApp->removeTranslator(d_ptr->translatorPtr.data());
    }
    if (!d_ptr->qtTranslatorPtr.isNull()) {
        qApp->removeTranslator(d_ptr->qtTranslatorPtr.data());
    }
    d_ptr->translatorPtr.reset(new QTranslator);
    d_ptr->qtTranslatorPtr.reset(new QTranslator);
#ifdef Q_OS_MACOS
    auto translationsPath = qApp->applicationDirPath() + "/../Resources/translations/";
#else
    auto translationsPath = qApp->applicationDirPath() + "/translations/";
#endif
    switch (d_ptr->currentLanguage) {
    case Chinese:
        qInfo() << d_ptr->translatorPtr->load(translationsPath + "qt-app_zh_CN.qm")
                << d_ptr->qtTranslatorPtr->load(translationsPath + "qt_zh_CN.qm");
        break;
    default:
        qInfo() << d_ptr->translatorPtr->load(translationsPath + "qt-app_en.qm")
                << d_ptr->qtTranslatorPtr->load(translationsPath + "qt_en.qm");
        break;
    }
    qApp->installTranslator(d_ptr->translatorPtr.data());
    qApp->installTranslator(d_ptr->qtTranslatorPtr.data());
}

LanguageConfig::LanguageConfig(QObject *parent)
    : QObject(parent)
    , d_ptr(new LanguageConfigPrivate(this))
{
    getConfig();
}

LanguageConfig::~LanguageConfig()
{
    saveConfig();
}

void LanguageConfig::getConfig()
{
    const QString configFile(Utils::configFilePath());
    if (QFileInfo::exists(configFile)) {
        QSettings setting(configFile, QSettings::IniFormat);
        setting.beginGroup("Language_config"); //向当前组追加前缀
        d_ptr->currentLanguage = Language(setting.value("Language").toInt());
        setting.endGroup();
    } else if (QLocale().language() == QLocale::Language::Chinese) {
        d_ptr->currentLanguage = Chinese;
    } else {
        d_ptr->currentLanguage = English;
    }
}

void LanguageConfig::saveConfig()
{
    const QString configFile(Utils::configFilePath());
    QSettings setting(configFile, QSettings::IniFormat);
    setting.beginGroup("Language_config");
    setting.setValue("Language", d_ptr->currentLanguage);
    setting.endGroup();
}

} // namespace Utils
