#include <iostream>
#include <exception>

#include <Ogre.h>
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreMath.h>
#include <OgreFrameListener.h>

#include "application.h"

class baseapp : public Application {
public:
  baseapp();
};

baseapp::baseapp() : Application("plugins.cfg", "resources-1.9.cfg") {
}

int main(int ac, char* av[]) {
  try {
    baseapp app;
    app.startApplication();
    return 0;
  }
  catch(const std::exception& e) {
    std::cout << "error: " << e.what() << std::endl;
  }
  return 1;
}
