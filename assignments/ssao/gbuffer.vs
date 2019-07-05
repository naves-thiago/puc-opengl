#version 330 core
layout (location = 0) in vec3 aPos;      // Vertex Position (obj space)
layout (location = 1) in vec3 aNormal;   // Vertex normal (obj space)
layout (location = 2) in vec2 aTexture;  // Texture coords

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;    // Fragment position (eye space)
out vec3 FragNormal; // Fragment normal (eye space)
out vec2 TexCoords;  // Texture coords

void main() {
	TexCoords = aTexture;
	FragPos = vec3(view * model * vec4(aPos, 1.0)); // Convert position to eye space
	// Transform the normal vector to the eye coords, using the normal matrix
	FragNormal = mat3(transpose(inverse(view * model))) * aNormal;
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
