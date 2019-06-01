#version 330 core


out vec4 FragColor;

void main() {
	vec3 normal = texture(normalMap, TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(TBN * normal);

	vec3 color = vec3(1.0);

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
