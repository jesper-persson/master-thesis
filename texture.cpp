#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "lodepng/lodepng.h"
#include "glew.h"

GLuint createTextureForHeightmap(int size) {
	GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    const int w = size;
    const int h = size;
    const int length = w * h * 4;
    float *pixels = new float[length];

    for (int i = 0; i < length; i += 4) {
		float h = 0.9f;
		if (i % 50 == 0) {
			h = 0.9f;
		}
        pixels[i] = h;
        pixels[i + 1] = h;
        pixels[i + 2] = h;
        pixels[i + 3] = h;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	delete pixels;

    return textureId;
}

std::vector<float>* pngTextureToFloatArray(std::string filename) {
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filename);
	std::vector<float>* imageCopy = new std::vector<float>(width * height);
	unsigned int newBufferIndex = 0;
	unsigned int oldBufferIndex = 0;
	for (newBufferIndex = 0; newBufferIndex < height * width; newBufferIndex++, oldBufferIndex += 4) {
		(*imageCopy)[newBufferIndex]  = ( (float)image[oldBufferIndex] );
	}

	return imageCopy;
}

GLuint loadPNGTexture(std::string filename) {
	const char* filenameC = (const char*)filename.c_str();
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filename);

	std::vector<unsigned char> imageCopy(width * height * 4);
	for (unsigned i = 0; i < height; i++) {
		memcpy(&imageCopy[(height - i - 1) * width * 4], &image[i * width * 4], width * 4);
	}

	GLuint texId;
	glGenTextures(1, &texId);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &imageCopy[0]);
	glGenerateMipmap(GL_TEXTURE_2D);
	return texId;
}