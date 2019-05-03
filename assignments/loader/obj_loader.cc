#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>

struct Face_vertex {
	int v;
	int t;
	int n;
};

class Obj_file {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> tex_coords;
	std::vector<std::vector<Face_vertex>> faces;

	void parse_vertex(std::string &line) {
		float x, y, z;
		size_t pos = line.find(' ');
		x = std::stof(line.substr(pos + 1), &pos);
		y = std::stof(line.substr(pos + 1), &pos);
		z = std::stof(line.substr(pos + 1), &pos);
		std::cout << x << "  " << y << "  " << z << std::endl;
		//glm::vec3 vertex(x, y, z);
		//vertices.push_back(vertex);
	}

	void parse_normal(std::string &line) {

	}

	void parse_tex_coords(std::string &line) {

	}

	void parse_face(std::string &line) {

	}

public:
	Obj_file(const std::string &fname) {
		std::ifstream f;
		f.open(fname);

		std::string line;
		while (std::getline(f, line)) {
			if (line.length() < 3)
				continue;

			if (line[0] == 'v' && line[1] == ' ')
				parse_vertex(line);
		}

		f.close();
	}
};

int main() {
	Obj_file obj("stones.obj");
	return 0;
}
