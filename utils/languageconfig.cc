#include "languageconfig.hpp"
#include "utils.h"

namespace Utils {

LanguageConfig::Language LanguageConfig::currentLanguage()
{
    return m_currentLanguage;
}

void LanguageConfig::loadLanguage(Language language)
{
    m_currentLanguage = language;
    loadLanguage();
}

void LanguageConfig::loadLanguage()
{
    if (!m_translatorPtr.isNull()) {
        qApp->removeTranslator(m_translatorPtr.data());
    }
    m_translatorPtr.reset(new QTranslator);
    switch (m_currentLanguage) {
    case Chinese:
        qInfo() << m_translatorPtr->load(qApp->applicationDirPath()
                                         + "/translations/language.zh_cn.qm");
        break;
    case English:
        qInfo() << m_translatorPtr->load(qApp->applicationDirPath()
                                         + "/translations/language.zh_en.qm");
        break;
    default:
        qInfo() << m_translatorPtr->load(qApp->applicationDirPath()
                                         + "/translations/language.zh_cn.qm");
        break;
    }
    qApp->installTranslator(m_translatorPtr.data());
}

LanguageConfig::LanguageConfig(QObject *parent)
    : QObject(parent)
{
    getConfig();
}

LanguageConfig::~LanguageConfig()
{
    saveConfig();
}

void LanguageConfig::getConfig()
{
    Utils::generateDirectorys(Utils::getConfigPath() + "/config");
    const QString configFile(Utils::getConfigPath() + "/config/config.ini");
    if (QFileInfo::exists(configFile)) {
        QSettings setting(configFile, QSettings::IniFormat);
        setting.beginGroup("Language_config"); //向当前组追加前缀
        m_currentLanguage = Language(setting.value("Language").toInt());
        setting.endGroup();
    } else if (QLocale().language() == QLocale::Language::Chinese) {
        m_currentLanguage = Chinese;
    } else {
        m_currentLanguage = English;
    }
}

void LanguageConfig::saveConfig()
{
    const QString configFile(Utils::getConfigPath() + "/config/config.ini");
    QSettings setting(configFile, QSettings::IniFormat);
    setting.beginGroup("Language_config");
    setting.setValue("Language", m_currentLanguage);
    setting.endGroup();
}

} // namespace Utils
