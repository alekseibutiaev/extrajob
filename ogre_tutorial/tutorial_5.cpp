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

  void create_mash(const Ogre::String& name, const Ogre::String& group, Ogre::VertexData* vd,
      Ogre::HardwareIndexBufferSharedPtr ibuf, const Ogre::AxisAlignedBox& box, const double radius) {
      /// Create the mesh via the MeshManager
    Ogre::MeshPtr msh = Ogre::MeshManager::getSingleton().createManual(name, group);
    msh->sharedVertexData = vd;
    /// Create one submesh
    Ogre::SubMesh* sub = msh->createSubMesh();

    /// Set parameters of the submesh
    sub->useSharedVertices = true;
    sub->indexData->indexBuffer = ibuf;
    sub->indexData->indexCount = ibuf->getNumIndexes();
    sub->indexData->indexStart = 0;

    /// Set bounding information (for culling)
    msh->_setBounds(box);
    msh->_setBoundingSphereRadius(radius);

    /// Notify -Mesh object that it has been loaded
    msh->load();
    msh->touch();
  }

  template<typename T, std::size_t N>
  void slot_machine_faces(T(&faces)[N][2][3], const std::size_t index) {
    if(index >= N)
      return;
    const T i = index * 2;
    faces[index][0][0] = i;
    faces[index][0][1] = i + 3;
    faces[index][0][2] = i + 1;
    faces[index][1][0] = i;
    faces[index][1][1] = i + 2;
    faces[index][1][2] = i + 3;
  }

  inline Ogre::Vector3 create_normal(const Ogre::Vector3& a, const Ogre::Vector3& b,
      const Ogre::Vector3& c) {
    Ogre::Vector3 res = (b - a).crossProduct(c - a);
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

  template<std::size_t face_count>
  void create_text(const std::size_t i, Ogre::Vector2(&cp)[face_count][2]) {
    static const float step = 1.0f / (face_count - 1);
    cp[i][0].x = 0.0f;
    cp[i][1].y = cp[i][0].y = i * step;
    cp[i][1].x = 1.0f;
  }

  template<std::size_t face_count, typename face_index_t = unsigned short>
  void slot_machine_wheel_text(const float radius, const float width) {
    static_assert(std::is_integral<face_index_t>::value && (sizeof(face_index_t) == sizeof(short) ||
      sizeof(face_index_t) == sizeof(int)), "face_index_t must by integer and 16 or 32 bit");

    const std::size_t vertice_count = face_count + 1;

    const Ogre::HardwareIndexBuffer::IndexType index_type = sizeof(face_index_t) == sizeof(unsigned short) ?
      Ogre::HardwareIndexBuffer::IT_16BIT : Ogre::HardwareIndexBuffer::IT_32BIT;
    const float step = 2 * M_PI / face_count;
    const float xpos = 0;

    Ogre::Vector3 vertices[vertice_count][2][2];
    Ogre::Vector2 text[vertice_count][2];
    face_index_t faces[face_count][2][3];

    double rad = -M_PI;
    for(std::size_t i = 0; i < vertice_count; ++i) {
      rad += step;
      slot_machine_faces(faces, i);
      vertices[i][0][0] = vertices[i][1][0] = Ogre::Vector3(xpos, std::sin(rad) * radius, std::cos(rad) * radius);
      vertices[i][1][0].x += width;
      create_text(i, text);
      apply_normal(vertices, i);
    }

    Ogre::VertexData* vd = new Ogre::VertexData();
    vd->vertexCount = (vertice_count) * 2;

    /// Create declaration (memory format) of vertex data
    Ogre::VertexDeclaration* decl = vd->vertexDeclaration;
    // 1st buffer
    decl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    decl->addElement(0, sizeof(Ogre::Vector3), Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
    /// Allocate vertex buffer of the requested number of vertices (vertexCount) 
    /// and bytes per vertex (offset)
    Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
      sizeof(Ogre::Vector3) * 2, vd->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    /// Upload the vertex data to the card
    vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);
    vd->vertexBufferBinding->setBinding(0, vbuf);

    // 2ed buffer
    decl->addElement(1, 0, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
    /// Allocate vertex buffer of the requested number of vertices (vertexCount) 
    /// and bytes per vertex (offset)
    Ogre::HardwareVertexBufferSharedPtr vbuf_text = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
      sizeof(Ogre::Vector2), vd->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    vbuf_text->writeData(0, vbuf_text->getSizeInBytes(), text, true);
    /// Set vertex buffer binding so buffer 1 is bound to our colour buffer
    vd->vertexBufferBinding->setBinding(1, vbuf_text);

    /// Allocate index buffer of the requested number of vertices (ibufCount) 
    Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().
          createIndexBuffer(index_type, face_count * 2 * 3, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    /// Upload the index data to the card
    ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

    create_mash("SpotWheelText", "General", vd, ibuf,
      Ogre::AxisAlignedBox(0, -radius, -radius, width, radius, radius),
      Ogre::Math::Sqrt(radius * radius + (width/2) * (width/2)));
  }

  void create_test() {

    struct vertices_t {
      Ogre::Vector3 verts;
      Ogre::Vector3 normal;
      Ogre::Vector2 text;
    };

    vertices_t vertices[] = {
      { {   0.0f,   0.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }, // 0
      { {  50.0f,   0.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.0f } }, // 1
      { { 100.0f,   0.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }, // 2
      { {   0.0f,  50.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.2f } }, // 3
      { {  50.0f,  50.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.2f } }, // 4
      { { 100.0f,  50.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.2f } }, // 5
      { {   0.0f, 100.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.4f } }, // 6
      { {  50.0f, 100.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.4f } }, // 7
      { { 100.0f, 100.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.4f } }, // 8
      { {   0.0f, 150.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.6f } }, // 9
      { {  50.0f, 150.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.6f } }, // 10
      { { 100.0f, 150.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.6f } }, // 11
      { {   0.0f, 200.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.8f } }, // 12
      { {  50.0f, 200.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.8f } }, // 13
      { { 100.0f, 200.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.8f } }, // 14
      { {   0.0f, 250.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }, // 15
      { {  50.0f, 250.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.5f, 1.0f } }, // 16
      { { 100.0f, 250.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } }, // 17
    };

    unsigned short faces[20][3] = {
      { 0, 1, 4 }, { 0, 4, 3 }, { 1, 2, 5 }, { 1, 5, 4 },
      { 3, 4, 7 }, { 3, 7, 6 }, { 4, 5, 8 }, { 4, 8, 7 },
      { 6, 7, 10 }, { 6, 10, 9}, { 7, 8, 11 }, { 1, 11, 10},
      { 9, 10, 13}, { 9, 13, 12}, { 10, 11, 14 }, { 10, 14, 13 },
      { 12, 13, 16}, { 12, 16, 15}, { 13, 14, 17}, { 13, 17, 16},
    };

    Ogre::VertexData* vd = new Ogre::VertexData();
    vd->vertexCount = array_size(vertices);
    Ogre::VertexDeclaration* mDecl = vd->vertexDeclaration;
    mDecl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    mDecl->addElement(0, sizeof(float) * 3, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
    mDecl->addElement(0, sizeof(float) * 6, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
    Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
      sizeof(vertices_t), vd->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);
    vd->vertexBufferBinding->setBinding(0, vbuf);

    Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
            Ogre::HardwareIndexBuffer::IT_16BIT, 60, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

    create_mash("patch", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, vd, ibuf,
      Ogre::AxisAlignedBox( 0.0f, 0.0f, 0.0f, 100.0f, 250.0f, 0),
      Ogre::Math::Sqrt(50.0f * 50.0f + 125.0f * 125.0f) );
    return;
  }

  void create_test_new() {

    Ogre::Vector3 vertices[] = {
      {   0.0f,   0.0f, 0.0f}, // 0
      {  50.0f,   0.0f, 0.0f}, // 1
      { 100.0f,   0.0f, 0.0f}, // 2
      {   0.0f,  50.0f, 0.0f}, // 3
      {  50.0f,  50.0f, 0.0f}, // 4
      { 100.0f,  50.0f, 0.0f}, // 5
      {   0.0f, 100.0f, 0.0f}, // 6
      {  50.0f, 100.0f, 0.0f}, // 7
      { 100.0f, 100.0f, 0.0f}, // 8
      {   0.0f, 150.0f, 0.0f}, // 9
      {  50.0f, 150.0f, 0.0f}, // 10
      { 100.0f, 150.0f, 0.0f}, // 11
      {   0.0f, 200.0f, 0.0f}, // 12
      {  50.0f, 200.0f, 0.0f}, // 13
      { 100.0f, 200.0f, 0.0f}, // 14
      {   0.0f, 250.0f, 0.0f}, // 15
      {  50.0f, 250.0f, 0.0f}, // 16
      { 100.0f, 250.0f, 0.0f}, // 17
    };

    Ogre::Vector3 normal[] = {
      { 0.0f, 0.0f, 1.0f }, // 0
      { 0.0f, 0.0f, 1.0f }, // 1
      { 0.0f, 0.0f, 1.0f }, // 2
      { 0.0f, 0.0f, 1.0f }, // 3
      { 0.0f, 0.0f, 1.0f }, // 4
      { 0.0f, 0.0f, 1.0f }, // 5
      { 0.0f, 0.0f, 1.0f }, // 6
      { 0.0f, 0.0f, 1.0f }, // 7
      { 0.0f, 0.0f, 1.0f }, // 8
      { 0.0f, 0.0f, 1.0f }, // 9
      { 0.0f, 0.0f, 1.0f }, // 10
      { 0.0f, 0.0f, 1.0f }, // 11
      { 0.0f, 0.0f, 1.0f }, // 12
      { 0.0f, 0.0f, 1.0f }, // 13
      { 0.0f, 0.0f, 1.0f }, // 14
      { 0.0f, 0.0f, 1.0f }, // 15
      { 0.0f, 0.0f, 1.0f }, // 16
      { 0.0f, 0.0f, 1.0f }, // 17
    };

    Ogre::Vector2 text[] = {
      { 0.0f, 0.0f }, // 0
      { 0.5f, 0.0f }, // 1
      { 1.0f, 0.0f }, // 2
      { 0.0f, 0.2f }, // 3
      { 0.5f, 0.2f }, // 4
      { 1.0f, 0.2f }, // 5
      { 0.0f, 0.4f }, // 6
      { 0.5f, 0.4f }, // 7
      { 1.0f, 0.4f }, // 8
      { 0.0f, 0.6f }, // 9
      { 0.5f, 0.6f }, // 10
      { 1.0f, 0.6f }, // 11
      { 0.0f, 0.8f }, // 12
      { 0.5f, 0.8f }, // 13
      { 1.0f, 0.8f }, // 14
      { 0.0f, 1.0f }, // 15
      { 0.5f, 1.0f }, // 16
      { 1.0f, 1.0f }, // 17
    };

    unsigned short faces[20][3] = {
      { 0, 1, 4 }, { 0, 4, 3 }, { 1, 2, 5 }, { 1, 5, 4 },
      { 3, 4, 7 }, { 3, 7, 6 }, { 4, 5, 8 }, { 4, 8, 7 },
      { 6, 7, 10 }, { 6, 10, 9}, { 7, 8, 11 }, { 1, 11, 10},
      { 9, 10, 13}, { 9, 13, 12}, { 10, 11, 14 }, { 10, 14, 13 },
      { 12, 13, 16}, { 12, 16, 15}, { 13, 14, 17}, { 13, 17, 16},
    };

    Ogre::VertexData* vd = new Ogre::VertexData();
    vd->vertexCount = array_size(vertices);
    Ogre::VertexDeclaration* mDecl = vd->vertexDeclaration;
    mDecl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    mDecl->addElement(1, 0, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
    mDecl->addElement(2, 0, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);

    Ogre::HardwareVertexBufferSharedPtr vbuf_vertex = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
      sizeof(Ogre::Vector3), vd->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    vbuf_vertex->writeData(0, vbuf_vertex->getSizeInBytes(), vertices, true);

    Ogre::HardwareVertexBufferSharedPtr vbuf_normal = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
      sizeof(Ogre::Vector3), vd->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    vbuf_normal->writeData(0, vbuf_normal->getSizeInBytes(), normal, true);

    Ogre::HardwareVertexBufferSharedPtr vbuf_text = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
      sizeof(Ogre::Vector2), vd->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    vbuf_text->writeData(0, vbuf_text->getSizeInBytes(), text, true);

    vd->vertexBufferBinding->setBinding(0, vbuf_vertex);
    vd->vertexBufferBinding->setBinding(1, vbuf_normal);
    vd->vertexBufferBinding->setBinding(2, vbuf_text);

    Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
            Ogre::HardwareIndexBuffer::IT_16BIT, 60, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

    create_mash("patch1", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, vd, ibuf,
      Ogre::AxisAlignedBox( 0.0f, 0.0f, 0.0f, 100.0f, 250.0f, 0),
      Ogre::Math::Sqrt(50.0f * 50.0f + 125.0f * 125.0f) );
    return;
  }

} /* namespace */

class tutorial5
    : public Application {
public:
  tutorial5();
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

tutorial5::tutorial5() : Application("plugins.cfg", "resources-1.9.cfg") {
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

void tutorial5::createScene()
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
  Ogre::Entity* ent;
  create_test();
  create_test_new();
  slot_machine_wheel_text<144>(200, 125.6);

  ent = sceneManager->createEntity("sw4", "SpotWheelText");
  ent->setMaterialName("casino/wheel1");
  node = sceneManager->getRootSceneNode()->createChildSceneNode();
  node->setPosition(251.2f, 0.0f, 0.0f);
  node->attachObject(ent);

  ent = sceneManager->createEntity("sw0", "SpotWheelText");
  ent->setMaterialName("casino/wheel1");
  node = sceneManager->getRootSceneNode()->createChildSceneNode();
  node->setPosition(125.6f, 0.0f, 0.0f);
  node->attachObject(ent);

  ent = sceneManager->createEntity("sw1", "SpotWheelText");
  ent->setMaterialName("casino/wheel1");
  node = sceneManager->getRootSceneNode()->createChildSceneNode();
  node->setPosition(0.0f, 0.0f, 0.0f);
  node->attachObject(ent);

  ent = sceneManager->createEntity("sw2", "SpotWheelText");
  ent->setMaterialName("casino/wheel1");
  node = sceneManager->getRootSceneNode()->createChildSceneNode();
  node->setPosition(-125.6f, 0.0f, 0.0f);
  node->attachObject(ent);

  ent = sceneManager->createEntity("sw3", "SpotWheelText");
  ent->setMaterialName("casino/wheel1");
  node = sceneManager->getRootSceneNode()->createChildSceneNode();
  node->setPosition(-251.2f, 0.0f, 0.0f);
  node->attachObject(ent);
  sw = node;
#if 0
  // create a patch entity from the mesh, give it a material, and attach it to the origin
  ent = sceneManager->createEntity("Patch", "patch");
  ent->setMaterialName("casino/wheel");
  node = sceneManager->getRootSceneNode()->createChildSceneNode();
  node->attachObject(ent);
  node->setPosition(125.6, 0, 0);
  // create a patch entity from the mesh, give it a material, and attach it to the origin
  ent = sceneManager->createEntity("Patch1", "patch1");
  ent->setMaterialName("casino/wheel");
  node = sceneManager->getRootSceneNode()->createChildSceneNode();
  node->attachObject(ent);
  node->setPosition(225.6, 0, 0);
  // create a patch entity from the mesh, give it a material, and attach it to the origin
  ent = sceneManager->createEntity("Patch2", "patch1");
  ent->setMaterialName("casino/wheel");
  node = sceneManager->getRootSceneNode()->createChildSceneNode();
  node->attachObject(ent);
  node->setPosition(325.6, 0, 0);
#endif
}

bool tutorial5::mouse_moved(const OIS::MouseEvent& value) {
  return true;
}

bool tutorial5::mouse_pressed(const OIS::MouseEvent& value, OIS::MouseButtonID id ) {
  return true;
}

bool tutorial5::mouse_released( const OIS::MouseEvent& value, OIS::MouseButtonID id ) {
  //set_mouse_listener(mouse_listener_ptr());
  return true;
}

bool tutorial5::key_pressed(const OIS::KeyEvent& value) {
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

bool tutorial5::key_released(const OIS::KeyEvent& value) {
  //set_key_listener(key_listener_ptr());
  return true;
}

bool tutorial5::frame_startted(const Ogre::FrameEvent& value) {
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
    tutorial5 app;
    app.startApplication();
    return 0;
  }
  catch(const std::exception& e) {
    std::cout << "error: " << e.what() << std::endl;
  }
  return 1;
}
