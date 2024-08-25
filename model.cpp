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



void ProcessMesh(model& model, aiMesh* mesh, aiNode* node, const aiScene* scene) {
    bool hasPositions = mesh->HasPositions();
    bool hasUVs = mesh->HasTextureCoords(0);
    bool hasNormals = mesh->HasNormals();
    bool hasIndices = mesh->HasFaces();

    std::vector<aiMaterialProperty*> properties;
    aiMaterial* meshMaterial = scene->mMaterials[mesh->mMaterialIndex];

    for (int i = 0; i < meshMaterial->mNumProperties; i++)
    {
        aiMaterialProperty* prop = meshMaterial->mProperties[i];
        properties.push_back(prop);
    }
    std::vector<float> verts;
    if (hasPositions && hasUVs && hasNormals) {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            verts.push_back(mesh->mVertices[i].x);
            verts.push_back(mesh->mVertices[i].y);
            verts.push_back(mesh->mVertices[i].z);
            verts.push_back(mesh->mNormals[i].x);
            verts.push_back(mesh->mNormals[i].y);
            verts.push_back(mesh->mNormals[i].z);
            verts.push_back(mesh->mTextureCoords[0][i].x);
            verts.push_back(mesh->mTextureCoords[0][i].y);            
        }
    }

    std::vector<uint32_t> indices;
    if (hasIndices) {
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace currentFace = mesh->mFaces[i];
            if (currentFace.mNumIndices != 3) {
                std::cerr << "Attempting to import a mesh with non triangular face structure! cannot load this mesh." << std::endl;
                return;
            }
            for (unsigned int index = 0; index < mesh->mFaces[i].mNumIndices; index++) {
                indices.push_back(static_cast<uint32_t>(mesh->mFaces[i].mIndices[index]));
            }
        }
    }
    shapes::aabb aabb = { {mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z},
                {mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z} };

    // gen vao
    // mesh should hold, vao, num indices, texture mappings
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
        return;
    }

    model m{};

    ProcessNode(m, scene->mRootNode, scene);

	return m;
}
