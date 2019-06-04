#ifndef MESH_HH
#define MESH_HH

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

struct Attribute {
	unsigned int id;
	unsigned int length;
	unsigned int offset;
};

class Attrib_array {
public:
	Attrib_array(const float &data, size_t data_size, GLenum usage = GL_STATIC_DRAW) :
		data(data), data_size(data_size), usage(usage) {
		glGenBuffers(1, &vbo);
	}

	void add_attribute(unsigned int id, unsigned int length, unsigned int offset) {
		Attribute b;
		b.id = id;
		b.length = length;
		b.offset = offset;
		attribs.push_back(b);
	}

	// Must be called with VAO alread bound
	void send_data(void) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data_size, data, usage);
		for (auto const &a: attribs) {
			glVertexAttribPointer(a.id, a.length, GL_FLOAT, GL_FALSE, a.length * sizeof(float),
					a.offset);
			glEnableVertexAttribArray(a.id);
		}
	}

	~Attrib_array() {
		glDeleteBuffers(1, &vbo);
	}

	unsigned int vbo;
	GLenum usage;
	const float &data;
	const size_t data_size;
	std::vector<Attribute> attribs;
};

class Mesh {
public:
	Mesh();
	bool load_obj(const std::string &fname);
	bool add_buffer(const Attrib_array &b);
	void use(void);

	// glBindVertexArray(vao);
private:
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ebo = 0;
	bool using_indexes;
};

#endif
