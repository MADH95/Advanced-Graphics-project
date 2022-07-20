/*

Name: Martin Harvey

Student ID: B00329330

I declare that the following code was produced by Martin Harvey as a individual assignment for the AGP module and that this is my own work.

I am aware of the penalties incurred by submitting in full or in part work that is not our own and that was developed by third parties that are not appropriately acknowledged.

*/
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

class Shader
{
public:
	
	// program ID
	unsigned int ID;

	// Read and build shader
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
	{
		// Retrieve vertex/fragment/geometry source code from filePath
		string vertexCode;
		string fragmentCode;
		string geometryCode;
		ifstream vShaderFile;
		ifstream fShaderFile;
		ifstream gShaderFile;

		// ensure ifstream objects can throw exceptions
		vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		gShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		try
		{
			// open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			stringstream vShaderStream, fShaderStream;

			//read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			//close file handlers
			vShaderFile.close();
			fShaderFile.close();

			//convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();

			//check if geometry shader path is present
			if (geometryPath != nullptr)
			{
				gShaderFile.open(geometryPath);
				stringstream gShaderStream;
				gShaderStream << gShaderFile.rdbuf();
				gShaderFile.close();
				geometryCode = gShaderStream.str();
			}
		}
		catch(ifstream::failure e)
		{
			cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << endl;
		}

		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		// Compile Shaders
		unsigned int vertex, fragment;

		// Vertex Shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");

		// Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");

		//geometry shader
		unsigned int geometry;
		if (geometryPath != nullptr)
		{
			const char* gShaderCode = geometryCode.c_str();
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, NULL);
			glCompileShader(geometry);
			checkCompileErrors(geometry, "GEOMETRY");
		}

		// Shader Program
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		if (geometryPath != nullptr)
			glAttachShader(ID, geometry);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");

		// delete the shaders
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (geometryPath != nullptr)
			glDeleteShader(geometry);
	}

	// use the shader
	void use()
	{
		glUseProgram(ID);
	}

	// utility uniform functions
	void setBool(const string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}

	void setInt(const string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setFloat(const string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setVec2(const string& name, glm::vec2 value)
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}

	void setVec3(const string& name, glm::vec3 value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}

	void setVec4(const string& name, glm::vec4 value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}

	void setMat2(const string& name, glm::mat2 value) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
	}

	void setMat3(const string& name, glm::mat3 value) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
	}

	void setMat4(const string& name, glm::mat4 value) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
	}

	void setMat4Array(const string& name, vector<glm::mat4> values, unsigned int size) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), size, &values[0][0][0]);
	}

private:
	//check compilation errors for shader
	void checkCompileErrors(GLuint shader, string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				cout << "ERROR::" << type << "_SHADER_COMPILATION_ERROR\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				cout << "ERROR::PROGRAM_LINKING_ERROR\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
			}
		}
	}
};
#endif
