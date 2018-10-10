#include <cmath>

#include <memory>
#include <iostream>
#include <exception>
#include <type_traits>
#include <chrono>

#include <Ogre.h>
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreMath.h>
#include <OgreFrameListener.h>

#include <OISMouse.h>
#include <OISKeyboard.h>

#include "application.h"

namespace {
  template<typename T, std::size_t N>
  std::size_t array_size(T(&)[N]) {
    return N;
  }
} /* namespace */

namespace {

  void sample_material() {
    Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(
      "Test/ColourTest", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
      material->getTechnique(0)->getPass(0)->setVertexColourTracking(Ogre::TVC_AMBIENT);
  }

  void create_mash(const Ogre::String& name, const Ogre::String& group, Ogre::VertexData* vd, std::size_t count,
      Ogre::HardwareIndexBufferSharedPtr ibuf, const Ogre::AxisAlignedBox& box, const double radius) {
      /// Create the mesh via the MeshManager
    Ogre::MeshPtr msh = Ogre::MeshManager::getSingleton().createManual(name, group);
    msh->sharedVertexData = vd;
    /// Create one submesh
    Ogre::SubMesh* sub = msh->createSubMesh();

    /// Set parameters of the submesh
    sub->useSharedVertices = true;
    sub->indexData->indexBuffer = ibuf;
    sub->indexData->indexCount = count;
    sub->indexData->indexStart = 0;

    /// Set bounding information (for culling)
    msh->_setBounds(box);
    msh->_setBoundingSphereRadius(radius);

    /// Notify -Mesh object that it has been loaded
    msh->load();
  }

  template<typename T, std::size_t N>
  void slot_machine_faces(T(&faces)[N][2][3], const std::size_t index) {
    static const std::size_t dev = N * 2;
    const T i = index * 2;
    faces[index][0][0] = i;
    faces[index][0][1] = (i + 3) % dev;
    faces[index][0][2] = i + 1;
    faces[index][1][0] = i;
    faces[index][1][1] = (i + 2) % dev;
    faces[index][1][2] = (i + 3) % dev;
  }

  inline Ogre::Vector3 create_normal(const Ogre::Vector3& a, const Ogre::Vector3& b,
      const Ogre::Vector3& c) {
    Ogre::Vector3 res = (c - a).crossProduct(b - a);
    res.normalise();
    return res;
    
  }

  template<std::size_t face_count>
  void apply_normal(Ogre::Vector3(&vertices)[face_count][2][2], std::size_t i) {
    if(0 != i) {
      const std::size_t pi = i - 1;
      vertices[pi][0][1] = create_normal(vertices[pi][0][0], vertices[i][0][0], vertices[pi][1][0]);
      vertices[pi][1][1] = create_normal(vertices[pi][1][0], vertices[pi][0][0], vertices[i][1][0]);
      if(1 == face_count - i) {
        vertices[i][0][1] = create_normal(vertices[i][0][0], vertices[0][0][0], vertices[i][1][0]);
        vertices[i][1][1] = create_normal(vertices[i][1][0], vertices[i][0][0], vertices[0][1][0]);
      }
    }
  }

  template<std::size_t face_count, typename face_index_t = unsigned short>
  void slot_machine_wheel(const double radius, const double width) {
    static_assert(std::is_integral<face_index_t>::value && (sizeof(face_index_t) == sizeof(short) ||
      sizeof(face_index_t) == sizeof(int)), "face_index_t must by integer and 16 or 32 bit");

    const Ogre::HardwareIndexBuffer::IndexType index_type = sizeof(face_index_t) == sizeof(unsigned short) ?
      Ogre::HardwareIndexBuffer::IT_16BIT : Ogre::HardwareIndexBuffer::IT_32BIT;
    const double step = 2 * M_PI / face_count;
    const double xpos = 0;

    Ogre::Vector3 vertices[face_count][2][2];
    face_index_t faces[face_count][2][3];

    Ogre::RGBA colours[face_count][2];
    Ogre::RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();

    double rad = -M_PI;
    for(std::size_t i = 0; i < face_count; ++i) {
      const double x = std::cos(rad);
      const double y = std::sin(rad);
      rad += step;
      {
        const int dd = i % 7; 
        rs->convertColourValue(Ogre::ColourValue(dd & 1, dd & 2, dd & 4), &colours[i][0]);
        rs->convertColourValue(Ogre::ColourValue(dd & 4, dd & 2, dd & 1), &colours[i][1]);
      }
      slot_machine_faces(faces, i);

      vertices[i][0][0] = Ogre::Vector3(xpos, y * radius, x * radius);
      vertices[i][1][0] = Ogre::Vector3(xpos + width, y * radius, x * radius);
      apply_normal(vertices, i);
    }

    Ogre::VertexData* vd = new Ogre::VertexData();
    vd->vertexCount = face_count * 2;

    /// Create declaration (memory format) of vertex data
    Ogre::VertexDeclaration* decl = vd->vertexDeclaration;
    size_t offset = 0;
    // 1st buffer
    decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    /// Allocate vertex buffer of the requested number of vertices (vertexCount) 
    /// and bytes per vertex (offset)
    Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
      offset, vd->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    /// Upload the vertex data to the card
    vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);
    vd->vertexBufferBinding->setBinding(0, vbuf);

    // 2ed buffer
    offset = 0;
    decl->addElement(1, offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_COLOUR);
    /// Allocate vertex buffer of the requested number of vertices (vertexCount) 
    /// and bytes per vertex (offset)
    vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
      offset, vd->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    /// Upload the vertex data to the card
    vbuf->writeData(0, vbuf->getSizeInBytes(), colours, true);
    /// Set vertex buffer binding so buffer 1 is bound to our colour buffer
    vd->vertexBufferBinding->setBinding(1, vbuf);

    /// Allocate index buffer of the requested number of vertices (ibufCount) 
    Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().
          createIndexBuffer(index_type, face_count * 2 * 3, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    /// Upload the index data to the card
    ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

    create_mash("SpotWheel", "General", vd, face_count * 2 * 3, ibuf,
      Ogre::AxisAlignedBox(0, -radius, -radius, width, radius, radius),
      Ogre::Math::Sqrt(radius * radius + (width/2) * (width/2)));

  }

  void createColourCube()
  {
    /// Create the mesh via the MeshManager
    Ogre::MeshPtr msh = Ogre::MeshManager::getSingleton().createManual("ColourCube", "General");

    /// Create one submesh
    Ogre::SubMesh* sub = msh->createSubMesh();

    const float sqrt13 = 0.577350269f; /* sqrt(1/3) */

    /// Define the vertices (8 vertices, each have 3 floats for position and 3 for normal)
    const size_t nVertices = 8;
    const size_t vbufCount = 3*2*nVertices;
    float vertices[vbufCount] = {
            -100.0,100.0,-100.0,        //0 position A
            -sqrt13,sqrt13,-sqrt13,     //0 normal
            100.0,100.0,-100.0,         //1 position B
            sqrt13,sqrt13,-sqrt13,      //1 normal
            100.0,-100.0,-100.0,        //2 position C               A-----B
            sqrt13,-sqrt13,-sqrt13,     //2 normal                  /|    /|
            -100.0,-100.0,-100.0,       //3 position D             / |   / |
            -sqrt13,-sqrt13,-sqrt13,    //3 normal                /  D--/--C
            -100.0,100.0,100.0,         //4 position E           /  /  /  / 
            -sqrt13,sqrt13,sqrt13,      //4 normal              E--/--F  / 
            100.0,100.0,100.0,          //5 position F          | /   | /
            sqrt13,sqrt13,sqrt13,       //5 normal              |/    |/
            100.0,-100.0,100.0,         //6 position G          H-----G
            sqrt13,-sqrt13,sqrt13,      //6 normal
            -100.0,-100.0,100.0,        //H position 7
            -sqrt13,-sqrt13,sqrt13,     //7 normal
    };

    Ogre::RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
    Ogre::RGBA colours[nVertices];
    Ogre::RGBA *pColour = colours;
    // Use render system to convert colour value since colour packing varies
    rs->convertColourValue(Ogre::ColourValue(1.0,0.0,0.0), pColour++); //0 colour
    rs->convertColourValue(Ogre::ColourValue(1.0,1.0,0.0), pColour++); //1 colour
    rs->convertColourValue(Ogre::ColourValue(0.0,1.0,0.0), pColour++); //2 colour
    rs->convertColourValue(Ogre::ColourValue(0.0,0.0,0.0), pColour++); //3 colour
    rs->convertColourValue(Ogre::ColourValue(1.0,0.0,1.0), pColour++); //4 colour
    rs->convertColourValue(Ogre::ColourValue(1.0,1.0,1.0), pColour++); //5 colour
    rs->convertColourValue(Ogre::ColourValue(0.0,1.0,1.0), pColour++); //6 colour
    rs->convertColourValue(Ogre::ColourValue(0.0,0.0,1.0), pColour++); //7 colour

    /// Define 12 triangles (two triangles per cube face)
    /// The values in this table refer to vertices in the above table
    const size_t ibufCount = 36;
    unsigned short faces[ibufCount] = {
            0,2,3,
            0,1,2,
            1,6,2,
            1,5,6,
            4,6,5,
            4,7,6,
            0,7,4,
            0,3,7,
            0,5,1,
            0,4,5,
            2,7,3,
            2,6,7
    };

    /// Create vertex data structure for 8 vertices shared between submeshes
    msh->sharedVertexData = new Ogre::VertexData();
    msh->sharedVertexData->vertexCount = nVertices;

    /// Create declaration (memory format) of vertex data
    Ogre::VertexDeclaration* decl = msh->sharedVertexData->vertexDeclaration;
    size_t offset = 0;
    // 1st buffer
    decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    /// Allocate vertex buffer of the requested number of vertices (vertexCount) 
    /// and bytes per vertex (offset)
    Ogre::HardwareVertexBufferSharedPtr vbuf = 
        Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
        offset, msh->sharedVertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    /// Upload the vertex data to the card
    vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);

    /// Set vertex buffer binding so buffer 0 is bound to our vertex buffer
    Ogre::VertexBufferBinding* bind = msh->sharedVertexData->vertexBufferBinding; 
    bind->setBinding(0, vbuf);

    // 2nd buffer
    offset = 0;
    decl->addElement(1, offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_COLOUR);
    /// Allocate vertex buffer of the requested number of vertices (vertexCount) 
    /// and bytes per vertex (offset)
    vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
        offset, msh->sharedVertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    /// Upload the vertex data to the card
    vbuf->writeData(0, vbuf->getSizeInBytes(), colours, true);

    /// Set vertex buffer binding so buffer 1 is bound to our colour buffer
    bind->setBinding(1, vbuf);

    /// Allocate index buffer of the requested number of vertices (ibufCount) 
    Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().
        createIndexBuffer(
        Ogre::HardwareIndexBuffer::IT_16BIT, 
        ibufCount, 
        Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    /// Upload the index data to the card
    ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

    /// Set parameters of the submesh
    sub->useSharedVertices = true;
    sub->indexData->indexBuffer = ibuf;
    sub->indexData->indexCount = ibufCount;
    sub->indexData->indexStart = 0;

    /// Set bounding information (for culling)
    msh->_setBounds(Ogre::AxisAlignedBox(-100,-100,-100,100,100,100));
    msh->_setBoundingSphereRadius(Ogre::Math::Sqrt(3*100*100));

    /// Notify -Mesh object that it has been loaded
    msh->load();
  }

  void create_test() {
    struct vertices_t {
      Ogre::Vector3 verts;
      Ogre::Vector3 normal;
      Ogre::Vector2 text;
    };

    vertices_t vertices[] = {
      { {   0.0f,   0.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
      { {  50.0f,   0.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.0f } },
      { { 100.0f,   0.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
      { {   0.0f,  50.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.2f } },
      { {  50.0f,  50.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.2f } },
      { { 100.0f,  50.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.2f } },
      { {   0.0f, 100.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.4f } },
      { {  50.0f, 100.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.4f } },
      { { 100.0f, 100.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.4f } },
      { {   0.0f, 150.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.6f } },
      { {  50.0f, 150.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.6f } },
      { { 100.0f, 150.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.6f } },
      { {   0.0f, 200.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.8f } },
      { {  50.0f, 200.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.8f } },
      { { 100.0f, 200.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.8f } },
      { {   0.0f, 250.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.999f } },
      { {  50.0f, 250.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.999f } },
      { { 100.0f, 250.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.999f } },
    };

    Ogre::VertexDeclaration* mDecl;
    Ogre::PatchMeshPtr mPatch;
		// specify a vertex format declaration for our patch: 3 floats for position, 3 floats for normal, 2 floats for UV
    mDecl = Ogre::HardwareBufferManager::getSingleton().createVertexDeclaration();
    mDecl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    mDecl->addElement(0, sizeof(float) * 3, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
    mDecl->addElement(0, sizeof(float) * 6, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
    
		// create a patch mesh using vertices and declaration
    mPatch = Ogre::MeshManager::getSingleton().createBezierPatch("patch", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
      (float*)vertices, mDecl, 3, 6, -1, -1,  Ogre::PatchSurface::VS_BOTH);

    mPatch->setSubdivision(0);   // start at 0 detail

/*
		// create a patch entity from the mesh, give it a material, and attach it to the origin
    Ogre::Entity* ent = mSceneMgr->createEntity("patch", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		ent->setMaterialName("casion/wheel");
    mSceneMgr->getRootSceneNode()->attachObject(ent);

		// save the main pass of the material so we can toggle wireframe on it
		mPatchPass = ent->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0);
*/

  }

} /* namespace */

class tutorial4
    : public Application {
public:
  tutorial4();
  void createScene() override;
private:
  bool mouse_moved(const OIS::MouseEvent& value);
	bool mouse_pressed(const OIS::MouseEvent& value, OIS::MouseButtonID id);
	bool mouse_released(const OIS::MouseEvent& value, OIS::MouseButtonID id);
  bool key_pressed(const OIS::KeyEvent& value);
	bool key_released(const OIS::KeyEvent& value);
  bool frame_startted(const Ogre::FrameEvent& value);
private:
  Ogre::Camera* camera = 0;
  Ogre::SceneNode* sw = 0;
  Ogre::Vector3 rotate;
  std::chrono::system_clock::time_point m_previous;
  int x = 0;
  int y = 0;
  int z = 0;
};

tutorial4::tutorial4() : Application("plugins.cfg", "resources-1.9.cfg") {
  const std::string s = OGRE_HOME;
  start_input();
  key_listener_ptr kl = key_listener_ptr(new key_listener_ptr::element_type());
  kl->m_pressed = [&](const OIS::KeyEvent& value){return key_pressed(value);};
  kl->m_released = [&](const OIS::KeyEvent& value){return key_released(value);};
  set_key_listener(std::move(kl));
  mouse_listener_ptr ml = mouse_listener_ptr(new mouse_listener_ptr::element_type());
  ml->m_move = [&](const OIS::MouseEvent& value){return mouse_moved(value);};
  ml->m_pressed = [&](const OIS::MouseEvent& value, OIS::MouseButtonID id){return mouse_pressed(value, id);};
  ml->m_released = [&](const OIS::MouseEvent& value, OIS::MouseButtonID id){return mouse_released(value, id);};
  set_mouse_listener(std::move(ml));
  frame_listener_ptr fl = frame_listener_ptr(new frame_listener_ptr::element_type());
  fl->m_started = [&](const Ogre::FrameEvent& value){return frame_startted(value);};
  set_frame_listener(std::move(fl));
}

void tutorial4::createScene()
{
  Ogre::SceneManager* sceneManager = create_scene_manager();
  sceneManager->setAmbientLight(Ogre::ColourValue(1.0, 1.0, 1.0));

  camera = sceneManager->createCamera("PlayerCam");
  camera->setPosition(0, 0, 300);
  camera->lookAt(Ogre::Vector3(0, 1, 0)/*, Ogre::Node::TransformSpace::TS_WORLD*/);

  camera->setNearClipDistance( 5 );
  
  Ogre::Viewport* viewPort = get_render_window()->addViewport( camera );
  viewPort->setBackgroundColour(Ogre::ColourValue(0.1, 0.1, 0.1));
  camera->setAspectRatio(Ogre::Real(viewPort->getActualWidth())/Ogre::Real(viewPort->getActualHeight()));
  camera->setProjectionType(Ogre::ProjectionType::PT_ORTHOGRAPHIC);

  // Create a Light and set its position
  Ogre::Light* light = sceneManager->createLight("MainLight");
  light->setPosition(0.0f, 0.0f, 0.120f);

  Ogre::SceneNode* node;
  sample_material();
  createColourCube();
  createColourCube();

#if 0
  slot_machine_wheel<36>(200.0, 125.6);
  Ogre::Entity* thisEntity = sceneManager->createEntity("sw", "SpotWheel");
  thisEntity->setMaterialName("Test/ColourTest");
  node = sceneManager->getRootSceneNode()->createChildSceneNode();
  node->setPosition(0, 0, 0);
  //  node->yaw(Ogre::Radian(1.0));
  //node->pitch(Ogre::Radian(1.0));
  node->attachObject(thisEntity);
  sw = node;

  /*Ogre::Entity**/ thisEntity = sceneManager->createEntity("sw1", "SpotWheel");
  thisEntity->setMaterialName("Test/ColourTest");
  node = sceneManager->getRootSceneNode()->createChildSceneNode();
  node->setPosition(-125.6, 0, 0);
  //  node->yaw(Ogre::Radian(1.0));
  //node->pitch(Ogre::Radian(1.0));
  node->attachObject(thisEntity);
#endif
#if 0
  /*Ogre::Entity**/ thisEntity = sceneManager->createEntity("cc", "ColourCube");
  thisEntity->setMaterialName("Test/ColourTest");
  /*Ogre::SceneNode* */node = sceneManager->getRootSceneNode()->createChildSceneNode();
  node->setPosition(0, 0, -300);
  node->attachObject(thisEntity);
  node->yaw(Ogre::Radian(1.0));
  node->pitch(Ogre::Radian(1.0));
#endif
#if 1
  try {
    create_test();
    // create a patch entity from the mesh, give it a material, and attach it to the origin
    Ogre::Entity* ent = sceneManager->createEntity("Patch", "patch");
    ent->setMaterialName("casino/wheel");
    node = sceneManager->getRootSceneNode()->createChildSceneNode();
    node->attachObject(ent);
    sw = node;
    // save the main pass of the material so we can toggle wireframe on it
    //Ogre::Pass* mPatchPass = ent->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0);
  }
  catch (const std::exception& e) {
    std::cout << e.what() << std::endl; 
  }
#endif

}

bool tutorial4::mouse_moved(const OIS::MouseEvent& value) {
  return true;
}

bool tutorial4::mouse_pressed(const OIS::MouseEvent& value, OIS::MouseButtonID id ) {
  return true;
}

bool tutorial4::mouse_released( const OIS::MouseEvent& value, OIS::MouseButtonID id ) {
  //set_mouse_listener(mouse_listener_ptr());
  return true;
}

bool tutorial4::key_pressed(const OIS::KeyEvent& value) {
  const float step = 10.0;
  const int s = 5;
  switch(value.key) {
    case OIS::KeyCode::KC_W :
      camera->move(Ogre::Vector3(0.0, 0.0, step));
      break;
    case OIS::KeyCode::KC_S :
      camera->move(Ogre::Vector3(0.0, 0.0, -step));
      break;
    case OIS::KeyCode::KC_A :
      camera->move(Ogre::Vector3(-step, 0.0, 0.0));
      break;
    case OIS::KeyCode::KC_D :
      camera->move(Ogre::Vector3(step, 0.0, 0.0));
      break;
    case OIS::KeyCode::KC_Q :
      camera->move(Ogre::Vector3(0.0, -step, 0.0));
      break;
    case OIS::KeyCode::KC_E :
      camera->move(Ogre::Vector3(0.0, step, 0.0));
      break;
    case OIS::KeyCode::KC_P :
      if(0 >= x)
        x += s;
      break;
    case OIS::KeyCode::KC_O :
      if(0 <= x)
        x -= s;
      break;
    case OIS::KeyCode::KC_I :
      if(0 >= y)
        y += s;
      break;
    case OIS::KeyCode::KC_U :
      if(0 <= y)
        y -= s;
      break;
    case OIS::KeyCode::KC_L :
      if(0 >= z)
        z += s;
      break;
    case OIS::KeyCode::KC_K :
      if(0 <= z)
        z -= s;
      break;
    default:
      break;
  };
  return true;
}

bool tutorial4::key_released(const OIS::KeyEvent& value) {
  //set_key_listener(key_listener_ptr());
  return true;
}

bool tutorial4::frame_startted(const Ogre::FrameEvent& value) {
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  std::chrono::system_clock::duration d = now - m_previous;
  auto c = d.count();
  if(c > 100000000){
    if( (true || 0 != z || 0 != y || 0 != x) ) {
      sw->roll(Ogre::Degree(z));
      sw->pitch(Ogre::Degree(x));
      sw->yaw(Ogre::Degree(y));
    }
    m_previous = now;
  }
  return true;
}

  int main(int ac, char* av[]) {
  try {
    tutorial4 app;
    app.startApplication();
    return 0;
  }
  catch(const std::exception& e) {
    std::cout << "error: " << e.what() << std::endl;
  }
  return 1;
}


#if 0
        // Create the mesh:
        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(meshName,Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::SubMesh* subMesh = mesh->createSubMesh();

        Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();
        Ogre::VaoManager* vaoManager = renderSystem->getVaoManager();

        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_NORMAL));
        //    //uvs
        //    for(int i=0; i<currentBuffer->uvSetCount; i++){
        //        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES));
        //    }

        int vertexCount = currentBuffer->vertexs.size();

        size_t vertexSize = vaoManager->calculateVertexSize(vertexElements);

        Ogre::Real* vertexData = static_cast<Ogre::Real*>( OGRE_MALLOC_SIMD( vertexSize * vertexCount, Ogre::MEMCATEGORY_GEOMETRY ) );
        Ogre::Real* pVertex = reinterpret_cast<Ogre::Real*>(vertexData);


        Ogre::Vector3 minBB(Ogre::Vector3::UNIT_SCALE*FLT_MAX);
        Ogre::Vector3 maxBB(Ogre::Vector3::UNIT_SCALE*-FLT_MAX);

        for(int i=0; i<vertexCount; i++)
        {
            Ogre::Vector3 pos = convertToYup(Ogre::Vector3(currentBuffer->vertexs.at(i).co[0],currentBuffer->vertexs.at(i).co[1],currentBuffer->vertexs.at(i).co[2]));
            //transform to Y-up
            *pVertex++ = pos.x;
            *pVertex++ = pos.y;
            *pVertex++ = pos.z;

            Ogre::Vector3 norm = convertToYup(Ogre::Vector3(currentBuffer->vertexs.at(i).no[0],currentBuffer->vertexs.at(i).no[1],currentBuffer->vertexs.at(i).no[2])).normalisedCopy();
            //Normals
            *pVertex++ = norm.x;
            *pVertex++ = norm.y;
            *pVertex++ = norm.z;

            //        //uvs
            //        for(int j=0; j<currentBuffer->uvSetCount; j++){
            //            *pVertex++ = currentBuffer->vertexs.at(i).uv[j].x;
            //            *pVertex++ = 1.0-currentBuffer->vertexs.at(i).uv[j].y;
            //        }

            //Calc Bounds
            minBB.makeFloor(pos);
            maxBB.makeCeil(pos);

        }

        Ogre::VertexBufferPackedVec vertexBuffers;

        Ogre::VertexBufferPacked *pVertexBuffer = vaoManager->createVertexBuffer( vertexElements, vertexCount, Ogre::BT_IMMUTABLE, vertexData, true );
        vertexBuffers.push_back(pVertexBuffer);



        //Indices

        unsigned int iBufSize = currentBuffer->triangles.size() * 3;

        static const unsigned short index16BitClamp = (0xFFFF) - 1;

        //Index buffer
        Ogre::IndexBufferPacked::IndexType buff_type = (iBufSize > index16BitClamp) ?
                    Ogre::IndexBufferPacked::IT_32BIT : Ogre::IndexBufferPacked::IT_16BIT;

        //Build index items
        bool using32 = buff_type == Ogre::IndexBufferPacked::IT_32BIT;

        Ogre::uint32 *indices32 = 0;
        Ogre::uint16 *indices16 = 0;

        if (!using32)
            indices16 = reinterpret_cast<Ogre::uint16*>( OGRE_MALLOC_SIMD(sizeof(Ogre::uint16) * iBufSize, Ogre::MEMCATEGORY_GEOMETRY ) );
        else
            indices32 = reinterpret_cast<Ogre::uint32*>( OGRE_MALLOC_SIMD(sizeof(Ogre::uint32) * iBufSize, Ogre::MEMCATEGORY_GEOMETRY ) );

        for (unsigned int cur = 0; cur < currentBuffer->triangles.size(); cur++)
        {
            const yTriangleIndex& currentTriangle = currentBuffer->triangles.at(cur);
            for(unsigned int i=0; i<3; i++){
                if(using32)
                    *indices32++ = (Ogre::uint32)currentTriangle.index[i];
                else
                    *indices16++ = (Ogre::uint16)currentTriangle.index[i];
            }
        }

        Ogre::IndexBufferPacked *indexBuffer;
        if(using32){
            indexBuffer = vaoManager->createIndexBuffer( buff_type, iBufSize, Ogre::BT_IMMUTABLE, indices32, true );
        }
        else{
            indexBuffer = vaoManager->createIndexBuffer( buff_type, iBufSize, Ogre::BT_IMMUTABLE, indices16, true );
        }


        Ogre::VertexArrayObject *vao = vaoManager->createVertexArrayObject(
                    vertexBuffers, indexBuffer, Ogre::v1::RenderOperation::OT_TRIANGLE_LIST );

        subMesh->mVao[0].push_back( vao );
        subMesh->mVao[1].push_back( vao );


        Ogre::Aabb bounds;
        bounds.merge(minBB);
        bounds.merge(maxBB);
        mesh->_setBounds(bounds,false);
        mesh->_setBoundingSphereRadius(bounds.getRadius());

        return mesh;
#endif