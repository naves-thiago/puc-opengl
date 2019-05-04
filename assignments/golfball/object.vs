#version 330 core
layout (location = 0) in vec3 aPos;      // Vertex Position (obj coords)
layout (location = 1) in vec3 aNormal;   // Vertex normal (obj coords)
layout (location = 2) in vec3 aTangent;  // Tangent vector (obj coords)
layout (location = 3) in vec2 aTexture;  // Texture coords

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
	vec3 FragPos;
	vec2 TexCoords;
	mat3 TBN;
} vs_out;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	//FragPos = vec3(model * vec4(aPos, 1.0)); // Convert position to world coords
	// Transform the normal vector to the world coords, using the normal matrix
	//Normal = mat3(transpose(inverse(model))) * aNormal;

	vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
	// re-orthogonalize T (may be needed on large meshes due to tangent averaging)
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	mat3 TBN = mat3(T, B, N);
	vs_out.TBN = TBN;
	vs_out.TexCoords = aTexture;
	vs_out.FragPos = vec3(model * vec4(aPos, 1.0)); // Convert position to world coords
}
