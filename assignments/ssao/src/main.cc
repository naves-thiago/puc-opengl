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
#include <model.hh>
#include <iostream>
#include <cstddef>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <random>
#include "cube.h"
#include <keyboard.hh>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void create_gBuffer(void);
void create_lights(void);
void send_lights_to_shader(const Shader &s);
void draw_light_cubes(const Shader &cube_shader);
void toggle_capture_cursor(int key);
void setup_keyboard(Keyboard &k);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const unsigned int LIGHT_COUNT = 20;
Camera camera((float)SCR_WIDTH / SCR_HEIGHT);

struct Light {
	glm::vec3 color;
	glm::vec3 position;
	glm::vec3 axis;
	float speed;
};

typedef enum {
	MODE_NORMAL,        // 1
	MODE_POSITION_BUFF, // 2
	MODE_NORMAL_BUFF,   // 3
	MODE_COLOR_BUFF,    // 4
	MODE_SSAO_BUFF,     // 5
	MODE_BLUR_BUFF      // 6
} View_mode;

GLFWwindow* window;
View_mode mode = MODE_NORMAL;
std::vector<Light> lights;
bool show_lights = false;
bool pause = false;
bool use_ssao = true;

std::vector<glm::vec3> ssao_kernel;
std::vector<glm::vec3> ssao_noise;
float lerp (float a, float b, float f) {
	return a + f * (b - a);
}

void create_ssao_kernel(void) {
	std::uniform_real_distribution<float> random_floats(0.0f, 1.0f);
	std::default_random_engine generator;
	for (int i=0; i<64; i++) {
		glm::vec3 sample(
			random_floats(generator) * 2.0f - 1.0f,
			random_floats(generator) * 2.0f - 1.0f,
			random_floats(generator));
		sample  = glm::normalize(sample);
		sample *= random_floats(generator);
		float scale = (float)i / 64.0f;
		scale  = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssao_kernel.push_back(sample);
	}

	for (int i=0; i<16; i++) {
		glm::vec3 noise(
			random_floats(generator) * 2.0f - 1.0f,
			random_floats(generator) * 2.0f - 1.0f,
			0.0f);
		ssao_noise.push_back(noise);
	}
}

float quad_vertices[] = {
	// Pos       Tex
	-1,  1,  0,   0, 1, // Top Left
	-1, -1,  0,   0, 0, // Bottom Left
	 1,  1,  0,   1, 1, // Top Right
	 1, -1,  0,   1, 0, // Bottom Right
};

unsigned int gBuffer, ssaoBuffer, ssaoBlurBuffer;     // Framebuffers
unsigned int gPosition, gNormal, gColor;              // gBuffer Textures
unsigned int noiseTexture, ssaoColor, ssaoColorBlur;  // SSAO Textures
void create_gBuffer(void) {
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// Position buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
		std::cout << "gBuffer: Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void create_ssao_buffer(void) {
	// Noise texture
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssao_noise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// SSAO
	glGenFramebuffers(1, &ssaoBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBuffer);

	glGenTextures(1, &ssaoColor);
	glBindTexture(GL_TEXTURE_2D, ssaoColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColor, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ssaoBuffer: Framebuffer not complete!" << std::endl;

	// SSAO Blur
	glGenFramebuffers(1, &ssaoBlurBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurBuffer);

	glGenTextures(1, &ssaoColorBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ssaoBlurBuffer: Framebuffer not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void create_lights(void) {
	Light l;
	l.color = glm::vec3(1.0f);
	l.position = glm::vec3(1.2f, 1.0f, 5.0f);
	//l.axis = glm::vec3();
	//l.speed = 0;
	lights.push_back(l);
}

void send_lights_to_shader(const Shader &s) {
	for (int i = 0; i<lights.size(); i++) {
		Light &l = lights[i];
		glm::vec3 light_pos = l.position;
		glm::vec3 light_color = l.color;
		glm::vec3 diffuse_color = light_color * glm::vec3(0.5f); // decrease influence
		glm::vec3 ambient_color = light_color * glm::vec3(0.5f); // low influence
		
		s.setVec("lights[" + std::to_string(i) + "].ambient", ambient_color);
		s.setVec("lights[" + std::to_string(i) + "].diffuse", diffuse_color);
		s.setVec("lights[" + std::to_string(i) + "].specular", glm::vec3(0.2f));
		s.setVec("lights[" + std::to_string(i) + "].position", light_pos);
	}
}

void draw_light_cubes(const Shader &cube_shader) {
	for (int i = 0; i<lights.size(); i++) {
		Light &l = lights[i];
		cube_shader.setVec("lightColor", glm::normalize(l.color));
		glm::mat4 light_model(1.0f);
		light_model = glm::translate(light_model, l.position);
		light_model = glm::scale(light_model, glm::vec3(0.2f));
		cube_shader.setMat("model", light_model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void toggle_capture_cursor(int key) {
	static bool mouse_captured = false;
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

void setup_keyboard(Keyboard &k) {
	k.on_key_down(GLFW_KEY_ENTER, toggle_capture_cursor);
	k.on_key_down(GLFW_KEY_ESCAPE, [](int i) {
		glfwSetWindowShouldClose(window, true);
	});
	k.on_key_down(GLFW_KEY_L, [](int i) { show_lights = !show_lights; });
	k.on_key_down(GLFW_KEY_P, [](int i) { pause = !pause; });
	k.on_key_down(GLFW_KEY_0, [](int i) { use_ssao = !use_ssao; });
	k.on_key_down(GLFW_KEY_1, [](int i) { mode = MODE_NORMAL; });
	k.on_key_down(GLFW_KEY_2, [](int i) { mode = MODE_POSITION_BUFF; });
	k.on_key_down(GLFW_KEY_3, [](int i) { mode = MODE_NORMAL_BUFF; });
	k.on_key_down(GLFW_KEY_4, [](int i) { mode = MODE_COLOR_BUFF; });
	k.on_key_down(GLFW_KEY_5, [](int i) { mode = MODE_SSAO_BUFF; });
	k.on_key_down(GLFW_KEY_6, [](int i) { mode = MODE_BLUR_BUFF; });
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

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SSAO", NULL, NULL);
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

	Keyboard keyboard(window);
	setup_keyboard(keyboard);

	glEnable(GL_DEPTH_TEST);

	unsigned int light_vbo, light_vao, quad_vbo, quad_vao;
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
	Model city("Lowpoly_City_Free_Pack.obj");
	city.setup_gpu();

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

	camera.set_move_speed(5.0f);
	camera.set_pos(-2.2, 1.5, 15, 0, -90, 45);
	camera.set_default_pos(-2.2, 1.5, 15, 0, -90, 45);

	Shader cube_shader("cube.vs", "cube.fs");
	Shader obj_shader("gbuffer.vs", "gbuffer.fs");
	Shader light_shader("light.vs", "light.fs");
	Shader buffer_shader("buffer.vs", "buffer.fs");
	Shader ssao_shader("ssao.vs", "ssao.fs");
	Shader ssao_blur_shader("ssao_blur.vs", "ssao_blur.fs");
	Shader ssao_buffer_shader("ssao_buffer_shader.vs", "ssao_buffer_shader.fs");
	Shader light_no_ssao_shader("light_no_ssao.vs", "light_no_ssao.fs");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0, 0, 0, 1.0f);

	Texture2D tex("Palette.jpg", 0);
	obj_shader.use();
	obj_shader.setInt("tex", 0);

	create_gBuffer();
	create_ssao_kernel();
	create_ssao_buffer();
	ssao_shader.use();
	ssao_shader.setInt("gPosition", 0);
	ssao_shader.setInt("gNormal", 1);
	ssao_shader.setInt("texNoise", 2);

	ssao_blur_shader.use();
	ssao_blur_shader.setInt("ssaoInput", 0);

	light_shader.use();
	light_shader.setInt("gPosition", 0);
	light_shader.setInt("gNormal", 1);
	light_shader.setInt("gColor", 2);
	light_shader.setInt("ssao", 3);

	ssao_buffer_shader.use();
	ssao_buffer_shader.setInt("ssaoInput", 0);

	light_no_ssao_shader.use();
	light_no_ssao_shader.setInt("gPosition", 0);
	light_no_ssao_shader.setInt("gNormal", 1);
	light_no_ssao_shader.setInt("gColor", 2);

	create_lights();
	while (!glfwWindowShouldClose(window))
	{
		keyboard.process_input();
		camera.key_press(window);

		glm::mat4 projection = camera.projection_matrix();
		glm::mat4 view = camera.view_matrix();

		// Geometry pass
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		obj_shader.use();
		glm::mat4 obj_model(1.0f);
		obj_model = glm::scale(obj_model, glm::vec3(0.005f));
		obj_shader.setMat("model", obj_model);
		obj_shader.setMat("view", view);
		obj_shader.setMat("projection", projection);
		tex.activateAndBind();
		city.draw();

		if (mode == MODE_NORMAL || mode == MODE_SSAO_BUFF || mode == MODE_BLUR_BUFF) {
			if (use_ssao || mode != MODE_NORMAL) {
				// Generate SSAO texture
				glBindFramebuffer(GL_FRAMEBUFFER, ssaoBuffer);
				glClear(GL_COLOR_BUFFER_BIT);
				ssao_shader.use();
				// Send kernel + rotation
				for (unsigned int i=0; i<ssao_kernel.size(); i++)
					ssao_shader.setVec("samples[" + std::to_string(i) + "]", ssao_kernel[i]);
				ssao_shader.setMat("projection", projection);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, gPosition);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, gNormal);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, noiseTexture);
				glBindVertexArray(quad_vao);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}

			if (mode == MODE_BLUR_BUFF || (mode == MODE_NORMAL && use_ssao)) {
				// Blur SSAO texture to remove noise
				glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurBuffer);
				glClear(GL_COLOR_BUFFER_BIT);
				ssao_blur_shader.use();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, ssaoColor);
				glBindVertexArray(quad_vao);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}

			if (mode != MODE_NORMAL) {
				// Render SSAO or SSAO_blur buffer
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				ssao_buffer_shader.use();
				glActiveTexture(GL_TEXTURE0);
				if (mode == MODE_SSAO_BUFF)
					glBindTexture(GL_TEXTURE_2D, ssaoColor);
				else
					glBindTexture(GL_TEXTURE_2D, ssaoColorBlur);
				glBindVertexArray(quad_vao);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
			else {
				// Light Pass
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				//glClearColor(0, 0, 0, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				Shader &lshader = use_ssao ? light_shader : light_no_ssao_shader;
				lshader.use();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, gPosition);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, gNormal);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, gColor);
				if (use_ssao) {
					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, ssaoColorBlur);
				}
				lshader.setFloat("shininess", 8.0f);
				lshader.setMat("view", view);
				send_lights_to_shader(lshader);
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
		}
		else {
			// Show gBuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			buffer_shader.use();
			glActiveTexture(GL_TEXTURE0);
			if (mode == MODE_POSITION_BUFF)
				glBindTexture(GL_TEXTURE_2D, gPosition);

			if (mode == MODE_NORMAL_BUFF)
				glBindTexture(GL_TEXTURE_2D, gNormal);

			if (mode == MODE_COLOR_BUFF)
				glBindTexture(GL_TEXTURE_2D, gColor);

			glBindVertexArray(quad_vao);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &light_vao);
	glDeleteBuffers(1, &light_vbo);
	glfwTerminate();
	return 0;
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
