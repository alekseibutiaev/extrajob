#pragma once

#include <memory>
#include <functional>

#include <OgreString.h>
#include <OgreCommon.h>
#include <OgreFrameListener.h>

#include <InputContext.h>

#include <OISMouse.h>
#include <OISKeyboard.h>
#include <OISPrereqs.h>


namespace Ogre
{
  class Root;
  class RenderWindow;
  class SceneManager;
  class Camera;
}

namespace OIS {
  class OISInputManager;
}

class Application
    : public Ogre::FrameListener
    , public OIS::KeyListener
    , public OIS::MouseListener
     {
public:
  class frame_listener {
  public:
    using frame_event_t = std::function<bool(const Ogre::FrameEvent&)>;
  public:
    frame_event_t m_started;
    frame_event_t m_rendering_queued;
    frame_event_t m_ended;
  };
  class mouse_listener {
  public:
    using mouse_move_f = std::function<bool(const OIS::MouseEvent&)>;  
    using mouse_button_event_f = std::function<bool(const OIS::MouseEvent&, OIS::MouseButtonID)>;
  public:
    mouse_move_f m_move;  
    mouse_button_event_f m_pressed;
    mouse_button_event_f m_released;
  };
  class key_listener {
  public:
    using key_event_f = std::function<bool(const OIS::KeyEvent&)>;  
  public:
    key_event_f m_pressed;  
    key_event_f m_released;
  };
  using frame_listener_ptr = std::unique_ptr<frame_listener>;
  using mouse_listener_ptr = std::unique_ptr<mouse_listener>;
  using key_listener_ptr = std::unique_ptr<key_listener>;
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
  void start_input(OIS::ParamList value = Application::oisdefault);
  void stop_input();
public:
  static const Ogre::NameValuePairList defparam;
  static const OIS::ParamList oisdefault;
protected:
  virtual void createScene();
  Ogre::SceneManager* create_scene_manager();
  Ogre::RenderWindow* get_render_window();
  void set_frame_listener(frame_listener_ptr&& value);
  void set_key_listener(key_listener_ptr&& value);
  void set_mouse_listener(mouse_listener_ptr&& value);
protected:
  using input_manager_ptr = std::unique_ptr<OIS::InputManager, void(*)(OIS::InputManager*)>;
protected:
  const Ogre::String m_plugin_config;
  const Ogre::String m_resource_config;
  std::unique_ptr<Ogre::Root> m_root;
  input_manager_ptr m_input_manager;
private:
  void windowResized();
  // Ogre::FrameListener
  bool frameStarted(const Ogre::FrameEvent& value);
  bool frameRenderingQueued(const Ogre::FrameEvent& value);
  bool frameEnded(const Ogre::FrameEvent& value);
  // OIS::MouseListener  
  bool mouseMoved(const OIS::MouseEvent& value);
  bool mousePressed(const OIS::MouseEvent& value, OIS::MouseButtonID id ) override;
  bool mouseReleased(const OIS::MouseEvent& value, OIS::MouseButtonID id ) override;
  // OIS::MouseListener  
  bool keyPressed(const OIS::KeyEvent& value) override;
  bool keyReleased(const OIS::KeyEvent& value) override;
private:
  OgreBites::InputContext m_input_context;
  Ogre::RenderWindow* m_renderWindow = 0;
  frame_listener_ptr m_frame_listener;
  key_listener_ptr m_key_listner;
  mouse_listener_ptr m_mouse_listner;
};