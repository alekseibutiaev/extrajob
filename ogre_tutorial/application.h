#pragma once

#include <memory>
#include <OgreString.h>
#include <OgreCommon.h>

namespace Ogre
{
    class Root;
    class RenderWindow;
    class SceneManager;
    class Camera;
}

class Application {
public:
    Application(const Ogre::String& plugin_config,
      const Ogre::String& resource_config);
    virtual ~Application();

    void startApplication();
    virtual void loadPlugins();
    void setRenderSystem();
    void initializeRenderSystem();
    virtual void createRenderWindow(const Ogre::String title = "Application",
      const unsigned int width = 800, const unsigned int height = 600,
      const bool full_screen = false, const Ogre::NameValuePairList* params = &Application::defparam );
    void parseResourceFileConfiguration();
    void initializeResources();

public:
    static const Ogre::NameValuePairList defparam;
protected:
    virtual void createScene();
    Ogre::SceneManager* create_scene_manager();
    Ogre::RenderWindow* get_render_window();
protected:
    const Ogre::String m_plugin_config;
    const Ogre::String m_resource_config;
    std::unique_ptr<Ogre::Root> root;
private:
    Ogre::RenderWindow* m_renderWindow;
};