#include "model.h"
#include "glm.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include <iostream>

static glm::vec3 AssimpToGLM(aiVector3D aiVec) {
    return glm::vec3(aiVec.x, aiVec.y, aiVec.z);
}

static glm::vec2 AssimpToGLM(aiVector2D aiVec) {
    return glm::vec2(aiVec.x, aiVec.y);
}



void ProcessMesh(model& model, aiMesh* m, aiNode* node, const aiScene* scene) {
    bool hasPositions = m->HasPositions();
    bool hasUVs = m->HasTextureCoords(0);
    bool hasNormals = m->HasNormals();
    bool hasIndices = m->HasFaces();

    std::vector<aiMaterialProperty*> properties;
    aiMaterial* mMaterial = scene->mMaterials[m->mMaterialIndex];

    for (int i = 0; i < mMaterial->mNumProperties; i++)
    {
        aiMaterialProperty* prop = mMaterial->mProperties[i];
        properties.push_back(prop);
    }
    std::vector<float> verts;
    vao_builder mesh_builder{};
    mesh_builder.begin();

    if (hasPositions && hasUVs && hasNormals) {
        for (unsigned int i = 0; i < m->mNumVertices; i++) {
            verts.push_back(m->mVertices[i].x);
            verts.push_back(m->mVertices[i].y);
            verts.push_back(m->mVertices[i].z);
            verts.push_back(m->mNormals[i].x);
            verts.push_back(m->mNormals[i].y);
            verts.push_back(m->mNormals[i].z);
            verts.push_back(m->mTextureCoords[0][i].x);
            verts.push_back(m->mTextureCoords[0][i].y);            
        }
        mesh_builder.add_vertex_buffer(verts);
    }

    std::vector<uint32_t> indices;
    if (hasIndices) {
        for (unsigned int i = 0; i < m->mNumFaces; i++) {
            aiFace currentFace = m->mFaces[i];
            if (currentFace.mNumIndices != 3) {
                std::cerr << "Attempting to import a m with non triangular face structure! cannot load this m." << std::endl;
                return;
            }
            for (unsigned int index = 0; index < m->mFaces[i].mNumIndices; index++) {
                indices.push_back(static_cast<uint32_t>(m->mFaces[i].mIndices[index]));
            }
        }
        mesh_builder.add_index_buffer(indices);
    }
    shapes::aabb aabb = { {m->mAABB.mMin.x, m->mAABB.mMin.y, m->mAABB.mMin.z},
                {m->mAABB.mMax.x, m->mAABB.mMax.y, m->mAABB.mMax.z} };

    mesh new_mesh{};
    new_mesh.m_vao = mesh_builder.build();
    new_mesh.m_index_count = indices.size();
    new_mesh.m_aabb = aabb;
    new_mesh.m_material_index = m->mMaterialIndex;

    model.m_meshes.push_back(new_mesh);
}

void ProcessNode(model& model, aiNode* node, const aiScene* scene) {

    if (node->mNumMeshes > 0) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            unsigned int sceneIndex = node->mMeshes[i];
            aiMesh* mesh = scene->mMeshes[sceneIndex];
            ProcessMesh(model, mesh, node, scene);
        }
    }

    if (node->mNumChildren == 0) {
        return;
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(model, node->mChildren[i], scene);
    }
}

model model::load_model_from_path(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.c_str(),
        aiProcess_Triangulate |
        aiProcess_CalcTangentSpace |
        aiProcess_OptimizeMeshes |
        aiProcess_GenSmoothNormals |
        aiProcess_OptimizeGraph |
        aiProcess_FixInfacingNormals |
        aiProcess_FindInvalidData |
        aiProcess_GenBoundingBoxes
    );
    //
    if (scene == nullptr) {
        return {};
    }

    model m{};

    ProcessNode(m, scene->mRootNode, scene);

    for (int i = 0; i < scene->mNumMaterials; i++)
    {
        auto* material = scene->mMaterials[i];
        material->GetName();
    }

	return m;
}
