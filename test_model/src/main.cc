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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow *window);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

Camera camera((float)SCR_WIDTH / SCR_HEIGHT);

bool mouse_captured = false;

Model city("Lowpoly_City_Free_Pack.obj");
//Model city("golfball_no_normals.obj");

int main(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "City", NULL, NULL);
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

	city.setup_gpu();

	Texture2D tex("Palette.jpg", 0);
	Shader obj_shader("golfball.vs", "golfball.fs");
	obj_shader.use();
	obj_shader.setInt("tex", 0);

	glEnable(GL_DEPTH_TEST);

	//camera.set_fov_limits(1, 60);
	//camera.set_fov(45);
	camera.set_move_speed(5.0f);
	camera.set_pos(-2.2, 1.5, 15, 0, -90, 45);
	camera.set_default_pos(-2.2, 1.5, 15, 0, -90, 45);

	while (!glfwWindowShouldClose(window))
	{
		process_input(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 light_pos(1.2f, 1.0f, 5.0f);
		glm::mat4 projection = camera.projection_matrix();
		glm::mat4 view = camera.view_matrix();

		glm::vec3 light_color(1.0f, 1.0f, 1.0f);

		glm::vec3 diffuse_color = light_color * glm::vec3(0.7f); // decrease influence
		glm::vec3 ambient_color = light_color * glm::vec3(0.1f); // low influence

		obj_shader.use();
		obj_shader.setFloat("shininess", 16.0f);
		obj_shader.setVec("light.ambient",  ambient_color);
		obj_shader.setVec("light.diffuse",  diffuse_color);
		obj_shader.setVec("light.specular", glm::vec3(0.5f));
		obj_shader.setVec("light.position", light_pos);
		glm::mat4 obj_model(1.0f);
		obj_model = glm::scale(obj_model, glm::vec3(0.005f));
		obj_shader.setMat("model", obj_model);
		obj_shader.setMat("view", view);
		obj_shader.setMat("projection", projection);
		obj_shader.setVec("viewPos", camera.position);
		tex.activateAndBind();

		city.draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

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
