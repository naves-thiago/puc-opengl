#version 330 core

in vec2 TexCoords;
out float FragColor;

const int kernelSize = 64;
const float radius = 0.5;
//const float bias = 0.025;
const float bias = 0.015;

uniform sampler2D gPosition; // Fragment Positions (eye space)
uniform sampler2D gNormal;   // Fragment Normals (eye space)
uniform sampler2D texNoise;  // Noise texture

uniform vec3 samples[kernelSize];
uniform mat4 projection;
uniform vec2 noiseScale;

// tile noise over screen (this is the number of tiles on each direction)
//const vec2 noiseScale = vec2(1280.0 / 4.0, 720.0 / 4.0);
//const vec2 noiseScale = vec2(800.0 / 4.0, 600.0 / 4.0);

void main() {
	vec3 fragPos   = texture(gPosition, TexCoords).rgb;
	vec3 normal    = texture(gNormal, TexCoords).rgb;
	vec3 randomVec = texture(texNoise, TexCoords * noiseScale).rgb;

	// Create TBN to transform samples to the eye space
	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal);

	// Use each kernel sample to offset the fragment position and compare
	// the fragment depth with the sample depth
	float occlusion = 0.0;
	for (int i=0; i<kernelSize; i++) {
		// get sample position
		vec3 sample = TBN * samples[i];
		sample = fragPos + sample * radius;

		// Transform the sample to screen space so we can get the position and depth
		vec4 offset = vec4(sample, 1.0);
		offset      = projection * offset;     // view to clip-space
		offset.xyz /= offset.w;                // perspective divide
		offset.xyz  = offset.xyz * 0.5 + 0.5;  // transform to 0.0 - 1.0 range

		// Get the sample depth from the position texture
		float sampleDepth = texture(gPosition, offset.xy).z;

		// Prevent contribution from surfaces far behind the test surface
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));

		occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	// Normalize the occlusion and subtract from 1 so we can directly use it to scale
	// the ambient lighting
	occlusion = 1.0 - (occlusion / kernelSize);
	FragColor = occlusion;
}
