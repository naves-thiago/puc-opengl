#version 330 core

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in VS_OUT {
	vec3 FragPos;
	vec2 TexCoords;
	mat3 TBN;
} fs_in;

uniform float shininess;
uniform Light light;
uniform vec3 viewPos;
uniform sampler2D normalMap;
uniform sampler2D diffuseMap;

out vec4 FragColor;

void main() {
	//vec3 normal = vec3(0.0, 0.0, 1.0);
	vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(fs_in.TBN * normal);

	vec3 color = vec3(texture(diffuseMap, fs_in.TexCoords));
	//vec3 color = vec3(texture(normalMap, fs_in.TexCoords));
	//vec3 color = vec3(1.0, 0.0, 0.0);

	vec3 ambient = light.ambient * color;

	vec3 lightDir = normalize(light.position - fs_in.FragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * color;

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec3 specular = spec * light.specular; // * vec3(1.0);

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);
}
