#ifndef KEYBOARD_HH
#define KEYBOARD_HH

#include <GLFW/glfw3.h>
#include <map>

typedef void (* Keyboard_callback)(int);

class Keyboard {
public:
	Keyboard(GLFWwindow *window) : window(window) {}

	void process_input(void) {
		for (auto it=keys.begin(); it!=keys.end(); it++) {
			Entry &e = it->second;
			bool state = glfwGetKey(window, it->first) == GLFW_PRESS;
			if (state != e.last_state) {
				e.last_state = state;
				if (state) {
					if (e.on_key_down) e.on_key_down(it->first);
				}
				else
					if (e.on_key_up) e.on_key_up(it->first);
			}
			if (state and e.while_key_down) e.while_key_down(it->first);
		}
	}

	void on_key_down(int key, Keyboard_callback cb) {
		Entry &e = keys[key];
		e.last_state = glfwGetKey(window, key) == GLFW_PRESS;
		e.on_key_down = cb;
	}

	void on_key_up(int key, Keyboard_callback cb) {
		Entry &e = keys[key];
		e.last_state = glfwGetKey(window, key) == GLFW_PRESS;
		e.on_key_up = cb;
	}

	void while_key_down(int key, Keyboard_callback cb) {
		Entry &e = keys[key];
		e.last_state = glfwGetKey(window, key) == GLFW_PRESS;
		e.while_key_down = cb;
	}

private:
	struct Entry {
		bool last_state;
		Keyboard_callback on_key_down;
		Keyboard_callback on_key_up;
		Keyboard_callback while_key_down;
	};
	std::map<int, Entry> keys;
	GLFWwindow *window;
};

#endif
