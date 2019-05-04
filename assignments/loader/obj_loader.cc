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
	std::vector<std::vector<float> *> vertices;
	std::vector<std::vector<float> *> normals;
	std::vector<std::vector<float> *> tex_coords;
	std::vector<std::vector<Face_vertex> *> faces;

#if 0
	template <class T>
	void parse(const std::string &line, std::vector<T> &out,
	           T (*F)(const std::string &, size_t *)) {
		size_t pos = line.find(' ');
		size_t newpos;
		while (pos < line.length()) {
			try {
				out.push_back(F(line.substr(pos + 1), &newpos));
				pos += newpos;
			}
			catch (...) {
				break;
			}
		}
	}

	void parse_float(const std::string &line, std::vector<float> &out) {
		parse<float>(line, out, std::stof);
	}

	void parse_int(const std::string &line, std::vector<int> &out) {
		parse<int>(line, out, std::stoi);
	}
#endif

	void parse_float(const std::string &line, std::vector<float> &out) {
		size_t pos = line.find(' ');
		size_t newpos;
		while (pos < line.length()) {
			try {
				out.push_back(std::stof(line.substr(pos + 1), &newpos));
				pos += newpos;
			}
			catch (...) {
				break;
			}
		}
	}

	void parse_int(const std::string &line, std::vector<int> &out) {
		size_t pos = line.find(' ');
		size_t newpos;
		while (pos < line.length()) {
			try {
				out.push_back(std::stoi(line.substr(pos + 1), &newpos));
				pos += newpos;
			}
			catch (...) {
				break;
			}
		}
	}

	void parse_vertex(const std::string &line) {
		auto v = new std::vector<float>;
		parse_float(line, *v);
		vertices.push_back(v);
	}

	void parse_normal(const std::string &line) {
		auto v = new std::vector<float>;
		parse_float(line, *v);
		normals.push_back(v);
	}

	void parse_tex_coords(const std::string &line) {
		auto v = new std::vector<float>;
		parse_float(line, *v);
		tex_coords.push_back(v);
	}

public:
	void parse_face(const std::string &line) {
		auto v = new std::vector<Face_vertex>;
		Face_vertex face[4];
		int count = sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&face[0].v, &face[0].t, &face[0].n,
				&face[1].v, &face[1].t, &face[1].n,
				&face[2].v, &face[2].t, &face[2].n,
				&face[3].v, &face[3].t, &face[3].n);

		if (count != 9 && count != 12) {
			std::cout << "Cannot parse face '" << line << "'\n";
			return;
		}

		count /= 3;
		for (int i=0; i<count; i++) {
			std::cout << face[i].v << "  " << face[i].t << "  " <<face[i].n << "  |  ";
			v->push_back(face[i]);
		}

		std::cout << std::endl;
		//parse_float(line, *v);
		faces.push_back(v);
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

			if (line[0] == 'f' && line[1] == ' ')
				parse_face(line);

		}

		f.close();
	}
};

int main() {
	Obj_file obj("stones.obj");

	return 0;
}
