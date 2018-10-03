#include <iostream>
#include <exception>

#include <Ogre.h>
#include <OgreConfigFile.h>
#include <OgreMath.h>
#include <OgreFrameListener.h>
#include <OgreApplicationContext.h>
#include <SampleContext.h>


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
            -100.0,100.0,-100.0,        //0 position
            -sqrt13,sqrt13,-sqrt13,     //0 normal
            100.0,100.0,-100.0,         //1 position
            sqrt13,sqrt13,-sqrt13,      //1 normal
            100.0,-100.0,-100.0,        //2 position
            sqrt13,-sqrt13,-sqrt13,     //2 normal
            -100.0,-100.0,-100.0,       //3 position
            -sqrt13,-sqrt13,-sqrt13,    //3 normal
            -100.0,100.0,100.0,         //4 position
            -sqrt13,sqrt13,sqrt13,      //4 normal
            100.0,100.0,100.0,          //5 position
            sqrt13,sqrt13,sqrt13,       //5 normal
            100.0,-100.0,100.0,         //6 position
            sqrt13,-sqrt13,sqrt13,      //6 normal
            -100.0,-100.0,100.0,        //7 position
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

    Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(
      "Test/ColourTest", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
      material->getTechnique(0)->getPass(0)->setVertexColourTracking(Ogre::TVC_AMBIENT);
}


class base_application : public OgreBites::SampleContext/*public OgreBites::ApplicationContext
    , public OgreBites::InputListener
    , public Ogre::FrameListener*/ {
public:
  base_application();
  void setup();
  bool keyPressed(const OgreBites::KeyboardEvent& evt);
  bool frameStarted(const Ogre::FrameEvent& evt);
  bool frameEnded(const Ogre::FrameEvent& evt);
private:
  Ogre::SceneNode* node = 0;
};

base_application::base_application() : OgreBites::SampleContext("OgreTutorialApp")
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
  root->addFrameListener(this);
  Ogre::SceneManager* scnMgr = root->createSceneManager();

  // register our scene with the RTSS
  Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
  shadergen->addSceneManager(scnMgr);

  // without light we would just get a black screen    
  Ogre::Light* light = scnMgr->createLight("MainLight");
  Ogre::SceneNode* lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
  lightNode->setPosition(0, 100, 150);
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
  camNode->setPosition(0, 0, 300);

  // and tell it to render into the main window
  getRenderWindow()->addViewport(cam);

#if 0
  // finally something to render
  Ogre::Entity* ent = scnMgr->createEntity("ogrehead.mesh");
  node = scnMgr->getRootSceneNode()->createChildSceneNode();
  node->setPosition(0,0,0);
  //node->roll(Ogre::Degree(-60));
  node->attachObject(ent);
#else
  createColourCube();
  Ogre::Entity* thisEntity = scnMgr->createEntity("cc", "ColourCube");
  thisEntity->setMaterialName("Test/ColourTest");
  Ogre::SceneNode* thisSceneNode = scnMgr->getRootSceneNode()->createChildSceneNode();
  thisSceneNode->setPosition(0, 0, -300);
  thisSceneNode->attachObject(thisEntity);
  thisSceneNode->yaw(Ogre::Radian(1.0));
  thisSceneNode->pitch(Ogre::Radian(1.0));
#endif

}
#if 1
bool base_application::frameStarted(const Ogre::FrameEvent& evt) {
  return true;  
}

bool base_application::frameEnded(const Ogre::FrameEvent& evt) {
  return true;  
}
#endif

int main(int ac, char* av[]) {
  try {
    base_application app;
    app.initApp();
    app.getRoot()->startRendering();
    app.closeApp();
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