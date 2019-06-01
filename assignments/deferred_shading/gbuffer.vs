#version 330 core
layout (location = 0) in vec3 aPos;      // Vertex Position (obj coords)
layout (location = 1) in vec3 aNormal;   // Vertex normal (obj coords)
layout (location = 2) in vec3 aTangent;  // Tangent vector (obj coords)
layout (location = 3) in vec2 aTexture;  // Texture coords

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Light light;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

uniform vec3 viewPos;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);

	vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
	// re-orthogonalize T (may be needed on large meshes due to tangent averaging)
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	mat3 TBNi = transpose(mat3(T, B, N));
	TexCoords = aTexture;
	TangentLightPos = TBNi * light.position;
	TangentViewPos  = TBNi * viewPos;
	TangentFragPos  = TBNi * vec3(model * vec4(aPos, 1.0));
}
