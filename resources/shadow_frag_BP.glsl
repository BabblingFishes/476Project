#version 330 core
uniform sampler2D Texture0;
uniform sampler2D shadowDepth;

in vec3 fragNor;
in vec3 halfVec;
in vec3 lightDir;

in vec4 fPos;
in vec2 vTexCoord;
in vec4 fPosLS;

in vec3 vColor;

out vec4 color;

uniform vec3 lightClr;
uniform vec3 matDif, matAmb, matSpec;
uniform float shine;

/* returns 1 if shadowed */
/* called with the point projected into the light's coordinate space */
float testShadow(vec4 LSfPos) {

  float bias = 0.005;
  vec3 shifted = (LSfPos.xyz + vec3(1.0)) * 0.5; // shift coords into %
  float lightDepth = texture(shadowDepth, shifted.xy).r; //read off stored shadow depth

	//compare to projected depth, return 1 if shadowed
  if (lightDepth < shifted.z - bias) return 1.0;
	return 0.0;
}

void main() {
		vec4 texColor0 = texture(Texture0, vTexCoord);
		float shade = testShadow(fPosLS);

		//blinn-phong shading
		float fallOff = distance(lightDir, fragNor) / 25.0f;
    //float fallOff = 1; //TODO
		vec3 normal = normalize(fragNor);
    vec3 half_norm = normalize(vec3(halfVec));
    vec3 L_norm = normalize(vec3(lightDir));

    vec3 diffuseRefl = matDif * max(0, dot(normal, L_norm));
    //vec3 specularRefl = matSpec * pow(max(dot(normal, half_norm), 0.0), shine);
    vec3 specularRefl = matSpec * pow(max(dot(half_norm, normal), 0.0), shine);
    vec3 ambientRefl = matAmb;

    vec3 ReflColor = (ambientRefl + diffuseRefl + specularRefl) * lightClr / fallOff;

    //color = amb*(texColor0) + (1.0-Shade)*texColor0*BaseColor;
		//color = 0.3 * (texColor0) + (1.0 - shade) * texColor0 * vec4(vColor, 1.0);
		//TODO currently just showing blinn-phong
		color = vec4(ReflColor, 1.0);

}
