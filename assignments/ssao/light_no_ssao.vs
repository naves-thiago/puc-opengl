#version 330 core

layout (location = 0) in vec3 vPosition;  // Vertex Position (obj space)
layout (location = 1) in vec2 vTexCoords; // Vertex normal (obj space)

out vec2 TexCoords;

void main() {
	TexCoords = vTexCoords;
	gl_Position = vec4(vPosition, 1.0);
}
