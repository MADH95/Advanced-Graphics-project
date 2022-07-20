/*

Name: Martin Harvey

Student ID: B00329330

I declare that the following code was produced by Martin Harvey as a individual assignment for the AGP module and that this is my own work.

I am aware of the penalties incurred by submitting in full or in part work that is not our own and that was developed by third parties that are not appropriately acknowledged.

*/
#include "Mesh.h"

//Constructor
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) : _Vertices(vertices), _Indices(indices), _Textures(textures)
{
	//Set the vertex buffers and its attribute pointers.
	setupMesh();
}

//Render the mesh
void Mesh::Draw(Shader shader) const
{
	//Bind appropriate textures
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;

	for (unsigned int i = 0; i < _Textures.size(); i++)
	{
		//Active proper texture unit before binding
		glActiveTexture(GL_TEXTURE0 + i);
		//Retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = _Textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);  //Transfer unsigned int to stream
		else if (name == "texture_normal")
			number = std::to_string(normalNr++);    //Transfer unsigned int to stream
		else if (name == "texture_height")
			number = std::to_string(heightNr++);    //Transfer unsigned int to stream

		//Now set the sampler to the correct texture unit
		shader.setInt(("material." + (name + number)).c_str(), i);
		//Bind the texture
		glBindTexture(GL_TEXTURE_2D, _Textures[i].id);
	}

	//Draw mesh
	glBindVertexArray(_VAO);
	glDrawElements(GL_TRIANGLES, (GLsizei)_Indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	//Set everything back to defaults
	glActiveTexture(GL_TEXTURE0);
}

//Initializes all the buffer objects/arrays
void Mesh::setupMesh()
{
	//Create buffers/arrays
	glGenVertexArrays(1, &_VAO);
	glGenBuffers(1, &_VBO);
	glGenBuffers(1, &_EBO);

	glBindVertexArray(_VAO);

	//Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, _Vertices.size() * sizeof(Vertex), &_Vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _Indices.size() * sizeof(unsigned int), &_Indices[0], GL_STATIC_DRAW);

	//Set the vertex attribute pointers
	//Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	//Vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	//Vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	//Vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	//Vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	glBindVertexArray(0);
}