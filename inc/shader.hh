#ifndef SHADER_HH
#define SHADER_HH

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
	// Program ID
	unsigned int ID;

	// Loads and compiles the shader
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

	// Activate the shader
	void use();

	// Uniform functions
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;

private:
	void checkCompileErrors(unsigned int shader, std::string type);
};

#endif
