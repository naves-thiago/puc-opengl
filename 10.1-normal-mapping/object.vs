#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexture;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
	vec3 FragPos;
	vec2 TexCoords;
	mat3 TBN;
} vs_out;

void main() {
	// note that we read the multiplication from right to left
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	//FragPos = vec3(model * vec4(aPos, 1.0)); // Convert position to world coords
	// Transform the normal vector to the world coords, using the normal matrix
	//Normal = mat3(transpose(inverse(model))) * aNormal;

	vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
	vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
	mat3 TBN = mat3(T, B, N);
	vs_out.TBN = TBN;
	vs_out.TexCoords = aTexture;
	vs_out.FragPos = vec3(model * vec4(aPos, 1.0)); // Convert position to world coords
}
