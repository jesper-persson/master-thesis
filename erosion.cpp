#include "textureOperations.cpp"
#include "shader.cpp"

class JumpStepOperation : public PingPongTextureOperation {
public:
    int passIndex;

    JumpStepOperation() {

    }

    JumpStepOperation(GLuint textureWidth, GLuint textureHeight, GLuint program, int numRepeat)
    : PingPongTextureOperation(textureWidth, textureHeight, program, numRepeat) {
        passIndex = 0;
    }

    void bindUniforms() override {
        glUniform1i(glGetUniformLocation(shaderProgram, "passIndex"), passIndex);
        PingPongTextureOperation::bindUniforms();
        passIndex += 1;
    }
};

class Erosion
{
  public:
    Erosion() {

    }
    TextureOperation init;
    JumpStepOperation jumpFlood;
    TextureOperation toDistanceMap;
    PingPongTextureOperation distribute;

    // 0 indicates no penetration, non zero indicates penetration
    GLuint penetrationTexture;

    // result from distribution shader program
    GLuint distributedPenetratitionTexture;

    int textureSize;

    GLuint distanceMap;

    // Step 4 (erosion)
    TextureOperation calcAvgHeight;
    TextureOperation erosionStep1;
    TextureOperation erosionStep2; // For ping pong
};

Erosion initializeErosion(int textureSize)
{
    GLuint shaderProgram1 = createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFloodStartStepFS.glsl");
    GLuint shaderProgram2 = createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFloodFS.glsl");
    GLuint shaderProgram3 = createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFloodFinalizeFS.glsl");
    GLuint shaderProgram4 = createShaderProgram("shaders/basicVS.glsl", "shaders/distributeToLowerContourValues.glsl");
    GLuint shaderProgram5 = createShaderProgram("shaders/basicVS.glsl", "shaders/erosionCalcAvgHeightFS.glsl");
    GLuint shaderProgram6 = createShaderProgram("shaders/basicVS.glsl", "shaders/erosionFS.glsl");
    

    Erosion erosion;
    erosion.textureSize = textureSize;
    erosion.init = TextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int times = log2(erosion.textureSize);
    erosion.jumpFlood = JumpStepOperation(erosion.textureSize, erosion.textureSize, shaderProgram2, times);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.toDistanceMap = TextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram3);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.distribute = PingPongTextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram4, 30);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.calcAvgHeight = TextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram5);
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.erosionStep1 = TextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram6);
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

    erosion.erosionStep2 = TextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram6);
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
    
    return erosion;
}

void buildDistanceMap(Erosion &erosion) {
    // Given a penetration texture, and a initial 0 texture, set precondition
    erosion.init.execute(erosion.penetrationTexture, 0);

    // ping pong precondition texture with jump flood
    erosion.jumpFlood.passIndex = 0;
    erosion.jumpFlood.execute(erosion.init.getTextureResult(), 0);

    // Build distance map from result of jumpfrog
    erosion.toDistanceMap.execute(erosion.jumpFlood.getTextureResult(), 0);
    erosion.distanceMap = erosion.toDistanceMap.getTextureResult();
}

void runDistribution(Erosion &erosion)
{
    // Distribute one level
    erosion.distribute.execute(erosion.penetrationTexture, erosion.distanceMap);
    erosion.distributedPenetratitionTexture = erosion.distribute.getTextureResult();
}

// Performs the last erosion step (even out high slopes)
GLuint runErosion(Erosion &erosion, GLuint heightmap, GLuint obsticleMap) {
    // int timesToRun = 100;
    // for (int i = 0; i < timesToRun; i++) {
    //     erosion.calcAvgHeight.execute(erosion.distributedPenetratitionTexture, 0);
    //     GLuint avgHeightTexture = erosion.calcAvgHeight.getTextureResult();

    //     TextureOperation to = erosion.erosionStep2;
    //     if (i % 2 == 0) {
    //         to = erosion.erosionStep1;
    //     }
    //     to.execute(erosion.distributedPenetratitionTexture, avgHeightTexture);
    //     GLuint erodedTexture = to.getTextureResult();

    //     erosion.distributedPenetratitionTexture = erodedTexture;
    // }

    int timesToRun = 1;
    for (int i = 0; i < timesToRun; i++) {
        erosion.calcAvgHeight.execute(heightmap, obsticleMap);
        GLuint avgHeightTexture = erosion.calcAvgHeight.getTextureResult();

        TextureOperation to = erosion.erosionStep2;
        if (to.getTextureResult() == heightmap) {
            to = erosion.erosionStep1;
        }
        to.execute(heightmap, avgHeightTexture);
        GLuint erodedTexture = to.getTextureResult();

        heightmap = erodedTexture;
    }

    return heightmap;
}