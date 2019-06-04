#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <shader.hh>
#include <texture.hh>
#include <camera.hh>
#include <iostream>
#include <cstddef>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow *window);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

Camera camera((float)SCR_WIDTH / SCR_HEIGHT);

struct vec3 {
	float x, y, z;
};

struct vec2 {
	float x, y;
};

struct Vertex {
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec2 texture;
};

std::vector<Vertex> obj_data;
std::vector<unsigned int> indices;
bool mouse_captured = false;
int mode = 1; // 1 - Normal Render, 2 - Position buffer, 3 - Normal buffer

float quad_vertices[] = {
	// Pos       Tex
	-1,  1,  0,   0, 1, // Top Left
	-1, -1,  0,   0, 0, // Bottom Left
	 1, -1,  0,   1, 0, // Bottom Right
	 1, -1,  0,   1, 0, // Bottom Right (2)
	 1,  1,  0,   1, 1, // Top Right    (2)
	-1,  1,  0,   0, 1  // Top Left     (2)
};

void load_obj(const std::string &fname) {
//	const aiScene *scene = import.ReadFile(fname, aiProcess_Triangulate |
//			aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(fname, aiProcess_Triangulate |
			aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		throw "Cannot load";
	}

	aiMesh *mesh = scene->mMeshes[0];

	for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex v;
		v.position.x = mesh->mVertices[i].x;
		v.position.y = mesh->mVertices[i].y;
		v.position.z = mesh->mVertices[i].z;

		v.normal.x = mesh->mNormals[i].x;
		v.normal.y = mesh->mNormals[i].y;
		v.normal.z = mesh->mNormals[i].z;

		v.tangent.x = mesh->mTangents[i].x;
		v.tangent.y = mesh->mTangents[i].y;
		v.tangent.z = mesh->mTangents[i].z;

		v.texture.x = mesh->mTextureCoords[0][i].x;
		v.texture.y = mesh->mTextureCoords[0][i].y;

		obj_data.push_back(v);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
}

unsigned int gBuffer;
unsigned int gPosition, gNormal, gColor;
unsigned int createGBuffer(void)
{
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// Position buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// Normal buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// Color buffer
	glGenTextures(1, &gColor);
	glBindTexture(GL_TEXTURE_2D, gColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColor, 0);

	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
	unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, attachments);

	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return gBuffer;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Deferred Shading", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	Shader cube_shader("cube.vs", "cube.fs");
	Shader obj_shader("gbuffer.vs", "gbuffer.fs");
	Shader light_shader("light.vs", "light.fs");
	Shader buffer_shader("buffer.vs", "buffer.fs");
	//Shader quad_shader("tmp.vs", "tmp.fs");

	float light_vertices[] = {
		// Positions
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};

	load_obj("golfball.obj");

	glEnable(GL_DEPTH_TEST);

	unsigned int light_vbo, light_vao, obj_vbo, obj_vao, obj_ebo, quad_vbo, quad_vao;
	// ---- light cube ----
	glGenVertexArrays(1, &light_vao);
	glGenBuffers(1, &light_vbo);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s),
	// and then configure vertex attributes(s).
	glBindVertexArray(light_vao);

	glBindBuffer(GL_ARRAY_BUFFER, light_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(light_vertices), light_vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// ---- object ----
	glGenVertexArrays(1, &obj_vao);
	glGenBuffers(1, &obj_vbo);
	glBindVertexArray(obj_vao);

    glGenBuffers(1, &obj_ebo);
	glBindBuffer(GL_ARRAY_BUFFER, obj_vbo);
	glBufferData(GL_ARRAY_BUFFER, obj_data.size() * sizeof(Vertex), obj_data.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
			indices.data(), GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);

	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);

	// tangent attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(void*)offsetof(Vertex, tangent));
	glEnableVertexAttribArray(2);

	// texture attribute
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(void*)offsetof(Vertex, texture));
	glEnableVertexAttribArray(3);

	// ---- quad ----
	glGenVertexArrays(1, &quad_vao);
	glGenBuffers(1, &quad_vbo);
	glBindVertexArray(quad_vao);

	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// texture attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// -------------------

	Texture2D normal_map("golfball.png", 0);
	obj_shader.use();
	obj_shader.setInt("normalMap", 0);

	camera.set_move_speed(2.0f);
	camera.set_pos(0, 0, 5.5, 0, -90, 45);
	camera.set_default_pos(0, 0, 5.5, 0, -90, 45);

	createGBuffer();
	light_shader.use();
	light_shader.setInt("gPosition", 0);
	light_shader.setInt("gNormal", 1);
	light_shader.setInt("gColor", 2);

	while (!glfwWindowShouldClose(window))
	{
		process_input(window);

		//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClearColor(0, 0, 0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 light_pos(1.2f, 1.0f, 5.0f);
		glm::mat4 projection = camera.projection_matrix();
		glm::mat4 view = camera.view_matrix();

		glm::vec3 light_color(1.0f, 1.0f, 1.0f);

		glm::vec3 diffuse_color = light_color * glm::vec3(0.7f); // decrease influence
		glm::vec3 ambient_color = light_color * glm::vec3(0.1f); // low influence

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		obj_shader.use();
		glm::mat4 obj_model(1.0f);
		obj_shader.setMat("model", obj_model);
		obj_shader.setMat("view", view);
		obj_shader.setMat("projection", projection);
		normal_map.activateAndBind();
		glBindVertexArray(obj_vao);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (mode == 1) {
			light_shader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gColor);

			light_shader.setFloat("shininess", 16.0f);
			light_shader.setVec("light.ambient",  ambient_color);
			light_shader.setVec("light.diffuse",  diffuse_color);
			light_shader.setVec("light.specular", glm::vec3(0.5f));
			light_shader.setVec("light.position", light_pos);
			light_shader.setMat("view", view);
			glBindVertexArray(quad_vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		if (mode == 2) {
			buffer_shader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glBindVertexArray(quad_vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		if (mode == 3) {
			buffer_shader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glBindVertexArray(quad_vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		if (mode == 4) {
			buffer_shader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gColor);
			glBindVertexArray(quad_vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		/*
		cube_shader.use();
		cube_shader.setVec("lightColor", light_color);
		glm::mat4 light_model(1.0f);
		light_model = glm::translate(light_model, light_pos);
		light_model = glm::scale(light_model, glm::vec3(0.2f));
		cube_shader.setMat("model", light_model);
		cube_shader.setMat("view", view);
		cube_shader.setMat("projection", projection);
		glBindVertexArray(light_vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		*/
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &light_vao);
	glDeleteVertexArrays(1, &obj_vao);
	glDeleteBuffers(1, &light_vbo);
	glDeleteBuffers(1, &obj_vbo);
	glfwTerminate();
	return 0;
}

void process_input(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	static int last_enter_state = GLFW_RELEASE;
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && last_enter_state == GLFW_RELEASE) {
		mouse_captured = ! mouse_captured;
		if (mouse_captured) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide / capture cursor
			camera.zero_mouse();
			glfwSetCursorPosCallback(window, mouse_callback);
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetCursorPosCallback(window, NULL);
		}
	}
	last_enter_state = glfwGetKey(window, GLFW_KEY_ENTER);

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		mode = 1;
	}

	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		mode = 2;
	}

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		mode = 3;
	}

	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
		mode = 4;
	}

	camera.key_press(window);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	camera.set_aspect_ratio((float)width / height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	camera.mouse_move(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.zoom(yoffset);
}
