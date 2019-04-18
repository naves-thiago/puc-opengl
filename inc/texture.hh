#ifndef TEXTURE_HH
#define TEXTURE_HH

#include <glad/glad.h>
#include <string>

class Texture2D {
public:
	unsigned int ID;
	Texture2D(const std::string &path, unsigned int wrapS = GL_REPEAT,
			unsigned int wrapT = GL_REPEAT);

	void bind(void) const;
};

#endif
