#version 330 core
layout (location = 0) in vec3 aPos;      // Vertex Position (obj coords)
layout (location = 1) in vec3 aNormal;   // Vertex normal (obj coords)
layout (location = 2) in vec2 aTexture;  // Texture coords

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 FragNormal;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);

	TexCoords = aTexture;
	FragPos = vec3(model * vec4(aPos, 1.0)); // Convert position to world coords
	FragNormal = aNormal;
}
