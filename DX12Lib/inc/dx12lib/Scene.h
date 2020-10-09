#pragma once

/*
 *  Copyright(c) 2019 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

/**
 *  @file Scene.h
 *  @date March 21, 2019
 *  @author Jeremiah van Oosten
 *
 *  @brief Scene file for storing scene data.
 */

#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace fs = std::experimental::filesystem;

class aiMaterial;
class aiMesh;
class aiNode;

class CommandList;
class SceneNode;
class Mesh;
class Material;

class Scene
{
public:
    Scene();
    virtual ~Scene();

    /**
     * Load a scene from a file on disc.
     */
    virtual bool LoadFromFile(CommandList& commandList, const std::wstring& fileName);

    /**
     * Load a scene from a string.
     * The scene can be preloaded into a byte array and the
     * scene can be loaded from the loaded byte array.
     *
     * @param scene The byte encoded scene file.
     * @param format The format of the scene file.
     */
    virtual bool LoadFromString(CommandList& commandList, const std::string& scene, const std::string& format);
    virtual void Render(CommandList& commandList);

    virtual std::shared_ptr<SceneNode> GetRootNode() const;

protected:

private:
    void ImportMaterial(CommandList& commandList, const aiMaterial& material, fs::path parentPath);
    void ImportMesh(CommandList& commandList, const aiMesh& mesh);
    std::shared_ptr<SceneNode> ImportSceneNode(CommandList& commandList, std::shared_ptr<SceneNode> parent, aiNode* aiNode);

    using MaterialMap = std::map<std::string, std::shared_ptr<Material> >;
    using MaterialList = std::vector < std::shared_ptr<Material> >;
    using MeshList = std::vector< std::shared_ptr<Mesh> >;

    MaterialMap m_MaterialMap;
    MaterialList m_Materials;
    MeshList m_Meshes;

    std::shared_ptr<SceneNode> m_RootNode;

    std::wstring m_SceneFile;
};