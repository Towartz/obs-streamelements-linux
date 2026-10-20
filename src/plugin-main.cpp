#include <obs-module.h>
#include <obs-frontend-api.h>
#include <QtWidgets/QMainWindow>
#include <QtCore/QMetaObject>
#include "MenuManager.hpp"
#include "BrowserWidgetManager.hpp"
#include "ApiServer.hpp"
#include "AudioWrapper.hpp"
#include "AuthManager.hpp"
#include <memory>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-streamelements", "en-US")

#define BLOG(level, msg, ...) blog(level, "[obs-streamelements] " msg, ##__VA_ARGS__)

#define PLUGIN_VERSION_STRING "26.9.4.994-linux"
#define PLUGIN_VERSION_NUM 20260904000994ULL

static std::unique_ptr<StreamElementsBrowserWidgetManager> s_browserManager;
static std::unique_ptr<StreamElementsMenuManager> s_menuManager;
static std::unique_ptr<StreamElementsApiServer> s_apiServer;

MODULE_EXPORT const char *obs_module_description(void)
{
    return "StreamElements SE.Live Plugin for Linux";
}

MODULE_EXPORT const char *obs_module_name(void)
{
    return "obs-streamelements";
}

MODULE_EXPORT uint64_t streamelements_get_version_number(void)
{
    return PLUGIN_VERSION_NUM;
}

static void on_frontend_event(enum obs_frontend_event event, void *private_data)
{
    Q_UNUSED(private_data);
    
    if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
        BLOG(LOG_INFO, "OBS frontend finished loading; initializing StreamElements UI components");
        
        QMainWindow *mainWindow = static_cast<QMainWindow *>(obs_frontend_get_main_window());
        if (mainWindow) {
            QMetaObject::invokeMethod(mainWindow, [mainWindow]() {
                StreamElementsAuthManager::instance().init();

                if (!s_browserManager) {
                    s_browserManager = std::make_unique<StreamElementsBrowserWidgetManager>(mainWindow);
                    s_browserManager->registerDefaultDocks();
                }
                if (!s_menuManager) {
                    s_menuManager = std::make_unique<StreamElementsMenuManager>(mainWindow, s_browserManager.get());
                    BLOG(LOG_INFO, "StreamElementsMenuManager initialized on main window");
                }
            }, Qt::QueuedConnection);
        } else {
            BLOG(LOG_ERROR, "Failed to get OBS main window pointer from obs_frontend_get_main_window()");
        }
    } else if (event == OBS_FRONTEND_EVENT_STREAMING_STARTED) {
        if (s_apiServer) s_apiServer->broadcastEvent("streamingStarted", {});
    } else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPED) {
        if (s_apiServer) s_apiServer->broadcastEvent("streamingStopped", {});
    } else if (event == OBS_FRONTEND_EVENT_RECORDING_STARTED) {
        if (s_apiServer) s_apiServer->broadcastEvent("recordingStarted", {});
    } else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPED) {
        if (s_apiServer) s_apiServer->broadcastEvent("recordingStopped", {});
    } else if (event == OBS_FRONTEND_EVENT_SCENE_CHANGED) {
        if (s_apiServer) s_apiServer->broadcastEvent("sceneChanged", {});
    } else if (event == OBS_FRONTEND_EVENT_EXIT) {
        BLOG(LOG_INFO, "OBS frontend exiting; tearing down StreamElements plugin");
        s_menuManager.reset();
        s_browserManager.reset();
        if (s_apiServer) {
            s_apiServer->stop();
            s_apiServer.reset();
        }
    }
}

bool obs_module_load(void)
{
    BLOG(LOG_INFO, "Loading StreamElements SE.Live Linux Plugin v%s", PLUGIN_VERSION_STRING);

    register_audio_wrapper_source();

    s_apiServer = std::make_unique<StreamElementsApiServer>();
    if (!s_apiServer->start()) {
        BLOG(LOG_WARNING, "StreamElementsApiServer failed to start, will continue loading UI");
    }

    return true;
}

void obs_module_post_load(void)
{
    BLOG(LOG_INFO, "Registering frontend event callback");
    obs_frontend_add_event_callback(on_frontend_event, nullptr);
}

void obs_module_unload(void)
{
    BLOG(LOG_INFO, "Unloading StreamElements SE.Live Linux Plugin");
    obs_frontend_remove_event_callback(on_frontend_event, nullptr);
    s_menuManager.reset();
    s_browserManager.reset();
    if (s_apiServer) {
        s_apiServer->stop();
        s_apiServer.reset();
    }
}
