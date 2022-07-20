/*

Name: Martin Harvey

Student ID: B00329330

I declare that the following code was produced by Martin Harvey as a individual assignment for the AGP module and that this is my own work.

I am aware of the penalties incurred by submitting in full or in part work that is not our own and that was developed by third parties that are not appropriately acknowledged.

*/
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"

#include "Shader.h"
#include "Mesh.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

class Model
{
	//Variables
private:
	//Model Data
	std::vector<Texture> _TexturesLoaded;	//Stores loaded textures
	std::vector<Mesh> _Meshes;				//Stores loaded meshes
	std::vector<glm::mat4> _Transforms;		//Stores node transformations

	std::string _Directory;
	bool _GammaCorrection;

	//Functions
public:

	//Constructors
	Model();

	Model(std::string const& path, bool gamma = false);

	//Render the model
	void draw(const Shader shader, const glm::mat4& model) const;

private:

	//Converts aiMatrix4x4 to glm::mat4
	inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from);

	//Converts aiVector3D to glm::vec3
	inline glm::vec3 aiVector3DToGlm(const aiVector3D& from);

	//Loads a model from file with ASSIMP
	void loadModel(std::string const& path);

	//Processes nodes recursively
	void processNode(aiNode* node, const aiScene* scene);

	//Processes the mesh
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	//Load material textures
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

	//Loads a texture from a file path
	unsigned int TextureFromFile(const char* path, bool gamma = false);
};