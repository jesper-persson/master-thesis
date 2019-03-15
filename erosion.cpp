#include "textureOperations.cpp"
#include "shader.cpp"

class SubTextureOperation : public TextureOperation {
    // In pixels.
public:
    int activeWidth;
    int activeHeight;
    int activeCenterX;
    int activeCenterY;

    SubTextureOperation() {

    }

    SubTextureOperation(GLuint textureWidth, GLuint textureHeight, GLuint program)
    : TextureOperation(textureWidth, textureHeight, program) {

    }

    void bindUniforms() override {
        glUniform1i(glGetUniformLocation(shaderProgram, "activeHeight"), activeHeight);
        glUniform1i(glGetUniformLocation(shaderProgram, "activeWidth"), activeWidth);
        glUniform1i(glGetUniformLocation(shaderProgram, "activeCenterX"), activeCenterX);
        glUniform1i(glGetUniformLocation(shaderProgram, "activeCenterY"), activeCenterY);
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
    TextureOperation toDistanceMap;
    PingPongTextureOperation distribute;
    DistributeToContourVelocity distributeWithVelocity;

    // 0 indicates no penetration, non zero indicates penetration
    GLuint penetrationTexture;

    // result from distribution shader program
    GLuint distributedPenetratitionTexture;

    int textureSize;

    GLuint velocityMap;

    TextureOperation setAllChannels; // Get rid off

    GLuint distanceMap;

    TextureOperation copyOperation;

    // Step 4 (erosion)
    SubTextureOperation calcAvgHeight;
    SubTextureOperation erosionStep1;
    SubTextureOperation erosionStep2; // For ping pong
};

Erosion initializeErosion(int textureSize)
{
    GLuint shaderProgram1 = createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFloodStartStepFS.glsl");
    GLuint shaderProgram2 = createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFloodFS.glsl");
    GLuint shaderProgram3 = createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFloodFinalizeFS.glsl");
    GLuint shaderProgram4 = createShaderProgram("shaders/basicVS.glsl", "shaders/distributeToLowerContourValues.glsl");
    GLuint shaderProgram5 = createShaderProgram("shaders/basicVS.glsl", "shaders/erosionCalcAvgHeightFS.glsl");
    GLuint shaderProgram6 = createShaderProgram("shaders/basicVS.glsl", "shaders/erosionFS.glsl");
    GLuint shaderProgram7 = createShaderProgram("shaders/basicVS.glsl", "shaders/contourBasedOnVelocityFS.glsl");
    GLuint shaderProgram8 = createShaderProgram("shaders/basicVS.glsl", "shaders/setAllChannels.glsl");
    GLuint shaderProgram9 = createShaderProgram("shaders/basicVS.glsl", "shaders/copyTextureFS.glsl");
    

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

    erosion.distribute = PingPongTextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram4, 20);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.distributeWithVelocity = DistributeToContourVelocity(erosion.textureSize, erosion.textureSize, shaderProgram7, 20);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.setAllChannels = TextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram8);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.calcAvgHeight = SubTextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram5);
    
    erosion.copyOperation = TextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram9);
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    erosion.erosionStep1 = SubTextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram6);
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

    erosion.erosionStep2 = SubTextureOperation(erosion.textureSize, erosion.textureSize, shaderProgram6);
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
    
    return erosion;
}

void buildDistanceMap(Erosion &erosion, GLuint prevPenetration) {
    // Given a penetration texture, and a initial 0 texture, set precondition
    erosion.init.execute(erosion.penetrationTexture, prevPenetration);

    // ping pong precondition texture with jump flood
    erosion.jumpFlood.passIndex = 0;
    erosion.jumpFlood.execute(erosion.init.getTextureResult(), 0);

    // Build distance map from result of jumpfrog
    erosion.toDistanceMap.execute(erosion.jumpFlood.getTextureResult(), 0);
    erosion.distanceMap = erosion.toDistanceMap.getTextureResult();
}

void runDistribution(Erosion &erosion)
{
    // Distribute one level (USING result from jumpflood)
    erosion.distribute.execute(erosion.penetrationTexture, erosion.distanceMap);
    erosion.distributedPenetratitionTexture = erosion.distribute.getTextureResult();

    // Distribute with velocity
    // erosion.distributeWithVelocity.velocity = glm::vec3(0, 0, 1);
    // erosion.distributeWithVelocity.execute(erosion.penetrationTexture, erosion.velocityMap);
    // erosion.setAllChannels.execute(erosion.distributeWithVelocity.getTextureResult(), 0);
    // erosion.distributedPenetratitionTexture = erosion.setAllChannels.getTextureResult();
}

// Performs the last erosion step (even out high slopes)
GLuint runErosion(Erosion &erosion, GLuint heightmap, GLuint obsticleMap) {
    int timesToRun = 11; // must be odd

    float centerX = 0;
    float centerY = 0;

    float width = 100;
    float height = 100;

    erosion.calcAvgHeight.centerX = centerX;
    erosion.calcAvgHeight.centerY = centerY;
    erosion.erosionStep1.centerX = centerX;
    erosion.erosionStep1.centerY = centerY;
    erosion.erosionStep2.centerX = centerX;
    erosion.erosionStep2.centerY = centerY;

    erosion.calcAvgHeight.width = width;
    erosion.calcAvgHeight.height = height;
    erosion.calcAvgHeight.activeWidth = width;
    erosion.calcAvgHeight.activeHeight = height;
    erosion.calcAvgHeight.activeCenterX = centerX;
    erosion.calcAvgHeight.activeCenterY = centerY;

    erosion.erosionStep1.width = width;
    erosion.erosionStep1.height = height;
    erosion.erosionStep1.activeWidth = width;
    erosion.erosionStep1.activeHeight = height;
    erosion.erosionStep1.activeCenterX = centerX;
    erosion.erosionStep1.activeCenterY = centerY;

    erosion.erosionStep2.width = width;
    erosion.erosionStep2.height = height;
    erosion.erosionStep2.activeWidth = width;
    erosion.erosionStep2.activeHeight = height;
    erosion.erosionStep2.activeCenterX = centerX;
    erosion.erosionStep2.activeCenterY = centerY;

    // Copy heightmap to erosionstep2
    erosion.calcAvgHeight.doClear = false;
    erosion.erosionStep1.doClear = false;
    erosion.erosionStep2.doClear = false;
    erosion.copyOperation.fbo = erosion.erosionStep1.fbo;
    erosion.copyOperation.execute(heightmap, 0);

    for (int i = 0; i < timesToRun; i++) {
        erosion.calcAvgHeight.execute(heightmap, obsticleMap);
        GLuint avgHeightTexture = erosion.calcAvgHeight.getTextureResult();

        TextureOperation to = erosion.erosionStep1;
        if (to.getTextureResult() == heightmap) {
            to = erosion.erosionStep2;
        }
        to.execute(heightmap, avgHeightTexture);
        GLuint erodedTexture = to.getTextureResult();

        heightmap = erodedTexture;
    }

    return heightmap;
}