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
#include <ctime>
#include <cstdlib>
#include "cube.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow *window);
unsigned int createGBuffer(void);
void create_lights(void);
void update_lights(bool restart);
void draw_lights(const Shader &s);
void draw_light_cubes(const Shader &cube_shader);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const unsigned int LIGHT_COUNT = 20;
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

struct Light {
	glm::vec3 color;
	glm::vec3 position;
	glm::vec3 axis;
	float speed;
};

std::vector<Vertex> obj_data;
std::vector<unsigned int> indices;
bool mouse_captured = false;
int mode = 1; // 1 - Normal Render, 2 - Position buffer, 3 - Normal buffer, 4 - Color buffer
std::vector<Light> lights;
bool show_lights = true;
bool pause = false;
bool pending_light_restart = false;

float quad_vertices[] = {
	// Pos       Tex
	-1,  1,  0,   0, 1, // Top Left
	-1, -1,  0,   0, 0, // Bottom Left
	 1,  1,  0,   1, 1, // Top Right
	 1, -1,  0,   1, 0, // Bottom Right
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
unsigned int createGBuffer(void) {
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

void create_lights(void) {
	Light l;
	srand(time(NULL));
	#define randf() ((rand() % 255) / 255.0f)
	for (unsigned int i=0; i<LIGHT_COUNT; i++) {
		l.color = glm::normalize(glm::vec3(randf(), randf(), randf())) * ((rand() % 100) / 300.0f);
		glm::vec3 axis(randf(), randf(), randf());
		axis = glm::normalize(axis);
		l.axis = axis;
		glm::vec3 tmp = axis * (4 + 2 * (rand() % 100) / 100.0f); // Distance in [6, 10] from (0, 0)
		l.position = glm::cross(tmp, glm::vec3(1.0f, 0.0f, 0.0f));
		l.speed = glm::radians(20.0f + rand() % 50);
		lights.push_back(l);
	}
}

void update_lights(bool restart = false) {
	static float last_time = glfwGetTime();
	if (restart)
		last_time = glfwGetTime();
	float current_time = glfwGetTime();
	float delta_time = current_time - last_time;
	last_time = current_time;

	for (int i = 0; i<LIGHT_COUNT; i++) {
		Light &l = lights[i];
		l.position = glm::vec3(
				glm::rotate(glm::mat4(1.0f), delta_time * l.speed, l.axis) *
				glm::vec4(l.position, 1.0));
	}
}

void draw_lights(const Shader &s) {
	for (int i = 0; i<LIGHT_COUNT; i++) {
		Light &l = lights[i];
		glm::vec3 light_pos = l.position;
		glm::vec3 light_color = l.color;
		glm::vec3 diffuse_color = light_color * glm::vec3(0.7f); // decrease influence
		glm::vec3 ambient_color = light_color * glm::vec3(0.1f); // low influence
		
		s.setVec("lights[" + std::to_string(i) + "].ambient", ambient_color);
		s.setVec("lights[" + std::to_string(i) + "].diffuse", diffuse_color);
		s.setVec("lights[" + std::to_string(i) + "].specular", glm::vec3(0.5f));
		s.setVec("lights[" + std::to_string(i) + "].position", light_pos);
	}
}

void draw_light_cubes(const Shader &cube_shader) {
	for (int i = 0; i<LIGHT_COUNT; i++) {
		Light &l = lights[i];
		cube_shader.setVec("lightColor", glm::normalize(l.color));
		glm::mat4 light_model(1.0f);
		light_model = glm::translate(light_model, l.position);
		light_model = glm::scale(light_model, glm::vec3(0.2f));
		cube_shader.setMat("model", light_model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

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
	camera.set_pos(0, 0, 13, 0, -90, 45);
	camera.set_default_pos(0, 0, 5.5, 0, -90, 45);

	createGBuffer();
	light_shader.use();
	light_shader.setInt("gPosition", 0);
	light_shader.setInt("gNormal", 1);
	light_shader.setInt("gColor", 2);

	buffer_shader.use();
	buffer_shader.setInt("gBuffer", 0);

	create_lights();
	while (!glfwWindowShouldClose(window))
	{
		process_input(window);

		glClearColor(0, 0, 0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		update_lights(pause || pending_light_restart);
		pending_light_restart = false;

		glm::vec3 light_pos = lights[0].position;
		glm::vec3 light_color = lights[0].color;

		glm::mat4 projection = camera.projection_matrix();
		glm::mat4 view = camera.view_matrix();

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
			draw_lights(light_shader);
			light_shader.setMat("view", view);
			glBindVertexArray(quad_vao);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			if (show_lights) {
				// copy depth buffer (may break... in particular with MSAA)
				glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
				glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT,
						GL_DEPTH_BUFFER_BIT, GL_NEAREST);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

				cube_shader.use();
				cube_shader.setMat("view", view);
				cube_shader.setMat("projection", projection);
				glBindVertexArray(light_vao);
				draw_light_cubes(cube_shader);
			}
		}
		else {
			buffer_shader.use();
			glActiveTexture(GL_TEXTURE0);
			if (mode == 2)
				glBindTexture(GL_TEXTURE_2D, gPosition);

			if (mode == 3)
				glBindTexture(GL_TEXTURE_2D, gNormal);

			if (mode == 4)
				glBindTexture(GL_TEXTURE_2D, gColor);

			glBindVertexArray(quad_vao);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

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
	
	static int last_l_state = GLFW_RELEASE;
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && last_l_state == GLFW_RELEASE) {
		show_lights = ! show_lights;
	}
	last_l_state = glfwGetKey(window, GLFW_KEY_L);

	static int last_p_state = GLFW_RELEASE;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && last_p_state == GLFW_RELEASE) {
		pause = ! pause;
	}
	last_p_state = glfwGetKey(window, GLFW_KEY_P);

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		mode = 1;

	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		mode = 2;

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		mode = 3;

	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		mode = 4;

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
