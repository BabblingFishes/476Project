#version 330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 LS;

uniform vec3 lightPos;
uniform vec3 camPos;

out vec3 fragNor;
out vec3 halfVec;
out vec3 lightDir;

out vec4 fPos;
out vec2 vTexCoord;
out vec4 fPosLS;

out vec3 vColor;


void main() {
  gl_Position = P * V * M * vertPos; //model transforms

  fragNor = (M * vec4(vertNor, 0.0)).xyz; //normal

  vec3 fragPos = (M * vertPos).xyz;

  lightDir = lightPos - fragPos;

  vec3 eye = camPos - fragPos;
  halfVec = normalize(normalize(eye) + normalize(lightDir));

  fPos = M * vertPos; //worldspace pos
  vTexCoord = vertTex; //texture coords
  fPosLS = LS * fPos; //light space coords

  vColor = vec3(max(dot(fragNor, normalize(lightDir)), 0));
}
