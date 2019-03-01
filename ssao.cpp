#include <random>

#include "glm/vec3.hpp"

#include "textureOperations.cpp"

float lerp(float a, float b, float f) {
    return a + f * (b - a);
}

GLuint randomTexture() {
    std::uniform_real_distribution<float> floatDistribution(0.0f, 1.0f);
    std::default_random_engine generator;

    glm::vec3 *randoms = new glm::vec3[16];
    for (int i = 0; i < 16; i++) {
        randoms[i] = glm::vec3(floatDistribution(generator) * 2.0f -1.0f, floatDistribution(generator) * 2.0f -1.0f, 0);
    }

    GLuint randomTexture; 
    glGenTextures(1, &randomTexture);
    glBindTexture(GL_TEXTURE_2D, randomTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &randoms[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

    return randomTexture;
}
    
glm::vec3* generateSamplePoints(int kernelSize) {
    std::uniform_real_distribution<float> floatDistribution(0.0f, 1.0f);
    std::default_random_engine generator;

    glm::vec3 *kernel = new glm::vec3[kernelSize];
    for (int i = 0; i < kernelSize; i++) {
        glm::vec3 sample = glm::vec3(floatDistribution(generator) * 2.0f - 1.0f, floatDistribution(generator) * 2.0f - 1.0f, floatDistribution(generator));
        sample = glm::normalize(sample);
        kernel[i] = sample;
        kernel[i] = sample * floatDistribution(generator);

        // Scale so that more points are closer toward center
        float scale = (float)i / (float)kernelSize;
        kernel[i] = sample * lerp(0.1f, 1.0f, scale * scale);
    } 

    return kernel;
}
