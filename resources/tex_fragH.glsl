#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;
uniform int randNum;

void main() {
    color = vec4(texture( texBuf, texCoord ).rgb, 1);

    if (randNum == 0) {
    	color.r = 0.5f;
    	color.g = 0.5f;
    	color.b = 0.5f;
    }

    else if (randNum == 1) {
    	color.r = 0;
    	color.g = 0;
    	color.b = 0;
    }

    else if (randNum == 2) {
    	color.r = 0.25;
    	color.g = 0.25;
    	color.b = 0.25;
    }
}
