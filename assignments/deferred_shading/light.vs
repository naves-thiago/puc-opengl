#version 330 core

layout (location = 0) in vec3 vPosition; // Vertex Position (obj space)
layout (location = 1) in vec2 TexCoords; // Vertex normal (obj space)

uniform sampler2D gPosition; // Fragment Positions (eye space)
uniform sampler2D gNormal;   // Fragment Normals (eye space)

//in vec3 vPosition; // Vertex Position (world space)
//in vec2 TexCoords; // Texture Coords

out vec3 FragPos;    // Fragment Position (eye space)
out vec3 FragNormal; // Fragment Normal (eye space)

void main() {
	FragPos = texture(gPosition, TexCoords).rgb;
	FragNormal = texture(gNormal, TexCoords).rgb;
	gl_Position = vec4(vPosition, 1.0);
}
