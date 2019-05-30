
#pragma once
#ifndef LAB471_GLTEXTUREWRITER_H_INCLUDED
#define LAB471_GLTEXTUREWRITER_H_INCLUDED

#include <string>
#include <memory>
#include "tTexture.h"
#include <GLFW/glfw3.h>


/**
 * GLTextureWriter outputs a three channel (GL_RGB)
 * image to a png file given by file name.
 *
 * Contact kpidding@calpoly.edu for any support questions!
 */

namespace GLTextureWriter
{
	bool WriteImage(std::shared_ptr<tTexture> texture, std::string fileName);
	bool WriteImage(const tTexture & texture, std::string fileName);
	bool WriteImage(GLint textureHandle, std::string fileName);
}

#endif // LAB471_GLTEXTUREWRITER_H_INCLUDED
