#include <model.hh>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Model::Model(const std::string &f) {
	Assimp::Importer import;
	//const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate |
	//	aiProcess_FlipUVs);
	const aiScene *scene = import.ReadFile(f, aiProcess_Triangulate |
			aiProcess_GenNormals);

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	process_node(scene->mRootNode, scene);
}

void Model::process_node(aiNode *node, const aiScene *scene) {
	for(unsigned int i=0; i<node->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		process_mesh(mesh, scene);
	}

	for(unsigned int i=0; i<node->mNumChildren; i++)
		process_node(node->mChildren[i], scene);
}

void Model::process_mesh(aiMesh *mesh, const aiScene *scene) {
	Mesh m;

	for (unsigned int i=0; i<mesh->mNumVertices; i++) {
		Vertex v;
		v.position.x = mesh->mVertices[i].x;
		v.position.y = mesh->mVertices[i].y;
		v.position.z = mesh->mVertices[i].z;

		v.normal.x = mesh->mNormals[i].x;
		v.normal.y = mesh->mNormals[i].y;
		v.normal.z = mesh->mNormals[i].z;

		/*
		v.tangent.x = mesh->mTangents[i].x;
		v.tangent.y = mesh->mTangents[i].y;
		v.tangent.z = mesh->mTangents[i].z;
		*/

		if (mesh->mTextureCoords[0]) {
			v.tex_coords.x = mesh->mTextureCoords[0][i].x;
			v.tex_coords.y = mesh->mTextureCoords[0][i].y;
		}

		m.vertices.push_back(v);
	}

	for (unsigned int i=0; i<mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j=0; j<face.mNumIndices; j++)
			m.indices.push_back(face.mIndices[j]);
	}

	meshes.push_back(std::move(m));
}

void Model::setup_gpu(void) {
	for (unsigned int i=0; i<meshes.size(); i++)
		meshes[i].setup_gpu();
}

void Model::draw(void) {
	for (unsigned int i=0; i<meshes.size(); i++)
		meshes[i].draw();
}

