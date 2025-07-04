#include "languagemanager.hpp"
#include "utils.h"

namespace Utils {

class LanguageManager::LanguageManagerPrivate
{
public:
    explicit LanguageManagerPrivate(LanguageManager *q)
        : q_ptr(q)
    {}
    ~LanguageManagerPrivate() {}

    LanguageManager *q_ptr;

    Language currentLanguage = Chinese;
    QScopedPointer<QTranslator> translatorPtr;
    QScopedPointer<QTranslator> qtTranslatorPtr;
};

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new LanguageManagerPrivate(this))
{}

LanguageManager::~LanguageManager() {}

LanguageManager::Language LanguageManager::currentLanguage()
{
    return d_ptr->currentLanguage;
}

void LanguageManager::loadLanguage(Language language)
{
    d_ptr->currentLanguage = language;
    loadLanguage();
}

void LanguageManager::loadLanguage()
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

void LanguageManager::saveSettings(QSettings &settings)
{
    settings.beginGroup("Language");
    settings.setValue("Language", d_ptr->currentLanguage);
    settings.endGroup();
}

void LanguageManager::loadSettings(QSettings &settings)
{
    if (QLocale().language() == QLocale::Language::Chinese) {
        d_ptr->currentLanguage = Chinese;
    } else {
        d_ptr->currentLanguage = English;
    }
    settings.beginGroup("Language");
    d_ptr->currentLanguage = Language(settings.value("Language", d_ptr->currentLanguage).toInt());
    settings.endGroup();
}

} // namespace Utils
