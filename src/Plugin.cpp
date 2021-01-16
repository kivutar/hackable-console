#include "Plugin.h"

#include "Logger.h"

#include <imguial_button.h>
#include <IconsFontAwesome4.h>

#include <stdlib.h>

#define TAG "[PMN] "

char const* hc::Plugin::getTypeName() {
    switch (getType()) {
        case Type::Audio: return "audio";
        case Type::Config: return "config";
        case Type::Input: return "input";
        case Type::Led: return "led";
        case Type::Logger: return "logger";
        case Type::Perf: return "perf";
        case Type::Video: return "video";
        case Type::Control: return "control";
        case Type::Memory: return "memory";
        case Type::Manager: return "manager";
    }

    abort();
}

hc::Plugins::Plugins() : _logger(nullptr) {}

void hc::Plugins::init(Logger* const logger) {
    _logger = logger;
}

void hc::Plugins::add(Plugin* const plugin) {
    View view = {plugin, true};
    _plugins.emplace_back(view);
}

char const* hc::Plugins::getName() {
    return "hc::Plugins built-in plugin manager";
}

char const* hc::Plugins::getVersion() {
    return "0.0.0";
}

char const* hc::Plugins::getLicense() {
    return "MIT";
}

char const* hc::Plugins::getCopyright() {
    return "Copyright (c) Andre Leiradella";
}

char const* hc::Plugins::getUrl() {
    return "https://github.com/leiradel/hackable-console";
}

void hc::Plugins::onStarted() {
    for (auto const& view : _plugins) {
        Plugin* const plugin = view.plugin;
        _logger->debug(TAG "onStarted plugin %s (%s): %s", plugin->getName(), plugin->getVersion(), plugin->getCopyright());
        plugin->onStarted();
    }
}

void hc::Plugins::onConsoleLoaded() {
    for (auto const& view : _plugins) {
        Plugin* const plugin = view.plugin;
        _logger->debug(TAG "onConsoleLoaded plugin %s (%s): %s", plugin->getName(), plugin->getVersion(), plugin->getCopyright());
        plugin->onConsoleLoaded();
    }
}

void hc::Plugins::onGameLoaded() {
    for (auto const& view : _plugins) {
        Plugin* const plugin = view.plugin;
        _logger->debug(TAG "onGameLoaded plugin %s (%s): %s", plugin->getName(), plugin->getVersion(), plugin->getCopyright());
        plugin->onGameLoaded();
    }
}

void hc::Plugins::onGamePaused() {
    for (auto const& view : _plugins) {
        Plugin* const plugin = view.plugin;
        _logger->debug(TAG "onGamePaused plugin %s (%s): %s", plugin->getName(), plugin->getVersion(), plugin->getCopyright());
        plugin->onGamePaused();
    }
}

void hc::Plugins::onGameResumed() {
    for (auto const& view : _plugins) {
        Plugin* const plugin = view.plugin;
        _logger->debug(TAG "onGameResumed plugin %s (%s): %s", plugin->getName(), plugin->getVersion(), plugin->getCopyright());
        plugin->onGameResumed();
    }
}

void hc::Plugins::onGameReset() {
    for (auto const& view : _plugins) {
        Plugin* const plugin = view.plugin;
        _logger->debug(TAG "onGameReset plugin %s (%s): %s", plugin->getName(), plugin->getVersion(), plugin->getCopyright());
        plugin->onGameReset();
    }
}

void hc::Plugins::onFrame() {
    for (auto const& view : _plugins) {
        Plugin* const plugin = view.plugin;
        // Don't log stuff per frame
        plugin->onFrame();
    }
}

void hc::Plugins::onDraw(bool* opened) {
    (void)opened; // the plugin manager is always visible

    if (ImGui::Begin(ICON_FA_PLUG " Plugins")) {
        ImGui::Columns(3);

        for (auto& view : _plugins) {
            Plugin* const plugin = view.plugin;

            ImGui::Text("%s", plugin->getName());
            ImGui::NextColumn();
            ImGui::Text("%s", plugin->getVersion());
            ImGui::NextColumn();

            char label[32];
            snprintf(label, sizeof(label), "Open##%p", static_cast<void*>(&view));

            if (ImGuiAl::Button(label, !view.opened)) {
                view.opened = true;
            }

            ImGui::NextColumn();
        }

        ImGui::Columns(1);
    }

    ImGui::End();

    for (auto& view : _plugins) {
        Plugin* const plugin = view.plugin;
        // Don't log stuff per frame

        if (plugin != this) {
            // Don't recursively draw the plugin manager
            plugin->onDraw(&view.opened);
        }
    }
}

void hc::Plugins::onGameUnloaded() {
    for (auto const& view : _plugins) {
        Plugin* const plugin = view.plugin;
        _logger->debug(TAG "onGameUnloaded plugin %s (%s): %s", plugin->getName(), plugin->getVersion(), plugin->getCopyright());
        plugin->onGameUnloaded();
    }
}

void hc::Plugins::onConsoleUnloaded() {
    for (auto const& view : _plugins) {
        Plugin* const plugin = view.plugin;
        _logger->debug(TAG "onConsoleUnloaded plugin %s (%s): %s", plugin->getName(), plugin->getVersion(), plugin->getCopyright());
        plugin->onConsoleUnloaded();
    }
}

void hc::Plugins::onQuit() {
    for (auto const& view : _plugins) {
        Plugin* const plugin = view.plugin;
        _logger->debug(TAG "onQuit plugin %s (%s): %s", plugin->getName(), plugin->getVersion(), plugin->getCopyright());
        plugin->onQuit();

        if (plugin != this) {
            delete plugin;
        }
    }

    _plugins.clear();
}

int hc::Plugins::push(lua_State* const L) {
    lua_createtable(L, 0, _plugins.size());

    for (auto const& view : _plugins) {
        Plugin* const plugin = view.plugin;
        plugin->push(L);
        lua_setfield(L, -2, plugin->getTypeName());
    }

    static struct {char const* const name; char const* const value;} const stringConsts[] = {
        {"_COPYRIGHT", "Copyright (c) 2020 Andre Leiradella"},
        {"_LICENSE", "MIT"},
        {"_VERSION", "1.0.0"},
        {"_NAME", "hc"},
        {"_URL", "https://github.com/leiradel/hackable-console"},
        {"_DESCRIPTION", "Hackable Console bindings"},

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        {"soExtension", "dll"}
#elif __linux__
        {"soExtension", "so"}
#else
        #error Unsupported platform
#endif
    };

    size_t const stringCount = sizeof(stringConsts) / sizeof(stringConsts[0]);

    for (size_t i = 0; i < stringCount; i++) {
        lua_pushstring(L, stringConsts[i].value);
        lua_setfield(L, -2, stringConsts[i].name);
    }

    return 1;
}
