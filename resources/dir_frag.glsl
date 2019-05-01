#version 330 core 
in vec3 fragNor, fragPos, lightDir;


layout(location = 0) out vec4 color;

uniform vec3 lightClr;
uniform vec3 matAmb, matDif, matSpec;
uniform float shine;

void main() {


	vec3 N = normalize(fragNor);
	vec3 L = normalize(lightDir);
	vec3 V = normalize(-fragPos);
	
	vec3 ambient = matAmb;
	vec3 diffuse = matDif * max(dot(L, N), 0.0);
	vec3 specular = matSpec * pow(max(dot(normalize(L + V), N), 0.0), shine);
	
	color = vec4((specular + diffuse + ambient) * lightClr, 1.0);
}
