#version 330 core

struct Light {
	vec3 position; // Not used in the fragment
	vec3 ambient;  // Ambient color
	vec3 diffuse;  // Diffuse color
	vec3 specular; // Specular color
};

in vec2 TexCoords;       // Texture coords
in vec3 TangentLightPos; // Light Position (tangent space)
in vec3 TangentFragPos;  // Fragment Position (tangent space)

uniform float shininess;
uniform Light light;
uniform sampler2D normalMap;

out vec4 FragColor;

void main() {
	vec3 normal = texture(normalMap, TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 color = vec3(1.0);

	vec3 ambient = light.ambient * color;

	vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * color;

	vec3 viewDir = normalize(-TangentFragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec3 specular = spec * light.specular;;

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);
}
