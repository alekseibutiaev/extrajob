#include <cassert>

#include <string>

#include <Ogre.h>
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreCamera.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreException.h>
#include <OgreEntity.h>

#include <OISInputManager.h>

#include "application.h"


const Ogre::NameValuePairList Application::defparam = {
  {"FSAA", "0"},
  {"VSync", "No"},
  {"macAPICocoaUseNSView", "true"},
  {"FSAAH", "Quality"},
  {"contentScalingFactor", "1"},
  {"displayFrequency", "0"},
  {"macAPI", "cocoa"}
};

const OIS::ParamList Application::oisdefault = {
  {"x11_keyboard_grab", "false"},
  {"x11_mouse_grab", "false"},
  {"w32_mouse", "DISCL_FOREGROUND"},
  {"w32_mouse", "DISCL_NONEXCLUSIVE"},
  {"w32_keyboard", "DISCL_FOREGROUND"},
  {"w32_keyboard", "DISCL_NONEXCLUSIVE"},
};

Application::Application(const Ogre::String& plugin_config,
      const Ogre::String& resource_config)
    : m_plugin_config(plugin_config)
    , m_resource_config(resource_config)
    , m_root(new Ogre::Root(m_plugin_config))
    , m_input_manager(0, &OIS::InputManager::destroyInputSystem) {
  loadPlugins();
  setRenderSystem();
  initializeRenderSystem();
  createRenderWindow();
}

Application::~Application() {
  stop_input();
}

void Application::startApplication()
{
  parseResourceFileConfiguration();
  initializeResources();
  createScene();
  m_root->addFrameListener(this);
  m_root->startRendering();
  m_root->removeFrameListener(this);

}

void Application::loadPlugins()
{
#if 0
    // Set the render system. In this case OpenGL
    Ogre::GLPlugin* gLPlugin = new Ogre::GLPlugin();
    root->installPlugin( gLPlugin );
    Ogre::ParticleFXPlugin *particleFXPlugin = new Ogre::ParticleFXPlugin();
    root->installPlugin( particleFXPlugin );
#endif    
}

void Application::setRenderSystem()
{
  const Ogre::RenderSystemList &renderSystemList = m_root->getAvailableRenderers();
  if( renderSystemList.size() == 0 )
    throw Ogre::Exception(Ogre::Exception::ERR_RENDERINGAPI_ERROR, "Sorry, no rendersystem was found.", __FILE__);
  Ogre::RenderSystem *lRenderSystem = renderSystemList[ 0 ];
  Ogre::String renderSystemName = lRenderSystem->getName();
  Ogre::LogManager::getSingleton().logMessage( "Render System found: " + renderSystemName, Ogre::LML_NORMAL );
  m_root->setRenderSystem( lRenderSystem );
}

void Application::initializeRenderSystem()
{
    bool createAWindowAutomatically = false;
    Ogre::String windowTitle = "";
    Ogre::String customCapacities = "";
    m_root->initialise( createAWindowAutomatically, windowTitle, customCapacities );
}

void Application::createRenderWindow(const Ogre::String title,
      const unsigned int width, const unsigned int height,
      const bool full_screen, const Ogre::NameValuePairList* params)
{
  m_renderWindow = m_root->createRenderWindow( title, width, height, full_screen, params);
}

void Application::initializeResources()
{
    // Set default mipmap level (note: some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps( 5 );
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void Application::start_input(OIS::ParamList value) {
  static const char* name_win = "WINDOW";
  std::size_t handle;
  get_render_window()->getCustomAttribute(name_win, &handle);
  value.insert({name_win, std::to_string(handle)});
  m_input_manager = std::move(input_manager_ptr(OIS::InputManager::createInputSystem(value),
    &OIS::InputManager::destroyInputSystem));
}

void Application::stop_input() {
  set_key_listener(key_listener_ptr());
  set_mouse_listener(mouse_listener_ptr());
  m_input_manager.reset();
}

void Application::parseResourceFileConfiguration()
{
    // set up resources and load resource paths from config file
    Ogre::String resourcesConfigurationPath = m_resource_config;
    Ogre::ConfigFile configurationFile;
    configurationFile.load( resourcesConfigurationPath );

    // Go through all sections & settings in the resource file
    Ogre::ConfigFile::SectionIterator sectionNameIterator = configurationFile.getSectionIterator();
    Ogre::String sectionNameOfTheResource;
    Ogre::String typeNameOfTheResource;
    Ogre::String absolutePathToTheResource;

    while( sectionNameIterator.hasMoreElements() )
    {
        sectionNameOfTheResource = sectionNameIterator.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = sectionNameIterator.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for( i = settings->begin(); i != settings->end(); ++i )
        {
            typeNameOfTheResource = i->first;
            absolutePathToTheResource = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation( absolutePathToTheResource, typeNameOfTheResource, sectionNameOfTheResource );
        }
    }

    const Ogre::ResourceGroupManager::LocationList genLocs = Ogre::ResourceGroupManager::getSingleton().getResourceLocationList("General");
    absolutePathToTheResource = "/opt/ogre-1.9/share/OGRE/Media";
    typeNameOfTheResource = "FileSystem";
    sectionNameOfTheResource = "Popular";

    // Add locations for supported shader languages
    if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsles"))
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(absolutePathToTheResource + "/materials/programs/GLSLES", typeNameOfTheResource, sectionNameOfTheResource);
    }
    else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl"))
    {
        if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl150"))
        {
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(absolutePathToTheResource + "/materials/programs/GLSL150", typeNameOfTheResource, sectionNameOfTheResource);
        }
        else
        {
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(absolutePathToTheResource + "/materials/programs/GLSL", typeNameOfTheResource, sectionNameOfTheResource);
        }

        if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl400"))
        {
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(absolutePathToTheResource + "/materials/programs/GLSL400", typeNameOfTheResource, sectionNameOfTheResource);
        }
    }
    else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(absolutePathToTheResource + "/materials/programs/HLSL", typeNameOfTheResource, sectionNameOfTheResource);
    }
#		ifdef OGRE_BUILD_PLUGIN_CG
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(absolutePathToTheResource + "/materials/programs/Cg", typeNameOfTheResource, sectionNameOfTheResource);
#		endif

#		ifdef INCLUDE_RTSHADER_SYSTEM
    if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsles"))
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(absolutePathToTheResource + "/RTShaderLib/GLSLES", typeNameOfTheResource, sectionNameOfTheResource);
    }
    else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl"))
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(absolutePathToTheResource + "/RTShaderLib/GLSL", typeNameOfTheResource, sectionNameOfTheResource);
        if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl150"))
        {
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(absolutePathToTheResource + "/RTShaderLib/GLSL150", typeNameOfTheResource, sectionNameOfTheResource);
        }
    }
    else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(absolutePathToTheResource + "/RTShaderLib/HLSL", typeNameOfTheResource, sectionNameOfTheResource);
    }
#			ifdef OGRE_BUILD_PLUGIN_CG
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(absolutePathToTheResource + "/RTShaderLib/Cg", typeNameOfTheResource, sectionNameOfTheResource);
#			endif
#		endif // include_rtshader_system
}

void Application::createScene()
{
  Ogre::SceneManager* sceneManager = create_scene_manager();
  sceneManager->setAmbientLight( Ogre::ColourValue( 0.5, 0.5, 0.5 ) );

  Ogre::Camera* camera = sceneManager->createCamera("PlayerCam");
  camera->setPosition(0, 0, 120);
  camera->setNearClipDistance( 5 );

  Ogre::Viewport* viewPort = get_render_window()->addViewport( camera );
  viewPort->setBackgroundColour( Ogre::ColourValue( 0, 0, 0 ) );
  camera->setAspectRatio( Ogre::Real( viewPort->getActualWidth() ) / Ogre::Real( viewPort->getActualHeight() ) );

  // Create a Light and set its position
  Ogre::Light* light = sceneManager->createLight("MainLight");
  light->setPosition(20.0f, 80.0f, 50.0f);

  // Create an Entity
  Ogre::Entity* ogreHead = sceneManager->createEntity("Head", "ogrehead.mesh");
  // Create a SceneNode and attach the Entity to it
  Ogre::SceneNode* headNode = sceneManager->getRootSceneNode()->createChildSceneNode("HeadNode");
  headNode->attachObject(ogreHead);
}

Ogre::SceneManager* Application::create_scene_manager() {
  return m_root->createSceneManager(Ogre::ST_GENERIC);    
}

Ogre::RenderWindow* Application::get_render_window() {
  return m_renderWindow;
}

void Application::set_frame_listener(frame_listener_ptr&& value) {
  m_frame_listener = std::move(value);
}

void Application::set_key_listener(key_listener_ptr&& value) {
  assert(static_cast<bool>(m_input_manager));
  m_key_listner = std::move(value);  
  if(static_cast<bool>(m_key_listner)) {
    if(0 != (m_input_context.mKeyboard = static_cast<OIS::Keyboard*>(m_input_manager->createInputObject(OIS::OISKeyboard, true))))
      m_input_context.mKeyboard->setEventCallback(this);
  }
  else if(0 != m_input_context.mKeyboard) {
    m_input_context.mKeyboard->setEventCallback(0);  
    m_input_manager->destroyInputObject(m_input_context.mKeyboard);
    m_input_context.mKeyboard = 0;
  }
}

void Application::set_mouse_listener(mouse_listener_ptr&& value){
  assert(static_cast<bool>(m_input_manager));
  m_mouse_listner = std::move(value);
  if(static_cast<bool>(m_mouse_listner)) {
    if(0 != (m_input_context.mMouse = static_cast<OIS::Mouse*>(m_input_manager->createInputObject(OIS::OISMouse, true)))) {
      windowResized();
      m_input_context.mMouse->setEventCallback(this);
    }
  }
  else if(m_input_context.mMouse) {
    m_input_context.mMouse->setEventCallback(0);
    m_input_manager->destroyInputObject(m_input_context.mMouse);
    m_input_context.mMouse = 0;
  }
}

 // Ogre::FrameListener
bool Application::frameStarted(const Ogre::FrameEvent& value) {
  m_input_context.capture();
  return static_cast<bool>(m_frame_listener) && static_cast<bool>(m_frame_listener->m_started) ?
    m_frame_listener->m_started(value) : true;
}

bool Application::frameRenderingQueued(const Ogre::FrameEvent& value) {
  return static_cast<bool>(m_frame_listener) && static_cast<bool>(m_frame_listener->m_rendering_queued) ?
    m_frame_listener->m_rendering_queued(value) : true;
}

bool Application::frameEnded(const Ogre::FrameEvent& value) {
  return static_cast<bool>(m_frame_listener) && static_cast<bool>(m_frame_listener->m_ended) ?
    m_frame_listener->m_ended(value) : true;
}
 
// OIS::MouseListener  
bool Application::mouseMoved(const OIS::MouseEvent& value) {
  if(m_mouse_listner && m_mouse_listner->m_move)
    return m_mouse_listner->m_move(value);
  return true;
}

bool Application::mousePressed(const OIS::MouseEvent& value, OIS::MouseButtonID id) {
  if(m_mouse_listner && m_mouse_listner->m_pressed)
    return m_mouse_listner->m_pressed(value, id);
  return true;
}

bool Application::mouseReleased(const OIS::MouseEvent& value, OIS::MouseButtonID id ) {
  if(m_mouse_listner && m_mouse_listner->m_released)
    return m_mouse_listner->m_released(value, id);
  return true;
}
// OIS::MouseListener  
bool Application::keyPressed(const OIS::KeyEvent& value) {
  if(m_key_listner && m_key_listner->m_pressed)
    return m_key_listner->m_pressed(value);
  return true;
}

bool Application::keyReleased(const OIS::KeyEvent& value) {
  if(m_key_listner && m_key_listner->m_released)
    return m_key_listner->m_released(value);
  return true;
}

void Application::windowResized() {
  if(m_input_context.mMouse) {
    const OIS::MouseState& ms = m_input_context.mMouse->getMouseState();
    Ogre::RenderWindow* value = get_render_window();
    ms.width = value->getWidth();
    ms.height = value->getHeight();
  }
}

