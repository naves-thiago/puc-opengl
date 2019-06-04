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
	Attrib_array(const float &data) : data(data) {
		glGenBuffers(1, &vbo);
	}

	void add_attribute(unsigned int id, unsigned int length, unsigned int offset) {
		Attribute b;
		b.id = id;
		b.length = length;
		b.offset = offset;
		attribs.push_back(b);
	}

	unsigned int vbo;
	const float &data;
	std::vector<Attribute> attribs;
};

class Mesh {
public:
	Mesh();
	bool load_obj(const std::string &fname);
	bool add_buffer(const Attrib_array &b);
	void use(void);

private:
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ebo = 0;
	bool using_indexes;
};

#endif
