#include "Desktop.h"

#include "Logger.h"
#include "Config.h"
#include "Video.h"
#include "Led.h"
#include "Audio.h"
#include "Input.h"
#include "Perf.h"

#include <imguial_button.h>
#include <IconsFontAwesome4.h>

#include <stdlib.h>
#include <math.h>

#define TAG "[DSK] "

hc::Desktop::Desktop()
    : View(nullptr)
    , _logger(nullptr)
    , _config(nullptr)
    , _video(nullptr)
    , _led(nullptr)
    , _audio(nullptr)
    , _input(nullptr)
    , _perf(nullptr)
{}

void hc::Desktop::init() {
    _drawCount = 0;
    _frameCount = 0;
}

void hc::Desktop::add(View* const view, bool const top, bool const free, char const* const id) {
    ViewProperties props = {view, top, free, id, true};
    _views.emplace(id != nullptr ? id : view->getTitle(), props);

    if (dynamic_cast<Logger*>(view) != nullptr) {
        _logger = dynamic_cast<Logger*>(view);
    }
    else if (dynamic_cast<Config*>(view) != nullptr) {
        _config = dynamic_cast<Config*>(view);
    }
    else if (dynamic_cast<Video*>(view) != nullptr) {
        _video = dynamic_cast<Video*>(view);
    }
    else if (dynamic_cast<Led*>(view) != nullptr) {
        _led = dynamic_cast<Led*>(view);
    }
    else if (dynamic_cast<Audio*>(view) != nullptr) {
        _audio = dynamic_cast<Audio*>(view);
    }
    else if (dynamic_cast<Input*>(view) != nullptr) {
        _input = dynamic_cast<Input*>(view);
    }
    else if (dynamic_cast<Perf*>(view) != nullptr) {
        _perf = dynamic_cast<Perf*>(view);
    }
}

double hc::Desktop::drawFps() {
    double const fps = static_cast<double>(_drawCount) * 1000000.0f / static_cast<double>(_drawTimer.getTimeUs());
    return isnan(fps) || isinf(fps) ? 0.0 : fps;
}

void hc::Desktop::resetDrawFps() {
    if (_drawTimer.started()) {
        _drawCount = 0;
        _drawTimer.reset();
    }
}

double hc::Desktop::frameFps() {
    double const fps = static_cast<double>(_frameCount) * 1000000.0f / static_cast<double>(_frameTimer.getTimeUs());
    return isnan(fps) || isinf(fps) ? 0.0 : fps;
}

void hc::Desktop::resetFrameFps() {
    if (_frameTimer.started()) {
        _frameCount = 0;
        _frameTimer.reset();
    }
}

hc::Logger* hc::Desktop::getLogger() const {
    return _logger;
}

hc::Config* hc::Desktop::getConfig() const {
    return _config;
}

hc::Video* hc::Desktop::getVideo() const {
    return _video;
}

hc::Led* hc::Desktop::getLed() const {
    return _led;
}

hc::Audio* hc::Desktop::getAudio() const {
    return _audio;
}

hc::Input* hc::Desktop::getInput() const {
    return _input;
}

hc::Perf* hc::Desktop::getPerf() const {
    return _perf;
}

char const* hc::Desktop::getTitle() {
    return ICON_FA_PLUG " Views";
}

void hc::Desktop::onStarted() {
    for (auto const& pair : _views) {
        View* const view = pair.second.view;
        _logger->debug(TAG "onStarted %s", view->getTitle());
        view->onStarted();
    }

    _drawTimer.start();
    _drawCount = 0;
}

void hc::Desktop::onCoreLoaded() {
    for (auto const& pair : _views) {
        View* const view = pair.second.view;
        _logger->debug(TAG "onCoreLoaded %s", view->getTitle());
        view->onCoreLoaded();
    }
}

void hc::Desktop::onGameLoaded() {
    for (auto const& pair : _views) {
        View* const view = pair.second.view;
        _logger->debug(TAG "onGameLoaded %s", view->getTitle());
        view->onGameLoaded();
    }
}

void hc::Desktop::onGameStarted() {
    for (auto const& pair : _views) {
        View* const view = pair.second.view;
        _logger->debug(TAG "onGameStarted %s", view->getTitle());
        view->onGameStarted();
    }

    _frameTimer.start();
    _frameCount = 0;
}

void hc::Desktop::onGamePaused() {
    for (auto const& pair : _views) {
        View* const view = pair.second.view;
        _logger->debug(TAG "onGamePaused %s", view->getTitle());
        view->onGamePaused();
    }

    _frameTimer.pause();
}

void hc::Desktop::onGameResumed() {
    for (auto const& pair : _views) {
        View* const view = pair.second.view;
        _logger->debug(TAG "onGameResumed %s", view->getTitle());
        view->onGameResumed();
    }

    _frameTimer.resume();
}

void hc::Desktop::onGameReset() {
    for (auto const& pair : _views) {
        View* const view = pair.second.view;
        _logger->debug(TAG "onGameReset %s", view->getTitle());
        view->onGameReset();
    }
}

void hc::Desktop::onFrame() {
    if (!_frameTimer.paused()) {
        _frameCount++;
    }

    for (auto const& pair : _views) {
        View* const view = pair.second.view;
        // Don't log stuff per frame
        view->onFrame();
    }
}

void hc::Desktop::onStep() {
    for (auto const& pair : _views) {
        View* const view = pair.second.view;
        // Don't log stuff per frame
        view->onStep();
    }
}

void hc::Desktop::onDraw() {
    _drawCount++;

    if (ImGui::Begin(ICON_FA_PLUG " Views")) {
        ImGui::Columns(2);

        for (auto& pair : _views) {
            ViewProperties& props = pair.second;
            View* const view = props.view;

            if (!props.top) {
                continue;
            }

            ImGui::Text("%s", view->getTitle());
            ImGui::NextColumn();

            char label[32];
            snprintf(label, sizeof(label), "Open##%p", reinterpret_cast<void const*>(&props));

            if (ImGuiAl::Button(label, !props.opened)) {
                props.opened = true;
            }

            ImGui::NextColumn();
        }

        ImGui::Columns(1);
    }

    ImGui::End();

    for (auto& pair : _views) {
        ViewProperties& props = pair.second;
        View* const view = pair.second.view;
        // Don't log stuff per frame

        // Don't recursively draw the plugin manager
        if (view != this && props.opened) {
            if (ImGui::Begin(view->getTitle(), &props.opened)) {
                view->onDraw();
                ImGui::End();
            }
        }
    }
}

void hc::Desktop::onGameUnloaded() {
    for (auto const& pair : _views) {
        View* const view = pair.second.view;
        _logger->debug(TAG "onGameUnloaded %s", view->getTitle());
        view->onGameUnloaded();
    }

    _frameTimer.stop();
}

void hc::Desktop::onCoreUnloaded() {
    for (auto const& pair : _views) {
        View* const view = pair.second.view;
        _logger->debug(TAG "onCoreUnloaded %s", view->getTitle());
        view->onCoreUnloaded();
    }
}

void hc::Desktop::onQuit() {
    for (auto const& pair : _views) {
        ViewProperties const& props = pair.second;
        View* const view = props.view;
        _logger->debug(TAG "onQuit plugin %s", view->getTitle());
        view->onQuit();

        if (view != this && props.free) {
            delete view;
        }
    }

    _views.clear();
    _drawTimer.stop();
}

int hc::Desktop::push(lua_State* const L) {
    lua_createtable(L, 0, _views.size());

    for (auto const& pair : _views) {
        ViewProperties const& props = pair.second;
        View* const view = props.view;

        auto const scriptable = dynamic_cast<Scriptable*>(view);

        if (scriptable != nullptr) {
            scriptable->push(L);
            lua_setfield(L, -2, props.id);
        }
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
