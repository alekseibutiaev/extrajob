
#include <iostream>
#include <exception>

#include <Ogre.h>
#include <OgreConfigFile.h>
#include <OgreMath.h>
#include <OgreApplicationContext.h>

class base_application : public OgreBites::ApplicationContext,
    public OgreBites::InputListener {
public:
  base_application();
  void setup();
  bool keyPressed(const OgreBites::KeyboardEvent& evt);
};

base_application::base_application() : OgreBites::ApplicationContext("OgreTutorialApp")
{
}

bool base_application::keyPressed(const OgreBites::KeyboardEvent& evt)
{
    if (evt.keysym.sym == OgreBites::SDLK_ESCAPE)
    {
        getRoot()->queueEndRendering();
    }
    return true;
}

void base_application::setup(void)
{
  Ogre::ConfigFile cf;
  cf.load("/home/aleksei/project/extrajob/ogre_tutorial/resources.cfg");
  Ogre::String sec, type, arch;
  // go through all specified resource groups
  Ogre::ConfigFile::SettingsBySection_::const_iterator seci;
  for(seci = cf.getSettingsBySection().begin(); seci != cf.getSettingsBySection().end(); ++seci) {
      sec = seci->first;
      const Ogre::ConfigFile::SettingsMultiMap& settings = seci->second;
      Ogre::ConfigFile::SettingsMultiMap::const_iterator i;

      // go through all resource paths
      for (i = settings.begin(); i != settings.end(); i++)
      {
          type = i->first;
          arch = Ogre::FileSystemLayer::resolveBundlePath(i->second);

          Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch, type, sec);
      }
  }
  // do not forget to call the base first
  OgreBites::ApplicationContext::setup();
  
  // register for input events
  addInputListener(this);

  // get a pointer to the already created root
  Ogre::Root* root = getRoot();
  Ogre::SceneManager* scnMgr = root->createSceneManager();

  // register our scene with the RTSS
  Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
  shadergen->addSceneManager(scnMgr);

  // without light we would just get a black screen    
  Ogre::Light* light = scnMgr->createLight("MainLight");
  Ogre::SceneNode* lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
  lightNode->setPosition(0, 10, 15);
  lightNode->attachObject(light);

  // also need to tell where we are
  Ogre::SceneNode* camNode = scnMgr->getRootSceneNode()->createChildSceneNode();
  camNode->setPosition(0, 0, 100);
  camNode->lookAt(Ogre::Vector3(0, 0, -1), Ogre::Node::TS_PARENT);

  // create the camera
  Ogre::Camera* cam = scnMgr->createCamera("myCam");
  cam->setNearClipDistance(5); // specific to this sample
  cam->setAutoAspectRatio(true);
  camNode->attachObject(cam);
  camNode->setPosition(0, 50, 300);

  // and tell it to render into the main window
  getRenderWindow()->addViewport(cam);

  // finally something to render
  Ogre::Entity* ent = scnMgr->createEntity("ogrehead.mesh");
  Ogre::SceneNode* node = scnMgr->getRootSceneNode()->createChildSceneNode();
  node->setPosition(0,0,0);
  node->roll(Ogre::Degree(-60));
  node->attachObject(ent);
}

int main(int ac, char* av[]) {
  try {
    base_application app;
    app.initApp();
    app.getRoot()->startRendering();
    app.closeApp();
  }
  catch(const std::exception& e) {
    std::cout << "error: " << e.what() << std::endl;
  }

  return 0;
}