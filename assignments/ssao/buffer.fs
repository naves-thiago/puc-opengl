#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D gBuffer;

void main() {
	vec3 color = texture(gBuffer, TexCoords).rgb;
	//vec3 color = normalize(abs(texture(gBuffer, TexCoords).rgb));
	//vec3 color = texture(gBuffer, TexCoords).rgb;
	//float offset = 0;
	//offset = min(offset, color.r);
	//offset = min(offset, color.g);
	//offset = min(offset, color.b);
	//color = normalize(color - vec3(1.0) * offset);

	FragColor = vec4(color, 1.0);
}
