/*
    Project Motorway Source Code
    Copyright (C) 2018 Prévost Baptiste

    This file is part of Project Motorway source code.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

class DrawCommandBuilder;

class Transform;
class Mesh;
class FreeCamera;
class TransactionHandler;
class GraphicsAssetCache;
class LightGrid;
class FileSystemObject;

struct PointLightData;
struct SpotLightData;
struct DirectionalLightData;
struct IBLProbeData;
struct Ray;

#include <vector>

#include <Maths/Transform.h>
#include <Maths/BoundingSphere.h>
#include <Framework/Mesh.h>
#include <Framework/Light.h>

using nyaComponentHandle_t = uint32_t;

class Scene
{
public:
    struct RenderableMesh
    {
        nyaComponentHandle_t transform;
        Mesh* meshResource;

        union
        {
            struct
            {
                uint8_t isVisible : 1;
                uint8_t renderDepth : 1;
                uint8_t useBatching : 1;
                uint8_t : 0;
            };

            uint32_t    flags;
        };

        RenderableMesh()
        {
            renderDepth = 1;
            isVisible = 1;
        }
    };

    struct IBLProbe
    {
        nyaComponentHandle_t    transform;
        IBLProbeData*           iblProbeData;
    };

    struct PointLight
    {
        nyaComponentHandle_t    transform;
        PointLightData*         pointLightData;
    };

    struct Node {
        std::string                     name;
        nyaStringHash_t                 hashcode;
        nyaComponentHandle_t            transform;
        std::vector<Scene::Node*>       children;

        Transform* worldTransform;

        Node( const std::string& nodeName = "Node" )
            : name( nodeName )
            , hashcode( nya::core::CRC32( name ) )
            , transform( 0 )
        {
            name.resize( 256 );
        }

        Node( Node& node )
            : name( node.name )
            , hashcode( node.hashcode )
            , transform( node.transform )
        {

        }

        Node* findChildByHashcode( const nyaStringHash_t nodeHashcode )
        {
            // TODO Precompute hash/node map to avoid a vector lookup?
            for ( Node* child : children ) {
                if ( child->hashcode == nodeHashcode ) {
                    return child;
                }
            }

            return nullptr;
        }

        virtual Node* clone( LightGrid* lightGrid )
        {
            return new Node( *this );
        }

        virtual nyaStringHash_t getNodeType() const
        {
            return NYA_STRING_HASH( "Node" );
        }

        virtual void remove( LightGrid* lightGrid )
        {

        }

        virtual bool intersect( const Ray& ray, float& hitDistance ) const
        {
            return false;
        }

        virtual void serialize( FileSystemObject* stream, const nyaStringHash_t sceneNodeHashcode = 0x0 )
        {
            //stream->write( sceneNodeHashcode );
            //stream->write<int32_t>( 0 );

            //stream->writeString( name.c_str() );
            //stream->write<uint8_t>( 0x0 );

            //stream->write<uint8_t>( 0 );
            //stream->write<uint8_t>( 0x0 ); // RESERVED       
            //stream->write<uint8_t>( 0x0 ); // RESERVED
            //stream->write<uint8_t>( 0x0 ); // RESERVED
        }

        virtual void deserialize( FileSystemObject* stream, GraphicsAssetCache* graphicsAssetCache, LightGrid* lightGrid )
        {
            //uint8_t hasRigidBody = 0, reserved1, reserved2, reserved3;
            //FLAN_DESERIALIZE_VARIABLE( stream, hasRigidBody );

            //// RESERVED
            //FLAN_DESERIALIZE_VARIABLE( stream, reserved1 );
            //FLAN_DESERIALIZE_VARIABLE( stream, reserved2 );
            //FLAN_DESERIALIZE_VARIABLE( stream, reserved3 );

            //transform.deserialize( stream );

            //if ( hasRigidBody == 1 ) {
            //    rigidBody = new RigidBody( 0 );
            //    rigidBody->deserialize( stream );
            //}
        }
    };

    struct StaticGeometryNode : public Node
    {
        nyaComponentHandle_t    mesh;
        Mesh**                  meshResource;

        Node* clone( LightGrid* lightGrid ) override
        {
            return new Node( *this );
        }

        nyaStringHash_t getNodeType() const override
        {
            return NYA_STRING_HASH( "StaticGeometryNode" );
        }

        bool intersect( const Ray& ray, float& hitDistance ) const override
        {
            float dontCare = 0.0f;

            const nyaVec3f& worldTranslation = worldTransform->getWorldTranslation();
            const nyaVec3f& worldScale = worldTransform->getWorldScale();

            AABB aabb = ( *meshResource )->getMeshAABB();
            aabb.minPoint += worldTranslation;
            aabb.maxPoint += worldTranslation;

            aabb.maxPoint *= worldScale;
            aabb.maxPoint *= worldScale;

            return nya::maths::RayAABBIntersectionTest( aabb, ray, hitDistance, dontCare );
        }
    };

    struct PointLightNode : public Node
    {
        nyaComponentHandle_t pointLight;
        PointLightData**     pointLightData;

        Node* clone( LightGrid* lightGrid ) override
        {
            return new Node( *this );
        }

        nyaStringHash_t getNodeType() const override
        {
            return NYA_STRING_HASH( "PointLightNode" );
        }

        bool intersect( const Ray& ray, float& hitDistance ) const override
        {
            BoundingSphere sphere;
            sphere.center = ( *pointLightData )->worldPosition;
            sphere.radius = 1.5f;

            return nya::maths::RaySphereIntersectionTest( sphere, ray, hitDistance );
        }
    };

    struct DirectionalLightNode : public Node
    {
        DirectionalLightData* dirLightData;

        Node* clone( LightGrid* lightGrid ) override
        {
            return new Node( *this );
        }

        nyaStringHash_t getNodeType() const override
        {
            return NYA_STRING_HASH( "DirectionalLightNode" );
        }

        bool intersect( const Ray& ray, float& hitDistance ) const override
        {
            const nyaVec3f& worldTranslation = worldTransform->getWorldTranslation();
            const float worldScale = worldTransform->getWorldBiggestScale();

            BoundingSphere sphere;
            sphere.center = worldTranslation;
            sphere.radius = 1.0f;

            return nya::maths::RaySphereIntersectionTest( sphere, ray, hitDistance );
        }
    };

    struct IBLProbeNode : public Node
    {
        nyaComponentHandle_t iblProbe;
        IBLProbeData**       iblProbeData;

        Node* clone( LightGrid* lightGrid ) override
        {
            return new Node( *this );
        }

        nyaStringHash_t getNodeType() const override
        {
            return NYA_STRING_HASH( "IBLProbeNode" );
        }

        bool intersect( const Ray& ray, float& hitDistance ) const override
        {
            const nyaVec3f& worldTranslation = worldTransform->getWorldTranslation();
            const float worldScale = worldTransform->getWorldBiggestScale();

            BoundingSphere sphere;
            sphere.center = ( *iblProbeData )->worldPosition + worldTranslation;
            sphere.radius = ( *iblProbeData )->radius * worldScale;
            
            return nya::maths::RaySphereIntersectionTest( sphere, ray, hitDistance );
        }
    };

public:
    template<typename T>
    struct ComponentDatabase
    {
        T* components;
        size_t                  capacity;
        nyaComponentHandle_t    usageIndex;

        ComponentDatabase()
            : components( nullptr )
            , capacity( 0ull )
            , usageIndex( 0u )
        {

        }

        nyaComponentHandle_t allocate()
        {
            return ( usageIndex++ % capacity );
        }

        T& operator [] ( const nyaComponentHandle_t handle )
        {
            return components[handle];
        }

        T& operator [] ( const nyaComponentHandle_t handle ) const
        {
            return components[handle];
        }
    };

    ComponentDatabase<FreeCamera>       FreeCameraDatabase;
    ComponentDatabase<Transform>        TransformDatabase;
    ComponentDatabase<RenderableMesh>   RenderableMeshDatabase;
    ComponentDatabase<IBLProbe>         IBLProbeDatabase;
    ComponentDatabase<PointLight>       PointLightDatabase;

public:
                            Scene( BaseAllocator* allocator, const std::string& sceneName = "Default Scene" );
                            Scene( Scene& scene ) = default;
                            Scene& operator = ( Scene& scene ) = default;
                            ~Scene();

    void                    setSceneName( const std::string& sceneName );
    const std::string&      getSceneName() const;

    void                    updateLogic( const float deltaTime );
    void                    collectDrawCmds( DrawCommandBuilder& drawCmdBuilder );

    Node*                   intersect( const Ray& ray );

    StaticGeometryNode*     allocateStaticGeometry();
    PointLightNode*         allocatePointLight();
    IBLProbeNode*           allocateIBLProbe();
    DirectionalLightNode*   allocateDirectionalLight();

private:
    std::string             name;
    BaseAllocator*          memoryAllocator;

    std::vector<Node*>      sceneNodes;
};
