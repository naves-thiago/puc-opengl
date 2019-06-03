#version 330 core

struct Light {
	vec3 position; // Light position (world space)
	vec3 ambient;  // Ambient color
	vec3 diffuse;  // Diffuse color
	vec3 specular; // Specular color
};

in vec2 TexCoords;
//in vec3 FragNormal;
//in vec3 FragPos;
out vec4 FragColor;

uniform sampler2D gPosition; // Fragment Positions (eye space)
uniform sampler2D gNormal;   // Fragment Normals (eye space)

uniform mat4 view;
uniform Light light;
uniform float shininess;

void main() {
	vec3 position = texture(gPosition, TexCoords).rgb;
	vec3 FragNormal = texture(gNormal, TexCoords).rgb;
	//FragColor = vec4(FragNormal, 1.0);
//	FragColor = vec4(TexCoords, 0.0, 1.0);
//
	vec3 color = vec3(1.0);

	vec3 ambient = light.ambient * color;

	vec3 lightPos = vec3(view * vec4(light.position, 1.0));
	vec3 lightDir = normalize(lightPos - position);
	float diff = max(dot(FragNormal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * color;

	vec3 viewDir = normalize(-position);
	vec3 reflectDir = reflect(-lightDir, FragNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec3 specular = spec * light.specular;

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);

	// TEST //
//	FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

//
//	vec3 normal = texture(normalMap, TexCoords).rgb;
//	normal = normalize(normal * 2.0 - 1.0);
//	normal = normalize(TBN * normal);
//
//	vec3 color = vec3(1.0);
//
//	vec3 ambient = light.ambient * color;
//
//	vec3 lightDir = normalize(light.position - FragPos);
//	float diff = max(dot(normal, lightDir), 0.0);
//	vec3 diffuse = diff * light.diffuse * color;
//
//	vec3 viewDir = normalize(viewPos - FragPos);
//	vec3 reflectDir = reflect(-lightDir, normal);
//	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
//	vec3 specular = spec * light.specular;;
//
//	vec3 result = ambient + diffuse + specular;
//	FragColor = vec4(result, 1.0);
//}
