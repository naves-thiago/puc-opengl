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
	 0.5,  0.5, 0,  // top right
	 0.5, -0.5, 0,  // bottom right
	-0.5, -0.5, 0,  // bottom left
	-0.5,  0.5, 0,  // top left
};

unsigned int indices[] = {
	0, 1, 3,
	1, 2, 3,
};

unsigned int VBO, EBO, VAO;
void send_vertices()
{
	// Create VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create vertex buffer
	glGenBuffers(1, &VBO); // Create buffer ID
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Copy the data. GL_STATIC_DRAW -> Data likely wont change
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Create index buffer
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Uses the last buffer associated with GL_ARRAY_BUFFER
	// Params: attribute location, attribute size, type, normalize, stride, start position (in the buffer)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0); // param: attribute location
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
		"void main()\n"
		"{\n"
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
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
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

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Filled mode (default)
	
	while (!glfwWindowShouldClose(window))
	{
		process_input(window);

		// Render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw triangles
		glUseProgram(shader_program);
		glBindVertexArray(VAO); // No need to bind every time, since we only got 1, but
		// Params: mode, vertex count, type of index vector, offset in the index vector
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// Params: mode, start index, vertex count
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		//glBindVertexArray(0); // No need to unbind every time

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// de-allocate resources
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();
	return 0;
}
