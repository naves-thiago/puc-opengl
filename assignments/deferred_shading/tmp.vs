#version 330 core
layout (location = 0) in vec3 aPos;      // Vertex Position (obj space)
layout (location = 1) in vec2 aTex;      // Texture Coords

out vec3 pos;

void main() {
	pos.x = (aPos.x + 1.0) / 2.0;
	pos.y = (aPos.y + 1.0) / 2.0;
	//pos.z = (aPos.z + 1.0) / 2.0;
	pos.z = 0.0;
	gl_Position = vec4(aPos, 1.0);;
}
