#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D gBuffer;

void main() {
	vec3 color = texture(gBuffer, TexCoords).rgb;
	FragColor = vec4(color, 1.0);
}
