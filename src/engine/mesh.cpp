#include "mesh.h"

#include "core/types.h"
#include "containers/darray.h"
#include "containers/hashtable.h"
#include "math/math.h"
#include "graphics/shader.h"

#include <ofbx.h>
#include <glad/glad.h>

// TODO: Add other things as necessary
struct Vertex
{
    Vector3 position;
    Vector3 normal;
    Vector4 color;
};

void Mesh::LoadFBX(DynamicArray<u8>& fileview)
{
    ofbx::IScene* scene = ofbx::load(fileview.data(), fileview.size(), (ofbx::u64) ofbx::LoadFlags::TRIANGULATE);
    AssertWithMessage(scene, ofbx::getError());

    Vertex* vertices = nullptr;
    u32* indices = nullptr;

    int meshCount = scene->getMeshCount();
    HashTable<void*, u32> ofbxObjectToSubmeshIndex(meshCount);
    submeshes.Reserve(meshCount);

    for (int i = 0; i < meshCount; i++)
    {
        const ofbx::Mesh& mesh = *scene->getMesh(i);
        const ofbx::Geometry& geom = *mesh.getGeometry();

        int vertexCount = geom.getVertexCount();
        int indexCount = geom.getIndexCount();

        vertices = (Vertex*) PlatformReallocate(vertices, vertexCount * sizeof(Vertex));
        indices  = (u32*) PlatformReallocate(indices, indexCount * sizeof(u32));

        // Copy vertices
        const ofbx::Vec3* fbxVertices = geom.getVertices();
        const ofbx::Vec3* fbxNormals  = geom.getNormals();
        const int* fbxMaterials = geom.getMaterials();

        for (int j = 0; j < vertexCount; j++) 
        {
            Vertex& vertex = vertices[j];

            vertex.position = Vector3(fbxVertices[j].x, fbxVertices[j].y, fbxVertices[j].z);

            vertex.normal = Vector3(fbxNormals[j].x, fbxNormals[j].y, fbxNormals[j].z);

            const ofbx::Material* material = mesh.getMaterial(fbxMaterials ? fbxMaterials[j / 3] : 0);
            ofbx::Color color = material->getDiffuseColor();
            vertex.color = Vector4(color.r, color.g, color.b, 1);
        }

        // Copy Indices
        const int* fbxIndices = geom.getFaceIndices();

        for (int j = 0; j < indexCount; j++)
            indices[j] = (fbxIndices[j] < 0) ? -fbxIndices[j] - 1 : fbxIndices[j];

        {   // Add Submesh
            Submesh& submesh = submeshes.EmplaceBack();

            glGenVertexArrays(1, &submesh.vao);
            glBindVertexArray(submesh.vao);

            u32 vbo;
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (const void*) offsetof(Vertex, position));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), (const void*) offsetof(Vertex, normal));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(Vertex), (const void*) offsetof(Vertex, color));

            glGenBuffers(1, &submesh.ibo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, submesh.ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * indexCount, indices, GL_STATIC_DRAW);

            submesh.indexCount = indexCount;

            {   // Transforms
                ofbx::Vec3 position = mesh.getLocalTranslation();
                submesh.transform.SetPosition(Vector3(position.x, position.y, position.z));

                ofbx::Vec3 rotation = mesh.getLocalRotation();
                submesh.transform.SetRotation(Math::DegToRad * Vector3(rotation.x, rotation.y, rotation.z));

                ofbx::Vec3 scale = mesh.getLocalScaling();
                submesh.transform.SetScale(Vector3(scale.x, scale.y, scale.z));

                ofbxObjectToSubmeshIndex[(void*)&mesh] = i;
            }
        }
    }

    // Set parents
    for (int i = 0; i < meshCount; i++)
    {
        const ofbx::Mesh& mesh = *scene->getMesh(i);
        ofbx::Object* parent = mesh.getParent();

        if (parent && ofbxObjectToSubmeshIndex.Find((void*)parent))
        {
            u32 parentIndex = ofbxObjectToSubmeshIndex[(void*)parent];
            Transform* parentTransform = &submeshes[parentIndex].transform;
            submeshes[i].transform.SetParent(parentTransform);
        }
    }

    PlatformFree(vertices);
    PlatformFree(indices);
}

