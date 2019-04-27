#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

float vertices[] = {
	-0.5, -0.5, 0, // left
	 0.5, -0.5, 0, // right
	 0,    0.5, 0, // top
};

float colors[] = {
	1, 0, 0, 1, // R - left
	0, 1, 0, 1, // G - right
	0, 0, 1, 1, // B - top
};

unsigned int pos_buffer, color_buffer, VAO;
void send_vertices()
{

	// Create VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &pos_buffer); // Create buffer ID
	glBindBuffer(GL_ARRAY_BUFFER, pos_buffer);
	// Copy the data. GL_STATIC_DRAW -> Data likely wont change
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Uses the last buffer associated with GL_ARRAY_BUFFER
	// Params: attribute location, attribute size, type, normalize, stride, start position (in the buffer)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0); // param: attribute location

	glGenBuffers(1, &color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(3); // param: attribute location
}

bool check_shader_compilation(unsigned int shader, char *name)
{
	// Check for compilation errors
	int success;
	char info_log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 521, NULL, info_log);
		printf("ERROR: %s compilation failed.\n%s\n", name, info_log);
	}
	return success;
}

bool check_program_link(unsigned int program, char *name)
{
	// Check for compilation errors
	int success;
	char info_log[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 521, NULL, info_log);
		printf("ERROR: %s link failed.\n%s\n", name, info_log);
	}
	return success;
}

unsigned int compile_and_send_vertex_shader()
{
	static const char * const shader =
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 3) in vec4 aColor;\n"
		"out vec4 colorV;\n"
		"void main()\n"
		"{\n"
		"	colorV = aColor;\n"
		"	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\n";
	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &shader, NULL);
	glCompileShader(vertex_shader);
	if (!check_shader_compilation(vertex_shader, "Vertex shader"))
		exit(-1);
	return vertex_shader;
}

unsigned int compile_and_send_fragment_shader()
{
	static const char * const shader =
		"#version 330 core\n"
		"in vec4 colorV;\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"	//FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"	FragColor = colorV;\n"
		"}\n";
	unsigned int frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &shader, NULL);
	glCompileShader(frag_shader);
	if (!check_shader_compilation(frag_shader, "Fragment shader"))
		exit(-1);
	return frag_shader;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
	if (window == NULL)
	{
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD\n");
		return -1;
	}
	
	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	send_vertices();

	// Create shaders
	unsigned int vertex_shader = compile_and_send_vertex_shader();
	unsigned int frag_shader = compile_and_send_fragment_shader();
	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, frag_shader);
	glLinkProgram(shader_program);
	if (!check_program_link(shader_program, "Shader program"))
		exit(-1);
	
	glDeleteShader(vertex_shader);
	glDeleteShader(frag_shader);

	while (!glfwWindowShouldClose(window))
	{
		process_input(window);

		// Render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw triangle
		glUseProgram(shader_program);
		glBindVertexArray(VAO); // No need to bind every time, since we only got 1, but
		// Params: mode, start index, vertex count
		glDrawArrays(GL_TRIANGLES, 0, 3);
		//glBindVertexArray(0); // No need to unbind every time

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// de-allocate resources
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &pos_buffer);
	glDeleteBuffers(1, &color_buffer);

	glfwTerminate();
	return 0;
}
