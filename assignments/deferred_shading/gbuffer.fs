#version 330 core

struct Light {
	vec3 position; // Light position (world space)
	vec3 ambient;  // Ambient color
	vec3 diffuse;  // Diffuse color
	vec3 specular; // Specular color
};

in vec3 FragPos;   // Fragment position (eye space)
in vec2 TexCoords; // Texture coords
in mat3 TBN;

uniform float shininess;
uniform Light light;
uniform sampler2D normalMap;
uniform mat4 view;

out vec4 FragColor;

void main() {
	vec3 normal = texture(normalMap, TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(TBN * normal);

	vec3 color = vec3(1.0);

	vec3 ambient = light.ambient * color;

	vec3 lightPos = vec3(view * vec4(light.position, 1.0));
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * color;

	vec3 viewDir = normalize(-FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec3 specular = spec * light.specular;

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);
}
