#version 330 core
layout (location = 0) in vec3 aPos;      // Vertex Position (obj space)
layout (location = 1) in vec3 aNormal;   // Vertex normal (obj space)
layout (location = 2) in vec3 aTangent;  // Tangent vector (obj space)
layout (location = 3) in vec2 aTexture;  // Texture coords

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;   // Fragment position (eye space))
out vec2 TexCoords; // Texture coords
out mat3 TBN;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);

	vec3 T = normalize(vec3(view * model * vec4(aTangent, 0.0)));
	vec3 N = normalize(vec3(view * model * vec4(aNormal, 0.0)));
	// re-orthogonalize T (may be needed on large meshes due to tangent averaging)
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	TBN = mat3(T, B, N);
	TexCoords = aTexture;
	FragPos = vec3(view * model * vec4(aPos, 1.0)); // Convert position to eye space
}
