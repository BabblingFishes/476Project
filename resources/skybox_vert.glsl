#version 330 core
layout (location = 0) in vec3 vertPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 TexCoords;

void main() {
	gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);
	TexCoords = vertPos;
}
