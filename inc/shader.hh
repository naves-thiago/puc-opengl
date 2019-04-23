#ifndef SHADER_HH
#define SHADER_HH

#include <glad/glad.h>
#include <glm/glm.hpp>

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
	void setMat(const std::string &name, const glm::mat2 &value) const;
	void setMat(const std::string &name, const glm::mat3 &value) const;
	void setMat(const std::string &name, const glm::mat4 &value) const;
	unsigned int getLocation(const std::string &name) const;

private:
	void checkCompileErrors(unsigned int shader, std::string type);
};

#endif
