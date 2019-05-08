#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 fragNor;
out vec3 halfVec;
out vec3 lightDir;
uniform vec3 lightPos;

void main() {
  gl_Position = P * V * M * vertPos;

  fragNor = (M * vec4(vertNor, 0.0)).xyz;

  lightDir = lightPos - (M * vertPos).xyz;
  vec3 eye = -((V * M * vertPos).xyz);
  halfVec = (normalize(eye) + normalize(lightDir)) / 2;
}
