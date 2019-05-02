#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

#include "lodepng/lodepng.h"
#include "glew.h"

#include "fbo.cpp"

using namespace std;

extern const int heightColumnScale;

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
    const int length = w * h;
    unsigned int *pixels = new unsigned int[length];

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int i = (y * w + x);

			// nt hh = 5;
			// if (x  > 50 && x < 150 && y > 50 && y < 150) {
			// 	hh = 5.0f;
			// }

			pixels[i] = 5 * heightColumnScale;
		}
	}

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, w, h, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, pixels);
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
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filename);

	std::vector<unsigned char> imageCopy(width * height * 4);
	for (unsigned i = 0; i < height; i++) {
		memcpy(&imageCopy[(height - i - 1) * width * 4], &image[i * width * 4], width * 4);
	}

	// TODO::: MULTIPLY HERE FROM 0:1 FLOAT RANGE TO INTEGER RANGE

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

void readBackAndAccumulatePixelValue(GLuint fboId, int textureSize, TextureFormat textureFormat) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
    int x = 0;
    int y = 0;
    int width = textureSize;
    int height = textureSize;

	if (textureFormat == TextureFormat::RGBA16F) {
		int pixelSize = 4 * 4; // 4 bytes and 4 channels
		GLfloat *data = (GLfloat*) malloc(pixelSize * width * height);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(x, y, width, height, GL_RGBA, GL_FLOAT, data	);

		int numReads = 0;
		float sum = 0;
		float sample = 0;
		for (int i = 0; i < width * height * 4; i+=4) {
			numReads++;
			sum += data[i];
			if (data[i] > 0 || data[i] < 0){
				sample = data[i];
			}
		}
		cout << "Total reads: " << numReads << ", sum: " << setprecision(20) << fixed << sum << " sample: " << sample << endl;
		free(data);

	} else if (textureFormat == TextureFormat::R32UI) {

		int pixelSize = 4 * 1; // 4 bytes and 4 channels
		GLuint *data = (GLuint*) malloc(pixelSize * width * height);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(x, y, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, data	);

		int numReads = 0;
		unsigned int sum = 0;
		float sample = 0;
		for (int i = 0; i < width * height; i++) {
			numReads++;
			sum += data[i];
			if (data[i] > 0 || data[i] < 0){
				sample = data[i];
			}
			if (data[i] != 5000) {
				// cout << data[i] << " not 5000" << endl;
			}
		}
		cout << "Total reads: " << numReads << ", sum: " << setprecision(20) << fixed << sum << " sample: " << sample << endl;
		free(data);
	} else if (textureFormat == TextureFormat::RGBA32UI) {

		int pixelSize = 4 * 4; // 4 bytes and 4 channels
		GLuint *data = (GLuint*) malloc(pixelSize * width * height);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(x, y, width, height, GL_RGBA_INTEGER, GL_UNSIGNED_INT, data	);

		int numReads = 0;
		unsigned int sumA = 0;
		unsigned int sumB = 0;
		unsigned int sumC = 0;
		float sample = 0;
		
		for (int i = 0; i < width * height * 4; i+=4) {
			numReads++;
			sumA += data[i];
			sumB += data[i+1] * data[i+2];
			sumC += data[i+2];
		}
		cout << "sumA " << fixed << sumA << ", sumB " << fixed << sumB << ", sumC " << fixed << sumC << endl;
		// cout << "avg Total reads: " << numReads << ", sum: " << setprecision(20) << fixed << sum << " sample: " << sample << endl;
		free(data);
	}

}