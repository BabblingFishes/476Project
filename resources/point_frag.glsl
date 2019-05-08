#version 330 core
in vec3 fragNor, fragPos, lightDir;

layout(location = 0) out vec4 color;

uniform vec3 lightClr;
uniform vec3 matAmb, matDif, matSpec;
uniform float shine;

void main() {
	//float fallOff = distance(lightDir, fragPos);
  float fallOff = distance(lightDir, fragPos) / 50.0f;
	vec3 N = normalize(fragNor);
	vec3 L = normalize(lightDir - fragPos);
	vec3 V = normalize(-fragPos);

	vec3 ambient = matAmb;
	float intensity = max(dot(N, L), 0.0); //previously L, N
	vec3 diffuse = matDif * intensity;
	vec3 specular = vec3(0.0);
	if(intensity > 0.0) {
		specular = matSpec * pow(max(dot(normalize(L + V), N), 0.0), shine);
	}

	vec3 tempColor = (specular + diffuse + ambient) * lightClr / fallOff;

	color = vec4(max(tempColor, vec3(0.1)), 1.0);
}
