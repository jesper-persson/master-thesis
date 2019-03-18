#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "lodepng/lodepng.h"
#include "glew.h"

GLuint createTextureForPenetration(int size) {
	GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    const int w = size;
    const int h = size;
    const int length = w * h * 4;
    float *pixels = new float[length];

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int i = (y * w + x) * 4;

			float hh = 0.0f;
			if (x  > 50 && x < 150 && y > 50 && y < 150) {
				hh = 0.0f;
			}

			pixels[i] = hh;
			pixels[i + 1] = hh;
			pixels[i + 2] = hh;
			pixels[i + 3] = hh;
		}
	}

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	delete pixels;

    return textureId;
}

GLuint createTextureForHeightmap(int size) {
	GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    const int w = size;
    const int h = size;
    const int length = w * h * 4;
    float *pixels = new float[length];

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int i = (y * w + x) * 4;

			float hh = 5.0f;
			if (x  > 50 && x < 150 && y > 50 && y < 150) {
				hh = 5.0f;
			}

			pixels[i] = hh;
			pixels[i + 1] = hh;
			pixels[i + 2] = hh;
			pixels[i + 3] = hh;
		}
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