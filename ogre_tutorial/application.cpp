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

bool Application::mouse_listener::mouseMoved(const OIS::MouseEvent& value) {
  return static_cast<bool>(m_move) ? m_move(value) : true;
}

bool Application::mouse_listener::mousePressed(const OIS::MouseEvent& value, OIS::MouseButtonID id) {
  return static_cast<bool>(m_pressed) ? m_pressed(value, id) : true;
}

bool Application::mouse_listener::mouseReleased(const OIS::MouseEvent& value, OIS::MouseButtonID id) {
  return static_cast<bool>(m_released) ? m_released(value, id) : true;
}

bool Application::key_listener::keyPressed(const OIS::KeyEvent& value) {
  return static_cast<bool>(m_pressed) ? m_pressed(value) : true;
}

bool Application::key_listener::keyReleased(const OIS::KeyEvent& value) {
  return static_cast<bool>(m_released) ? m_released(value) : true;
}


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
  m_root->startRendering();
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

void Application::start_input(const OIS::ParamList& value) {
  static const char* name_win = "WINDOW";
  OIS::ParamList paramlist = value;
  std::size_t handle;
  Ogre::RenderWindow* win = get_render_window();
  win->getCustomAttribute(name_win, &handle);
  paramlist.insert({name_win, std::to_string(handle)});
  m_input_manager = std::move(input_manager_ptr(OIS::InputManager::createInputSystem(paramlist),
    &OIS::InputManager::destroyInputSystem));
  windowResized(win);
}

void Application::stop_input() {
  set_key_listner(key_listener_ptr());
  set_mouse_listner(mouse_listener_ptr());
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

void Application::set_key_listner(key_listener_ptr&& value) {
  assert(static_cast<bool>(m_input_manager));
  m_key_listner = std::move(value);  
  if(static_cast<bool>(m_key_listner)) {
    if(0 != (m_input_context.mKeyboard = static_cast<OIS::Keyboard*>(m_input_manager->createInputObject(OIS::OISKeyboard, true)))){
      m_input_context.mKeyboard->setEventCallback(this);
      m_input_context.mKeyboard->capture();
    }
  }
  else if(0 != m_input_context.mKeyboard) {
    m_input_context.mKeyboard->setEventCallback(0);  
    m_input_manager->destroyInputObject(m_input_context.mKeyboard);
  }
}

void Application::set_mouse_listner(mouse_listener_ptr&& value){
  assert(static_cast<bool>(m_input_manager));
  m_mouse_listner = std::move(value);
  if(static_cast<bool>(m_mouse_listner)) {
    if(0 != (m_input_context.mMouse = static_cast<OIS::Mouse*>(m_input_manager->createInputObject(OIS::OISMouse, true)))) {
      m_input_context.mMouse->setEventCallback(this);
      m_input_context.mMouse->capture();
    }
  }
  else if(m_input_context.mMouse) {
    m_input_context.mMouse->setEventCallback(0);
    m_input_manager->destroyInputObject(m_input_context.mMouse);
  }
}
// OIS::MouseListener  
bool Application::mouseMoved(const OIS::MouseEvent& value) {
  if(m_mouse_listner)
    return m_mouse_listner->mouseMoved(value);
  return true;
}

bool Application::mousePressed(const OIS::MouseEvent& value, OIS::MouseButtonID id) {
  if(m_mouse_listner)
    return m_mouse_listner->mousePressed(value, id);
  return true;
}

bool Application::mouseReleased(const OIS::MouseEvent& value, OIS::MouseButtonID id ) {
  if(m_mouse_listner)
    return m_mouse_listner->mouseReleased(value, id);
  return true;
}
// OIS::MouseListener  
bool Application::keyPressed(const OIS::KeyEvent& value) {
  if(m_key_listner)
    return m_key_listner->keyPressed(value);
  return true;
}

bool Application::keyReleased(const OIS::KeyEvent& value) {
  if(m_key_listner)
    return m_key_listner->keyReleased(value);
  return true;
}

void Application::windowResized(Ogre::RenderWindow* value) {
  if(m_input_context.mMouse) {
    const OIS::MouseState& ms = m_input_context.mMouse->getMouseState();
    ms.width = value->getWidth();
    ms.height = value->getHeight();
  }

}

