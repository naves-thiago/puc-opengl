#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColor;

in vec3 FragPos;    // Fragment position (eye space)
in vec3 FragNormal; // Fragment normal (eye space)
in vec2 TexCoords;  // Texture coords

uniform sampler2D tex;

out vec4 FragColor;

void main() {
	gNormal = normalize(FragNormal);
	gPosition = FragPos;
	gColor = texture(tex, TexCoords);
}
