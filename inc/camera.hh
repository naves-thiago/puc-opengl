#ifndef CAMERA_HH
#define CAMERA_HH

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

enum Direction {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
};

class GenericCamera {
	float pitch;
	float yaw;
	float fov;
	float max_fov;
	float min_fov;
	float move_speed;
	float look_sensitivity;
	float zoom_sensitivity;
	float aspect_ratio;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;

	void update_vectors(void) {
		glm::vec3 new_front;
		new_front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		new_front.y = sin(glm::radians(pitch));
		new_front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		front = glm::normalize(new_front);
		// Also re-calculate the right and up vectors
		// Normalize the vectors, because their length gets closer to 0 the
		// more you look up or down which results in slower movement.
		//right = glm::normalize(glm::cross(front, up));
		right = glm::normalize(glm::cross(front, world_up));
		up    = glm::normalize(glm::cross(right, front));
	}

	void _set_pitch(float value) {
		if (value < -89.0f)
			pitch = -89.0f;
		else if (value > 89.0f)
			pitch = 89.0f;
		else
			pitch = value;
	}

	void _set_yaw(float value) {
		yaw = value;
	}

public:
	glm::vec3 position;

	GenericCamera(float aspect_ratio) : aspect_ratio(aspect_ratio) {
		pitch   = 0.0f;
		yaw     = -90.0f;
		fov     = 45.0f;
		max_fov = 45.0f;
		min_fov = 1.0f;
		move_speed       = 4.0f;
		look_sensitivity = 0.05f;
		zoom_sensitivity = 1.0f;
		position = glm::vec3(0.0f, 0.0f,  3.0f);
		front    = glm::vec3(0.0f, 0.0f, -1.0f);
		up       = glm::vec3(0.0f, 1.0f,  0.0f);
		right    = glm::vec3(1.0f, 0.0f,  0.0f);
		world_up = glm::vec3(0.0f, 1.0f,  0.0f);
	}

	glm::mat4 view_matrix(void) {
		return glm::lookAt(position, position + front, up);
	}

	glm::mat4 projection_matrix(void) {
		return glm::perspective(glm::radians(fov), aspect_ratio, 0.1f, 100.0f);
	}

	void look(float xoffset, float yoffset) {
		xoffset = xoffset * look_sensitivity;
		yoffset = yoffset * look_sensitivity;
		_set_yaw(yaw + xoffset);
		_set_pitch(pitch + yoffset);
		update_vectors();
	}

	void move(Direction dir, float delta_time) {
		float offset = move_speed * delta_time;
		switch (dir) {
		case FORWARD:  position += front * offset; break;
		case BACKWARD: position -= front * offset; break;
		case LEFT: position  -= right * offset; break;
		case RIGHT: position += right * offset; break;
		case DOWN: position  -= up * offset; break;
		case UP: position += up * offset; break;
		}
	}

	void zoom(float offset) {
		set_fov(fov - offset * zoom_sensitivity);
	}

	void set_aspect_ratio(float value) {
		aspect_ratio = value;
	}

	void set_fov_limits(float fmin, float fmax) {
		min_fov = fmin;
		max_fov = fmax;
	}

	void set_fov(float value) {
		if (value < min_fov)
			fov = min_fov;
		else if (value > max_fov)
			fov = max_fov;
		else
			fov = value;
	}

	void set_pitch(float value) {
		_set_pitch(value);
		update_vectors();
	}

	void set_yaw(float value) {
		_set_yaw(value);
		update_vectors();
	}

	void set_move_seed(float value) {
		move_speed = value;
	}

	void set_look_sensitivity(float value) {
		look_sensitivity = value;
	}

	void set_zoom_sensitivity(float value) {
		zoom_sensitivity = value;
	}

	void set_pos(glm::vec3 pos, float pitch, float yaw, float fov) {
		this->position = pos;
		_set_pitch(pitch);
		_set_yaw(yaw);
		set_fov(fov);
		update_vectors();
	}

	void set_pos(float x, float y, float z, float pitch, float yaw, float fov) {
		set_pos(glm::vec3(x, y, z), pitch, yaw, fov);
	}

	void set_front(glm::vec3 value) {
		front = value;
	}

	void set_front(float x, float y, float z) {
		set_front(glm::vec3(x, y, z));
	}

	void set_up(glm::vec3 value) {
		up = value;
	}

	void set_up(float x, float y, float z) {
		up = glm::vec3(x, y, z);
	}
};

class Camera : public GenericCamera {
	bool first_mouse;     // First mouse position call
	float last_x, last_y; // Last mouse position
	float delta_time;     // Time between current frame and last frame
	float last_frame;     // Time of the last frame
	glm::vec3 default_pos;
	float default_pitch;
	float default_yaw;
	float default_fov;

public:
	Camera(float aspect_ratio) : GenericCamera(aspect_ratio) {
		first_mouse = true;
		delta_time = 0.0f;
		last_frame = 0.0f;
		default_pos = glm::vec3(0.0f, 0.0f, 3.0f);
		default_pitch = 0.0f;
		default_yaw = -90.0f;
		default_fov = 45.0f;
	}


	void set_default_pos(glm::vec3 pos, float pitch, float yaw, float fov) {
		default_pos = pos;
		default_pitch = pitch;
		default_yaw = yaw;
		default_fov = fov;
	}

	void set_default_pos(float x, float y, float z, float pitch, float yaw, float fov) {
		set_default_pos(glm::vec3(x, y, z), pitch, yaw, fov);
	}

	void mouse_move(float xpos, float ypos) {
		if (first_mouse) {
			last_x = xpos;
			last_y = ypos;
			first_mouse = false;
			return;
		}

		float xoffset = xpos - last_x;
		float yoffset = last_y - ypos; // reversed y
		last_x = xpos;
		last_y = ypos;
		look(xoffset, yoffset);
	}

	// Set next mouse position in mouse_move() as the reference
	void zero_mouse(void) {
		first_mouse = true;
	}

	void key_press(GLFWwindow *window)
	{
		float current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			move(FORWARD, delta_time);

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			move(BACKWARD, delta_time);

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			move(LEFT, delta_time);

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			move(RIGHT, delta_time);

		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
			move(UP, delta_time);

		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
			move(DOWN, delta_time);

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			set_pos(default_pos, default_pitch, default_yaw, default_fov);
		}
	}
};

#endif
