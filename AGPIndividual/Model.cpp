/*

Name: Martin Harvey

Student ID: B00329330

I declare that the following code was produced by Martin Harvey as a individual assignment for the AGP module and that this is my own work.

I am aware of the penalties incurred by submitting in full or in part work that is not our own and that was developed by third parties that are not appropriately acknowledged.

*/
#include "Model.h"

//Constructors
Model::Model()
	: _GammaCorrection(false)
{
}

Model::Model(std::string const& path, bool gamma) : _GammaCorrection(gamma)
{
	loadModel(path);
}

//Render the model
void Model::draw(const Shader shader, const glm::mat4& model) const
{
	if (!_Meshes.empty())
	{
		for (unsigned int i = 0; i < _Meshes.size(); i++)
		{
			//Modifies the model matrix for the mesh's relative position
			shader.setMat4("model", model * _Transforms[i]);
			_Meshes[i].Draw(shader);
		}
	}
}

//Converts aiMatrix4x4 to glm::mat4
inline glm::mat4 Model::aiMatrix4x4ToGlm(const aiMatrix4x4& from)
{
	glm::mat4 to;
	to[0][0] = (GLfloat)from.a1; to[0][1] = (GLfloat)from.b1;  to[0][2] = (GLfloat)from.c1; to[0][3] = (GLfloat)from.d1;
	to[1][0] = (GLfloat)from.a2; to[1][1] = (GLfloat)from.b2;  to[1][2] = (GLfloat)from.c2; to[1][3] = (GLfloat)from.d2;
	to[2][0] = (GLfloat)from.a3; to[2][1] = (GLfloat)from.b3;  to[2][2] = (GLfloat)from.c3; to[2][3] = (GLfloat)from.d3;
	to[3][0] = (GLfloat)from.a4; to[3][1] = (GLfloat)from.b4;  to[3][2] = (GLfloat)from.c4; to[3][3] = (GLfloat)from.d4;
	return to;
}

//Converts aiVector3D to glm::vec3
inline glm::vec3 Model::aiVector3DToGlm(const aiVector3D& from)
{
	glm::vec3 to;
	to[0] = from.x;
	to[1] = from.y;
	to[2] = from.z;
	return to;
}

//Loads a model from file with ASSIMP
void Model::loadModel(std::string const& path)
{
	//Read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs);

	//Check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}

	//Retrieve the directory path of the filepath
	_Directory = path.substr(0, path.find_last_of('/'));

	//Process ASSIMP's root node recursively
	processNode(scene->mRootNode, scene);
}

//Processes nodes recursively
void Model::processNode(aiNode* node, const aiScene* scene)
{
	//Process each mesh at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiNode* navNode = node->mParent;
		aiMatrix4x4 transform = node->mTransformation;
		while (navNode != NULL)
		{
			transform = navNode->mTransformation * transform;
			navNode = navNode->mParent;
		}
		_Transforms.push_back(aiMatrix4x4ToGlm(transform));

		//The node object only contains indices to index the actual objects in the scene.
		//The scene contains all the data, node is just to keep stuff organized (like relations between nodes)
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		_Meshes.push_back(processMesh(mesh, scene));
	}
	//After all meshes are processed, process each child node
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

//Processes the mesh
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	//Data to fill
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	//Loop through each vertex in the mesh
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		//Positions
		vertex.Position = aiVector3DToGlm(mesh->mVertices[i]);

		//Normals
		if (mesh->HasNormals())
		{
			vertex.Normal = aiVector3DToGlm(mesh->mNormals[i]);
		}

		//Texture coordinates
		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			//A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			//use models where a vertex can have multiple texture coordinates so we always take the first set (0)
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
		{
			vertex.TexCoords = glm::vec2(0.0f);
		}

		//Tangents and Bitangents
		if (mesh->HasTangentsAndBitangents())
		{
			//Tangent
			vertex.Tangent = aiVector3DToGlm(mesh->mTangents[i]);
			//Bitangent=
			vertex.Bitangent = aiVector3DToGlm(mesh->mBitangents[i]);
		}

		vertices.push_back(vertex);
	}

	//Loop through each face in the mesh
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		//Retrieve and store indices
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	//Process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	/*
	We assume a convention for sampler names in the shaders. Each diffuse texture should be named
	as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER

	Same applies to other texture as the following list summarizes:
	diffuse: texture_diffuseN
	specular: texture_specularN
	normal: texture_normalN
	height: texture_heightN
	*/

	//Diffuse maps
	std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

	//Specular maps
	std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

	//Normal maps
	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

	//Height maps
	std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	//Return a mesh object created from the extracted mesh data
	return Mesh(vertices, indices, textures);
}

//Load material textures
std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;

	//Loop through each of the textures
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		bool skip = false;
		for (unsigned int j = 0; j < _TexturesLoaded.size(); j++)
		{
			//Check if texture was loaded before
			if (std::strcmp(_TexturesLoaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(_TexturesLoaded[j]);
				skip = true;
				break;
			}
		}

		if (!skip)
		{
			//Load texture if not done previously
			Texture texture;
			texture.id = TextureFromFile(str.C_Str());
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			//Store it as texture loaded to avoid duplicates
			_TexturesLoaded.push_back(texture);
		}
	}
	return textures;
}

//Loads a texture from a file path
unsigned int Model::TextureFromFile(const char* path, bool gamma)
{
	std::string filename = std::string(path);
	filename = _Directory + '/' + filename;

	//Generate texture ID
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

	//Check if the texture was loaded
	if (data)
	{
		//Check colour format
		GLenum format{};
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		//Bind texture to ID
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//Configure texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		//Display error if texture loading fails
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}