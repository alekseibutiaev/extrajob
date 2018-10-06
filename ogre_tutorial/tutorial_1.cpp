#include <iostream>
#include <exception>

#include <Ogre.h>
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreMath.h>
#include <OgreFrameListener.h>

#include "application.h"

class tutorial1 : public Application {
public:
  tutorial1();
  void createScene() override;
};

tutorial1::tutorial1() : Application("plugins.cfg","resources-1.9.cfg"){
}

void tutorial1::createScene()
{
  // get a pointer to the already created root
  Ogre::SceneManager* scnMgr = create_scene_manager();

  //register our scene with the RTSS
  //Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
  //shadergen->addSceneManager(scnMgr);

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
  get_render_window()->addViewport(cam);

  // finally something to render
  Ogre::Entity* ent = scnMgr->createEntity("ogrehead.mesh");
  Ogre::SceneNode* node = scnMgr->getRootSceneNode()->createChildSceneNode();
  node->setPosition(0,0,0);
  node->roll(Ogre::Degree(-60));
  node->attachObject(ent);
}

int main(int ac, char* av[]) {
  try {
    tutorial1 app;
    app.startApplication();
    return 0;
  }
  catch(const std::exception& e) {
    std::cout << "error: " << e.what() << std::endl;
  }
  return 1;
}
