#version 330 core

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 FragPos;    // Fragment position
in vec2 TexCoords;  // Texture coords
in mat3 TBN;

uniform float shininess;
uniform Light light;
uniform vec3 viewPos;  // Camera position
uniform sampler2D normalMap;
uniform sampler2D diffuseMap;

out vec4 FragColor;

void main() {
	vec3 normal = texture(normalMap, TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(TBN * normal);

	vec3 color = texture(diffuseMap, TexCoords).rgb;

	vec3 ambient = light.ambient * color;

	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * color;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec3 specular = spec * light.specular;;

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);
}
