#include "application.h"

#include <Ogre.h>
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreCamera.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreException.h>
#include <OgreEntity.h>

const Ogre::NameValuePairList Application::defparam = {
  {"FSAA", "0"},
  {"VSync", "No"},
  {"macAPICocoaUseNSView", "true"},
  {"FSAAH", "Quality"},
  {"contentScalingFactor", "1"},
  {"displayFrequency", "0"},
  {"macAPI", "cocoa"}
};

Application::Application(const Ogre::String& plugin_config,
      const Ogre::String& resource_config)
  : m_plugin_config(plugin_config)
  , m_resource_config(resource_config)
  , root( new Ogre::Root(m_plugin_config))
{
}

Application::~Application()
{
}

void Application::startApplication()
{
//    root = new Ogre::Root(plugin_config, Ogre::StringUtil::BLANK );

  loadPlugins();
  setRenderSystem();
  initializeRenderSystem();
  createRenderWindow();
  parseResourceFileConfiguration();
  initializeResources();

  createScene();

  root->startRendering();
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
    const Ogre::RenderSystemList &renderSystemList = root->getAvailableRenderers();
    if( renderSystemList.size() == 0 )
    {
        throw Ogre::Exception(Ogre::Exception::ERR_RENDERINGAPI_ERROR, "Sorry, no rendersystem was found.", __FILE__);
    }

    Ogre::RenderSystem *lRenderSystem = renderSystemList[ 0 ];
    Ogre::String renderSystemName = lRenderSystem->getName();
    Ogre::LogManager::getSingleton().logMessage( "Render System found: " + renderSystemName, Ogre::LML_NORMAL );
    root->setRenderSystem( lRenderSystem );
}

void Application::initializeRenderSystem()
{
    bool createAWindowAutomatically = false;
    Ogre::String windowTitle = "";
    Ogre::String customCapacities = "";
    root->initialise( createAWindowAutomatically, windowTitle, customCapacities );
}

void Application::createRenderWindow(const Ogre::String title,
      const unsigned int width, const unsigned int height,
      const bool full_screen, const Ogre::NameValuePairList* params)
{
#if 0
  Ogre::String windowTitle = title;
  unsigned int windowSizeX = width;
  unsigned int WindowSizeY = height;
  bool isFullscreen = full_screen;
#endif
#if 1
  Ogre::NameValuePairList additionalParameters;
  additionalParameters["FSAA"] = "0";
  additionalParameters["VSync"]="No";
  additionalParameters["macAPICocoaUseNSView"] = "true";
  additionalParameters["FSAAH"] = "Quality";
  additionalParameters["contentScalingFactor"] = "1";
  additionalParameters["displayFrequency"] = "0";
  additionalParameters["macAPI"] = "cocoa";
#endif
  renderWindow = root->createRenderWindow( title, width, height, full_screen, &additionalParameters );
}

void Application::initializeResources()
{
    // Set default mipmap level (note: some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps( 5 );
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
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
    sceneManager = root->createSceneManager(Ogre::ST_GENERIC);
    sceneManager->setAmbientLight( Ogre::ColourValue( 0.5, 0.5, 0.5 ) );

    camera = sceneManager->createCamera("PlayerCam");
    camera->setPosition(0, 0, 120);
    camera->setNearClipDistance( 5 );

    Ogre::Viewport* viewPort = renderWindow->addViewport( camera );
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

int main()
{
  try
  {
    Application application("plugins.cfg","resources-1.9.cfg");
    application.startApplication();
  }catch( Ogre::Exception &e )
  {
      std::cerr << "An exception has occured: " << e.getFullDescription() << std::endl;
  }
  return 0;
}