#ifndef MESH_HH
#define MESH_HH

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <texture.hh>

using std::vector;

struct vec3 {
	float x, y, z;
};

struct vec2 {
	float x, y;
};

/**
 * Vertex attributes in the shader:
 * 0 - Position
 * 1 - Normal
 * 2 - Texture coords
 */

struct Vertex {
	vec3 position;
	vec3 normal;
	vec2 tex_coords;
};

class Mesh {
public:
	vector<Vertex> vertices;
	vector<unsigned int> indices;

	void setup_gpu(void);
	void draw(void);
	void free_gpu(void);
	Mesh() : did_setup(false) {}

	Mesh(const Mesh &old) noexcept : vertices(old.vertices),
			indices(old.indices), did_setup(old.did_setup) {
		did_setup = false;
	}

	Mesh(Mesh &&old) noexcept : vertices(move(old.vertices)),
			indices(move(old.indices)), did_setup(old.did_setup) {
		did_setup = false;
	}

	~Mesh();
private:
	unsigned int VAO, VBO, EBO;
	bool did_setup;
};

#endif
