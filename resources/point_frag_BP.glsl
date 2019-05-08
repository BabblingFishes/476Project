#version 330 core
in vec3 fragNor;
in vec3 halfVec; //halfvector
in vec3 lightDir;
out vec4 color;

uniform vec3 lightClr;
uniform vec3 matDif, matAmb, matSpec;
uniform float shine;

void main() {
		float fallOff = distance(lightDir, fragNor) / 25.0f;
		vec3 normal = normalize(fragNor);
    vec3 half_norm = normalize(vec3(halfVec));
    vec3 L_norm = normalize(vec3(lightDir));

    vec3 diffuseRefl = matDif * max(0, dot(normal, L_norm));
    vec3 specularRefl = matSpec * pow(max(dot(normal, half_norm), 0.0), shine);
    vec3 ambientRefl = matAmb;

    vec3 ReflColor = (ambientRefl + diffuseRefl + specularRefl) * lightClr / fallOff;
		color = vec4(ReflColor, 1.0);
}
