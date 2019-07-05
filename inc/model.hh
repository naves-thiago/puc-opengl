#ifndef MODEL_HH
#define MODEL_HH

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <mesh.hh>
//#include <texture.hh>
#include <vector>
#include <iostream>

using std::vector;

class Model {
public:
	Model(const std::string &f);
	void setup_gpu(void);
	void draw(void);

private:
	vector<Mesh> meshes;
	//vector<Texture2D *> textures;

	void process_node(aiNode *node, const aiScene *scene);
	void process_mesh(aiMesh *mesh, const aiScene *scene);
};

#endif
