#include <texture.hh>
#include <stb_image.h>
#include <iostream>

Texture2D::Texture2D(const std::string &path, unsigned int location,
			bool verticalFlip, unsigned int wrapS,
			unsigned int wrapT) : location(GL_TEXTURE0 + location) {
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(verticalFlip);
	unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
				nrChannels == 4 ? GL_RGBA : GL_RGB,
				GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture " << path << std::endl;
	}
	stbi_image_free(data);
}

void Texture2D::bind(void) const {
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture2D::activateAndBind(void) const {
	glActiveTexture(location);
	glBindTexture(GL_TEXTURE_2D, ID);
}
