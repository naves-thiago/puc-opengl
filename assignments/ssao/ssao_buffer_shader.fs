#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D ssaoInput;

void main() {
	float x = texture(ssaoInput, TexCoords).x;
	FragColor = vec4(x, x, x, 1.0);
}

