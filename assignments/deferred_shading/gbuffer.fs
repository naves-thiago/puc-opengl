#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColor;

in vec3 FragPos;   // Fragment position (eye space)
in vec2 TexCoords; // Texture coords
in mat3 TBN;

uniform sampler2D normalMap;

out vec4 FragColor;

void main() {
	gNormal = texture(normalMap, TexCoords).rgb;
	gNormal = normalize(gNormal * 2.0 - 1.0);
	gNormal = normalize(TBN * gNormal);
	gPosition = FragPos;
	gColor = vec4(1.0);
}
