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
        mesh_builder.add_vertex_attribute(0, 8 * sizeof(float), 3);
        mesh_builder.add_vertex_attribute(1, 8 * sizeof(float), 3);
        mesh_builder.add_vertex_attribute(2, 8 * sizeof(float), 2);
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

void get_material_texture(const std::string& directory, aiMaterial* material, model::material_entry& mat, aiTextureType ass_texture_type, texture_map_type gl_texture_type)
{
    uint32_t tex_count = aiGetMaterialTextureCount(material, ass_texture_type);
    if (tex_count > 0)
    {
        aiString resultPath;
        aiGetMaterialTexture(material, ass_texture_type, 0, &resultPath);
        std::string finalPath = directory + std::string(resultPath.C_Str());
        texture tex(finalPath);

        mat.m_material_maps[gl_texture_type] = tex;
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
    std::string directory = path.substr(0, path.find_last_of('/') + 1);
    for (int i = 0; i < scene->mNumMaterials; i++)
    {
        auto* material = scene->mMaterials[i];

        material_entry mat{};

        get_material_texture(directory, material, mat, aiTextureType_DIFFUSE, texture_map_type::diffuse);
        get_material_texture(directory, material, mat, aiTextureType_NORMALS, texture_map_type::normal);
        get_material_texture(directory, material, mat, aiTextureType_SPECULAR, texture_map_type::specular);

        uint32_t diffuseCount = aiGetMaterialTextureCount(material, aiTextureType_DIFFUSE);
        if (diffuseCount > 0)
        {
            aiString resultPath;
            aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &resultPath);
            std::string finalPath = directory + std::string(resultPath.C_Str());
            texture tex(finalPath);

            mat.m_material_maps[texture_map_type::diffuse] = tex;
        }


        m.m_materials.push_back(mat);
    }

	return m;
}
