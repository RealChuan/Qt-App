#include "iplugin.hpp"

namespace ExtensionSystem {

IPlugin::IPlugin(QObject *parent)
    : QObject(parent)
{}

IPlugin::~IPlugin() = default;

} // namespace ExtensionSystem
