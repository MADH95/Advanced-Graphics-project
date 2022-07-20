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

#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

//Vertex data
struct Vertex
{
	//Position
	glm::vec3 Position;
	//Normal
	glm::vec3 Normal;
	//TexCoords
	glm::vec2 TexCoords;
	//Tangent
	glm::vec3 Tangent;
	//Bitangent
	glm::vec3 Bitangent;
};

//Texture data
struct Texture
{
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh {

	//Variables
private:
	//Mesh Data
	std::vector<Vertex> _Vertices;
	std::vector<unsigned int> _Indices;
	std::vector<Texture> _Textures;
	unsigned int _VAO, _VBO, _EBO;

	//Functions
public:

	//Constructor
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

	//Render the mesh
	void Draw(Shader shader) const;

private:

	//Initializes all the buffer objects/arrays
	void setupMesh();
};