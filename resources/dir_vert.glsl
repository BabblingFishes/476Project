#version 330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform vec3 lightPos;

uniform mat4 P, V, M;
out vec3 fragPos, fragNor, lightDir;

void main() {
	gl_Position = P * V * M * vertPos;
	fragPos = vec3(V * M * vertPos);
	fragNor = vec3(V * M * vec4(vertNor, 0.0));
	lightDir = vec3(V * vec4(lightPos, 1.0));
}