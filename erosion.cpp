#include "textureOperations.cpp"
#include "shader.cpp"

class JumpFloodFinalize : public TextureOperation {
public:
    float compressionRatio;

    JumpFloodFinalize() {

    }

    JumpFloodFinalize(GLuint textureWidth, GLuint textureHeight, GLuint program)
    : TextureOperation(textureWidth, textureHeight, program) {
        compressionRatio = 1;
    }

    void bindUniforms() override {
        glUniform1f(glGetUniformLocation(shaderProgram, "compressionRatio"), compressionRatio);
        TextureOperation::bindUniforms();
    }
};

class ErosionOperation : public TextureOperation {
public:
    float terrainSize;
    float slopeThreshold;
    float roughness;

    ErosionOperation() {

    }

    ErosionOperation(GLuint textureWidth, GLuint textureHeight, GLuint program)
    : TextureOperation(textureWidth, textureHeight, program) {
        terrainSize = 1;
    }

    void bindUniforms() override {
        glUniform1f(glGetUniformLocation(shaderProgram, "terrainSize"), terrainSize);
        glUniform1f(glGetUniformLocation(shaderProgram, "slopeThreshold"), slopeThreshold);
        glUniform1f(glGetUniformLocation(shaderProgram, "roughness"), roughness);
        TextureOperation::bindUniforms();
    }
};

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

class DistributeToContourVelocity : public PingPongTextureOperation {
public:
    glm::vec3 velocity;

    DistributeToContourVelocity() {

    }

    DistributeToContourVelocity(GLuint textureWidth, GLuint textureHeight, GLuint program, int numRepeat)
    : PingPongTextureOperation(textureWidth, textureHeight, program, numRepeat) {
        velocity = glm::vec3(0, 0, 0);
    }

    void bindUniforms() override {
        glUniform3f(glGetUniformLocation(shaderProgram, "velocity"), velocity.x, velocity.y, velocity.z);
        PingPongTextureOperation::bindUniforms();
    }
};

class Erosion
{
  public:
    Erosion() {

    }
    TextureOperation init;
    JumpStepOperation jumpFlood;
    JumpFloodFinalize toDistanceMap;
    PingPongTextureOperation distribute;
    DistributeToContourVelocity distributeWithVelocity;

    // 0 indicates no penetration, non zero indicates penetration
    GLuint penetrationTexture;

    // result from distribution shader program
    GLuint distributedPenetratitionTexture;

    int textureSize;

    GLuint velocityMap;

    TextureOperation setChannels; // Get rid off

    GLuint distanceMap;

    TextureOperation copyOperation;

    // Step 4 (erosion)
    ErosionOperation calcAvgHeight;
    ErosionOperation erosionStep1;
    ErosionOperation erosionStep2; // For ping pong
};

Erosion initializeErosion(int textureSize, int numTimesToRunDistributeToCoutour, float compressionRatio)
{
    GLuint shaderProgram1 = createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFloodStartStepFS.glsl");
    GLuint shaderProgram2 = createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFloodFS.glsl");
    GLuint shaderProgram3 = createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFloodFinalizeFS.glsl");
    GLuint shaderProgram4 = createShaderProgram("shaders/basicVS.glsl", "shaders/distributeToLowerContourValues.glsl");
    GLuint shaderProgram5 = createShaderProgram("shaders/basicVS.glsl", "shaders/erosionCalcAvgHeightFS.glsl");
    GLuint shaderProgram6 = createShaderProgram("shaders/basicVS.glsl", "shaders/erosionFS.glsl");
    GLuint shaderProgram7 = createShaderProgram("shaders/basicVS.glsl", "shaders/contourBasedOnVelocityFS.glsl");
    GLuint shaderProgram8 = createShaderProgram("shaders/basicVS.glsl", "shaders/setChannels.glsl");
    GLuint shaderProgram9 = createShaderProgram("shaders/basicVS.glsl", "shaders/copyTextureFS.glsl");
    

    Erosion erosion;
    erosion.textureSize = textureSize;
    erosion.init = TextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram1);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int times = log2(erosion.textureSize);
    erosion.jumpFlood = JumpStepOperation(erosion.textureSize, erosion.textureSize, shaderProgram2, times);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.toDistanceMap = JumpFloodFinalize(erosion.textureSize, erosion.textureSize, shaderProgram3);
    erosion.toDistanceMap.compressionRatio = compressionRatio;
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.distribute = PingPongTextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram4, numTimesToRunDistributeToCoutour);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.distributeWithVelocity = DistributeToContourVelocity(erosion.textureSize, erosion.textureSize, shaderProgram7, numTimesToRunDistributeToCoutour);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.setChannels = TextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram8);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.calcAvgHeight = ErosionOperation(erosion.textureSize, erosion.textureSize, shaderProgram5);
    
    erosion.copyOperation = TextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram9);
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.erosionStep1 = ErosionOperation(erosion.textureSize, erosion.textureSize, shaderProgram6);
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

    erosion.erosionStep2 = ErosionOperation(erosion.textureSize, erosion.textureSize, shaderProgram6);
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
    
    return erosion;
}

void buildDistanceMap(Erosion &erosion, GLuint prevPenetration, ActiveArea& activeArea) {
    erosion.init.doClear = false;
    erosion.jumpFlood.doClear = false;
    erosion.toDistanceMap.doClear = false;

    erosion.init.activeArea = activeArea;
    erosion.jumpFlood.activeArea = activeArea;
    erosion.toDistanceMap.activeArea = activeArea;

    // Given a penetration texture, and a initial 0 texture, set precondition
    erosion.init.execute(erosion.penetrationTexture, prevPenetration);

    // ping pong precondition texture with jump flood
    erosion.jumpFlood.passIndex = 0;
    erosion.jumpFlood.execute(erosion.init.getTextureResult(), 0);

    // Build distance map from result of jumpfrog
    erosion.toDistanceMap.execute(erosion.jumpFlood.getTextureResult(), erosion.penetrationTexture);
    erosion.distanceMap = erosion.toDistanceMap.getTextureResult();
}

void runDistribution(Erosion &erosion, ActiveArea& activeArea) {
    erosion.distribute.activeArea = activeArea;
    erosion.distribute.doClear = false;

    // Distribute one level (USING result from jumpflood)
    erosion.distribute.execute(erosion.distanceMap, 0);
    erosion.distributedPenetratitionTexture = erosion.distribute.getTextureResult();
}

// Performs the last erosion step (even out high slopes)
void runErosion(Erosion &erosion, GLuint heightmap, GLuint obsticleMap, ActiveArea& activeArea, int numErosionStepsPerFrame) {
    int timesToRun = numErosionStepsPerFrame;

    if (timesToRun % 2 == 0) {
        cout << "Must be odd" << endl;
    }

    erosion.calcAvgHeight.activeArea = activeArea;
    erosion.erosionStep1.activeArea = activeArea;
    erosion.erosionStep2.activeArea = activeArea;

    erosion.calcAvgHeight.doClear = false;
    erosion.erosionStep1.doClear = false;
    erosion.erosionStep2.doClear = false;

    // Combine heightmap and obsticle map to one
    erosion.setChannels.doClear = false;
    erosion.setChannels.activeArea = activeArea;
    erosion.setChannels.execute(heightmap, obsticleMap);

    GLuint calcAvgHeightInput = erosion.setChannels.getTextureResult();

    for (int i = 0; i < timesToRun; i++) {
        erosion.calcAvgHeight.execute(calcAvgHeightInput, 0);
        GLuint avgHeightTexture = erosion.calcAvgHeight.getTextureResult();

        ErosionOperation to = erosion.erosionStep1;
        if (to.getTextureResult() == heightmap) {
            to = erosion.erosionStep2;
        }
        to.execute(avgHeightTexture, 0);
        GLuint erodedTexture = to.getTextureResult();

        calcAvgHeightInput = erodedTexture;

        heightmap = erodedTexture;
    }
}