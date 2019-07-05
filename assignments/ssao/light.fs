#version 330 core

struct Light {
	vec3 position; // Light position (world space)
	vec3 ambient;  // Ambient color
	vec3 diffuse;  // Diffuse color
	vec3 specular; // Specular color
};

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D gPosition; // Fragment Positions (eye space)
uniform sampler2D gNormal;   // Fragment Normals (eye space)
uniform sampler2D gColor;    // Fragment colors

const int LIGHT_COUNT = 20;

uniform mat4 view;
uniform Light lights[LIGHT_COUNT];
uniform float shininess;

void main() {
	vec3 FragPos = (texture(gPosition, TexCoords).rgb);
	vec3 FragNormal = (texture(gNormal, TexCoords).rgb);

	vec3 color = texture(gColor, TexCoords).rgb;

	vec3 result = vec3(0.0, 0.0, 0.0);

	for (int i=0; i<LIGHT_COUNT; i++) {
		vec3 ambient = lights[i].ambient * color;

		vec3 lightPos = vec3(view * vec4(lights[i].position, 1.0));
		vec3 lightDir = normalize(lightPos - FragPos);
		float diff = max(dot(FragNormal, lightDir), 0.0);
		vec3 diffuse = diff * lights[i].diffuse * color;

		vec3 viewDir = normalize(-FragPos);
		vec3 reflectDir = reflect(-lightDir, FragNormal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
		vec3 specular = spec * lights[i].specular;

		result = result + ambient + diffuse + specular;
	}
	FragColor = vec4(result, 1.0);
}
