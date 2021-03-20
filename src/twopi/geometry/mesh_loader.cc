#include <twopi/geometry/mesh_loader.h>

#include <stdexcept>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <twopi/geometry/mesh.h>

namespace twopi
{
namespace geometry
{
namespace impl
{
class MeshLoaderImpl
{
public:
  MeshLoaderImpl()
  {
  }

  ~MeshLoaderImpl() = default;

  std::shared_ptr<Mesh> Load(const std::string& filepath)
  {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
      throw std::runtime_error(importer.GetErrorString());

    dirpath_ = filepath.substr(0, filepath.find_last_of('/'));

    return ProcessNode(scene->mRootNode, scene);
  }

private:
  std::shared_ptr<Mesh> ProcessNode(aiNode* node, const aiScene* scene)
  {
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      // TODO: collect all meshes
      // meshes_.push_back(ProcessMesh(mesh, scene));
      return ProcessMesh(mesh, scene);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      const auto mesh = ProcessNode(node->mChildren[i], scene);
      if (mesh != nullptr)
        return mesh;
    }

    return nullptr;
  }

  std::shared_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene)
  {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> tex_coords;
    std::vector<uint32_t> indices;
    std::string texture_filepath;

    const auto has_texture = mesh->HasTextureCoords(0);
    const auto has_normal = mesh->HasNormals();

    for (int i = 0; i < mesh->mNumVertices; i++)
    {
      vertices.push_back(mesh->mVertices[i].x);
      vertices.push_back(mesh->mVertices[i].y);
      vertices.push_back(mesh->mVertices[i].z);

      if (has_normal)
      {
        normals.push_back(mesh->mNormals[i].x);
        normals.push_back(mesh->mNormals[i].y);
        normals.push_back(mesh->mNormals[i].z);
      }

      if (has_texture)
      {
        tex_coords.push_back(mesh->mTextureCoords[0][i].x);
        tex_coords.push_back(mesh->mTextureCoords[0][i].y);
      }
    }

    for (int i = 0; i < mesh->mNumFaces; i++)
    {
      aiFace face = mesh->mFaces[i];
      for (int j = 0; j < face.mNumIndices; j++)
        indices.push_back(face.mIndices[j]);
    }

    // process material
    if (mesh->mMaterialIndex >= 0)
    {
      aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

      const auto diffuse_texture_filepaths = LoadMaterialTextureFilepaths(material, aiTextureType_DIFFUSE, "texture_diffuse");
      if (!diffuse_texture_filepaths.empty())
        texture_filepath = diffuse_texture_filepaths[0];

      // TODO: load multiple textures
      /*
      std::vector<std::string> texture_filepaths;

      const auto diffuse_texture_filepaths = LoadMaterialTextureFilepaths(material, aiTextureType_DIFFUSE, "texture_diffuse");
      texture_filepaths.insert(texture_filepaths.end(), diffuse_texture_filepaths.begin(), diffuse_texture_filepaths.end());

      const auto specular_texture_filepaths = LoadMaterialTextureFilepaths(material, aiTextureType_SPECULAR, "texture_specular");
      texture_filepaths.insert(texture_filepaths.end(), specular_texture_filepaths.begin(), specular_texture_filepaths.end());
      */
    }

    auto geometryMesh = std::make_shared<Mesh>();
    geometryMesh->SetVertices(std::move(vertices));
    geometryMesh->SetNormals(std::move(normals));
    geometryMesh->SetTexCoords(std::move(tex_coords));
    geometryMesh->SetIndices(std::move(indices));
    geometryMesh->SetTextureFilepath(std::move(texture_filepath));

    return geometryMesh;
  }

  std::vector<std::string> LoadMaterialTextureFilepaths(aiMaterial* mat, aiTextureType type, const std::string& typeName)
  {
    std::vector<std::string> texture_filepaths;

    for (int i = 0; i < mat->GetTextureCount(type); i++)
    {
      aiString str;
      mat->GetTexture(type, i, &str);
      texture_filepaths.push_back(dirpath_ + '/' + str.C_Str());
    }

    return texture_filepaths;
  }

  std::string dirpath_;
};
}

MeshLoader::MeshLoader()
{
  impl_ = std::make_unique<impl::MeshLoaderImpl>();
}

MeshLoader::~MeshLoader() = default;

std::shared_ptr<Mesh> MeshLoader::Load(const std::string& filepath)
{
  return impl_->Load(filepath);
}
}
}
