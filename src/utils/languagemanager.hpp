#pragma once

#include "utils_global.hpp"

#include <QObject>

#include <memory>

class QSettings;

namespace Utils {

class UTILS_EXPORT LanguageManager : public QObject
{
    Q_OBJECT
public:
    enum Language : int { SimplifiedChinese, English };
    Q_ENUM(Language)

    explicit LanguageManager(QObject *parent = nullptr);
    ~LanguageManager() override;

    [[nodiscard]] auto currentLanguage() const -> Language;

    void loadLanguage(Language language);
    void loadLanguage();

    void saveSettings(QSettings &settings) const;
    void loadSettings(QSettings &settings);

private:
    class LanguageManagerPrivate;
    std::unique_ptr<LanguageManagerPrivate> d_ptr;
};

} // namespace Utils
