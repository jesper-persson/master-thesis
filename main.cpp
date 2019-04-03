#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include "glew.h"
#include "glfw3.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/vec3.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/euler_angles.hpp"

const int WINDOW_HEIGHT = 1200;
const int WINDOW_WIDTH = 1800;
const int occlusionTextureWidth = WINDOW_WIDTH / 10.0;
const int occlusionTextureHeight = WINDOW_HEIGHT / 10.0;

const float snowCoverMaxHeight = 10.0f;
const float terrainSize = 50.0f; // World space size of terrain mesh.
const int numVerticesPerRow = 60; // Resolution of terrain mesh.
// Tesselation is manually set in Tessellation control shader.

//30 -> 300

const float textureSizeSnowHeightmap = 600;
const float boundingBoxMargin = 4.2f;

const float compression = 1.0f; // 1 nothing compressed, 0 everything compressed
const float roughness = 0.3f;
const float slopeThreshold = 2.9f;
const int numErosionStepsPerFrame = 31;

const int numBlurNormals = 4; 

const int numTimesToRunDistributeToCoutour = 5;

#include "texture.cpp"
#include "shader.cpp"
#include "box.cpp"
#include "quad.cpp"
#include "terrain.cpp"
#include "fbo.cpp"
#include "animatedFootsteps.cpp"
#include "ssao.cpp"
#include "textureOperations.cpp"
#include "erosion.cpp"
#include "timing.cpp"

using namespace std;

map<int, bool> keysDown{};
map<int, bool> keysPressed{};

bool isKeyDown(int key) {
    return keysDown[key];
}

bool isKeyPressed(int key) {
    return keysPressed[key];
}

class Camera {
public:
    glm::vec3 position = glm::vec3(0.0f, 10.0f, 10.0f);
    glm::vec3 up = glm::vec3(0, 1.0f, 0);
    glm::vec3 forward = glm::normalize(glm::vec3(0.0f, 0, -1.0f));
};

void updateCamera(Camera &camera, float dt) {
    float speed = 15.0f * dt;
    float rotationSpeed = speed / 8.0f;
    glm::vec3 left = glm::cross(camera.up, camera.forward);
    if (isKeyDown(GLFW_KEY_W)) {
        camera.position = camera.position + camera.forward * speed;
    }
    if (isKeyDown(GLFW_KEY_S)) {
        camera.position = camera.position - camera.forward * speed;
    }
    if (isKeyDown(GLFW_KEY_A)) {
        camera.position = camera.position + left * speed;
    }
    if (isKeyDown(GLFW_KEY_D)) {
        camera.position = camera.position - left * speed;
    }
    if (isKeyDown(GLFW_KEY_LEFT)) {
        camera.forward = glm::normalize(glm::rotate(camera.forward, rotationSpeed, glm::vec3(0, 1, 0)));
        camera.up = glm::normalize(glm::rotate(camera.up, rotationSpeed, glm::vec3(0, 1, 0)));
    }
    if (isKeyDown(GLFW_KEY_RIGHT)) {
        camera.forward = glm::normalize(glm::rotate(camera.forward, -rotationSpeed, glm::vec3(0, 1, 0)));
        camera.up = glm::normalize(glm::rotate(camera.up, -rotationSpeed, glm::vec3(0, 1, 0)));
    }
    if (isKeyDown(GLFW_KEY_UP)) {
        camera.forward = glm::normalize(glm::rotate(camera.forward, rotationSpeed, left));
        camera.up = glm::normalize(glm::rotate(camera.up, rotationSpeed, left));
    }
    if (isKeyDown(GLFW_KEY_DOWN)) {
        camera.forward = glm::normalize(glm::rotate(camera.forward, -rotationSpeed, left));
        camera.up = glm::normalize(glm::rotate(camera.up, -rotationSpeed, left));
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        keysDown[key] = true;
        keysPressed[key] = true;
    }
    if (action == GLFW_RELEASE) {
        keysDown[key] = false;
    }
}

void setActiveAreaForObject(glm::vec3 &terrainOrigin, float terrainSize, glm::vec3& positionOfObject, glm::vec3& boundingBoxSize, vector<ActiveArea> &activeAreas) {
    float ratio = textureSizeSnowHeightmap / terrainSize;
    glm::vec3 diff = (positionOfObject - terrainOrigin) * ratio;
    int size = boundingBoxSize.x * ratio * boundingBoxMargin;
    if (size % 2 != 0) {
        size += 1;
    }
    activeAreas.push_back(ActiveArea(diff.x, diff.z, size, size));
}

int main()
{
    // Set up OpenGL
    if (!glfwInit())
    {
        cerr << "glfwInit() failed" << endl;
    }

    string title = "Master thesis";
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title.c_str(), NULL, NULL);
    if (!window)
    {
        cerr << "Failed to create window" << endl;
    }

    glfwSetKeyCallback(window, keyCallback);
    glfwMakeContextCurrent(window);
    glewInit();
    glfwSwapInterval(0);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // glDisable(GL_CULL_FACE);

    float aspectRatio = (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT;

    GLuint shaderProgram = createShaderProgram("shaders/basicVS.glsl", "shaders/basicFS.glsl");
    GLuint shaderProgramTerrain = createShaderProgram("shaders/terrainVS.glsl", "shaders/terrainTCS.glsl", "shaders/terrainTES.glsl", "shaders/terrainFS.glsl");
    GLuint shaderProgramQuad = createShaderProgram("shaders/basicVS.glsl", "shaders/quadFS.glsl");

    // Occlusion shaders
    GLuint shaderProgramOcclusionStep = createShaderProgram("shaders/basicVS.glsl", "shaders/occlusionStep.glsl");
    GLuint shaderProgramOcclusionStepTerrain = createShaderProgram("shaders/terrainVS.glsl", "shaders/terrainTCS.glsl", "shaders/terrainTES.glsl", "shaders/occlusionStepTerrainFS.glsl");

    // Texture operation shaders
    GLuint shaderProgramLowpass = createShaderProgram("shaders/basicVS.glsl", "shaders/blurFS.glsl");
    GLuint shaderProgramPixelMin = createShaderProgram("shaders/basicVS.glsl", "shaders/minFS.glsl");
    GLuint shaderProgramCalculateNormals = createShaderProgram("shaders/basicVS.glsl", "shaders/calculateNormalsFS.glsl");
    GLuint shaderProgramSubtract = createShaderProgram("shaders/basicVS.glsl", "shaders/subtractFS.glsl");
    GLuint shaderProgramAdd = createShaderProgram("shaders/basicVS.glsl", "shaders/addFS.glsl");
    GLuint shaderProgramCopyTexture = createShaderProgram("shaders/basicVS.glsl", "shaders/copyTextureFS.glsl");

    glm::vec3 terrainOrigin = glm::vec3(0, 0, 0); // Center
    float terrainSizeHalf = terrainSize / 2.0f;

    glm::mat4 perspective = glm::perspectiveFov(1.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 1.0f, 500.0f);
    glm::mat4 orthoUI = glm::ortho(-(float)WINDOW_WIDTH/2.0f,(float)WINDOW_WIDTH/2.0f,-(float)WINDOW_HEIGHT/2.0f,(float)WINDOW_HEIGHT/2.0f,-20.0f,20.0f);
    glm::mat4 orthoFBO = glm::ortho(-1 * terrainSizeHalf, 1 * terrainSizeHalf, -1 * terrainSizeHalf, 1 * terrainSizeHalf, 0.0f, snowCoverMaxHeight);
    glm::mat4 orthoFBOLight = glm::ortho(-1 * 30.0f,1 * 30.0f,-1 * 30.0f,1 * 30.0f,0.0f,80.0f);
    glm::mat4 unitMatrix = glm::mat4(1.0f);
    
    Camera camera;
    Box box = Box::createBox();
    box.scale = glm::vec3(2, 4, 2);
    box.position = glm::vec3(-5, 0 +10.5f-4.0f, 0);
    box.textureId = loadPNGTexture("images/gray.png");
    // box.normalMap = loadPNGTexture("images/normalmap3.png");
    box.useNormalMapping = false;
    Box box2 = Box::createBox();
    box2.scale = glm::vec3(6, 2, 0.6f);
    box2.position = glm::vec3(-10.5f, 0.0f+9.0f-4.0f, 4.0f);
    box2.textureId = loadPNGTexture("images/gray.png");
    // box2.normalMap = loadPNGTexture("images/normalmap3.png");
    box2.useNormalMapping = false;
    Terrain ground(numVerticesPerRow);
    ground.scale = glm::vec3(terrainSize, 1, terrainSize);
    ground.position = terrainOrigin;
    ground.textureId = loadPNGTexture("images/white.png");
    ground.normalmap = loadPNGTexture("images/normalmap6.png");
    ground.heightmap = createTextureForHeightmap(textureSizeSnowHeightmap);


    float quadSize = 400;
    Quad quad;
    quad.scale = glm::vec3(quadSize, quadSize, 1);
    quad.position = glm::vec3(WINDOW_WIDTH / 2.0f - quadSize / 2.0f - 20, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f + 20, 0);
    Quad quadLL;
    quadLL.scale = glm::vec3(quadSize, quadSize, 1);
    quadLL.position = glm::vec3(-WINDOW_WIDTH / 2.0f + quadSize / 2.0f + 10, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f + 10, 0);

    FBOWrapper fboDepthSSAO = createFBOForDepthTexture(occlusionTextureWidth, occlusionTextureHeight);
    FBOWrapper fboOcclusionStep = createFrameBufferSingleTexture(occlusionTextureWidth, occlusionTextureHeight);
    FBOWrapper fboSnowCoverDepth = createFBOForDepthTexture(textureSizeSnowHeightmap, textureSizeSnowHeightmap);
    FBOWrapper fboLight = createFBOForDepthTexture(textureSizeSnowHeightmap, textureSizeSnowHeightmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    ground.shadowMap = fboLight.textureId;
    glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0,
                         0.0, 0.5, 0.0, 0.0,
                         0.0, 0.0, 0.5, 0.0,
                         0.5, 0.5, 0.5, 1.0
                        );

    GLuint shaderProgramMultiply = createShaderProgram("shaders/basicVS.glsl", "shaders/multiplyFS.glsl");

    PingPongTextureOperation blurOcclusion = PingPongTextureOperation(occlusionTextureHeight, occlusionTextureHeight, shaderProgramLowpass, 4);

    vector<ActiveArea> activeAreas;
    activeAreas.push_back(ActiveArea(0, 0, textureSizeSnowHeightmap, textureSizeSnowHeightmap));

    float heightScale = textureSizeSnowHeightmap / terrainSize;
    CalculateNormalsOperation calculateNormalsOperation = CalculateNormalsOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramCalculateNormals, heightScale);
    TextureOperation minTextureOperation1 = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramPixelMin);
    TextureOperation minTextureOperation2 = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramPixelMin);
    TextureOperation minTextureOperation3 = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramPixelMin);
    TextureOperation subtract = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramSubtract);
    TextureOperation subtract2 = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramSubtract);
    TextureOperation addTextureOperation = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramAdd);
    TextureOperation copyTextureProgram = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramCopyTexture);
    TextureOperation copyTextureHeightMapProgram = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramCopyTexture);
    
    
    PingPongTextureOperation blurSnowHeightmap = PingPongTextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramLowpass, 4);
    MultiplyOperation multiplyOperation = MultiplyOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramMultiply, 1);

    // In each frame only a subpart of the heightmap is updated, so we begin by writing 
    // the full heightmap to the framebuffer    
    copyTextureHeightMapProgram.execute(ground.heightmap, 0);
    ground.heightmap = copyTextureHeightMapProgram.getTextureResult();
    copyTextureHeightMapProgram.doClear = false;

    calculateNormalsOperation.execute(ground.heightmap, 0);
    blurSnowHeightmap.execute(calculateNormalsOperation.getTextureResult(), 0);
    ground.normalmapMacro = blurSnowHeightmap.getTextureResult();
    calculateNormalsOperation.doClear = false;
    blurSnowHeightmap.doClear = false;

    calculateNormalsOperation.doClear = false;
    minTextureOperation1.doClear = false;
    minTextureOperation2.doClear = false;
    subtract.doClear = false;
    subtract2.doClear = false;
    addTextureOperation.doClear = false;
    copyTextureProgram.doClear = false; // true?
    copyTextureHeightMapProgram.doClear = false;
    blurSnowHeightmap.doClear = false;
    multiplyOperation.doClear = false; 

    // SSAO stuff
    int kernelSize = 64; // Change kernel size in shader too
    glm::vec3 *samples = generateSamplePoints(kernelSize);
    GLuint noiseTexture = randomTexture();
    ground.numSamples = kernelSize;
    ground.ssaoMap = fboDepthSSAO.textureId;
    ground.randomTexture = noiseTexture;
    ground.samples = samples;
    box.ssaoMap = fboDepthSSAO.textureId;
    box.samples = samples;
    box.numSamples = kernelSize;
    box.randomTexture = noiseTexture;
    box2.ssaoMap = fboDepthSSAO.textureId;
    box2.samples = samples;
    box2.numSamples = kernelSize;
    box2.randomTexture = noiseTexture;

    Timing timing{};

    Box tire = loadUsingTinyObjLoader("tire.obj");
    tire.textureId = loadPNGTexture("images/gray.png");
    tire.position = glm::vec3(-15.0f, 5.5f, 0);
    tire.scale = glm::vec3(0.1f, 0.1f, 0.1f);

  
    RunningFootsteps footstep;
    // footstep.box.position.x = -15;
    // footstep.box.ssaoMap = fboDepthSSAO.textureId;
    // footstep.box.samples = samples;
    // footstep.box.numSamples = kernelSize;
    // footstep.box.randomTexture = noiseTexture;

    Erosion erosion = initializeErosion(textureSizeSnowHeightmap, numTimesToRunDistributeToCoutour, compression);
    erosion.calcAvgHeight.terrainSize = terrainSize;
    erosion.erosionStep1.terrainSize = terrainSize;
    erosion.erosionStep2.terrainSize = terrainSize;
    erosion.calcAvgHeight.slopeThreshold = slopeThreshold;
    erosion.calcAvgHeight.roughness = roughness;
    erosion.erosionStep1.slopeThreshold = slopeThreshold;
    erosion.erosionStep2.slopeThreshold = slopeThreshold;

    int i = 0;
    double previousTimeFPS = glfwGetTime();
    double previousTime = glfwGetTime();
    int frameCounter = 0;
    float dt = 0;

    GLuint prevObsticleMap; // Obsticle map from prev frame
    GLuint obsticleMap;

    float tireRotation = 0;



    while (!glfwWindowShouldClose(window))
    {
        glClearColor(135/255.0f, 206/255.0f, 235/255.0f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate deltaTime and FPS
        double currentTime = glfwGetTime();
        dt = currentTime - previousTime;
        previousTime = currentTime;
        if (currentTime - previousTimeFPS > 1.0f) {
            int fps = frameCounter;
            string title = "FPS: " + to_string(fps) + " - Frame time: " + to_string(dt) + " s";
            glfwSetWindowTitle(window, title.c_str());
            frameCounter = 0;
            previousTimeFPS = currentTime;
        }
        frameCounter++;

        // Update
        timing.begin("UPDATE_OBJECTS");
        updateCamera(camera, dt);
        glm::vec3 cameraLookAtPosition = camera.position + camera.forward * 10.0f;
        glm::mat4 worldToCamera = glm::lookAt(camera.position, cameraLookAtPosition, camera.up);
        footstep.update(dt*1.0f);

        // Moving box 
        box.position.y = 6.2f + 2 * sin(i*0.01f);
        box.position.x = 4 + 6 * cos(i*0.01f);
        box.position.z = 2 + 6 * sin(i*0.01f);
        box.rotation = glm::rotate(glm::mat4(1.0f), i * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));

        // Spinning box

        // box.position.y = 6.0f;
        // float speed = 0.01f;
        // float something = 1;
        // float cosV = cos(i * speed);
        // float sinV = sin(i * speed);
        // if (cosV < 0) {
        //     sinV *= 4;
        // }
        // box.position.x = 4 + 6 * cosV;
        // box.position.z = 2 + 6 * sinV;

        // box2.rotation = glm::rotate(glm::mat4(1.0f), 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        // box2.rotation = glm::rotate(glm::mat4(1.0f), 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        box2.rotation = glm::rotate(glm::mat4(1.0f), i * 0.081f, glm::vec3(0.0f, 1.0f, 0.0f));

        tire.position.y = 6.5f;
        tire.position.z = -10.0f;
        tire.position.x += 1.0f * dt;
        tireRotation += dt * 0.50f;
        tire.rotation = glm::rotate(glm::mat4(1.0f), tireRotation, glm::vec3(0.0f, 0.0f, -1.0f));
        timing.end("UPDATE_OBJECTS");


        // Update active areas
        timing.begin("UPDATE_ACTIVE_AREAS");
        activeAreas.clear();
        setActiveAreaForObject(terrainOrigin, terrainSize, box.position, box.scale, activeAreas);
        glm::vec3 sizeFootsteps = glm::vec3(2, 2, 2);
        setActiveAreaForObject(terrainOrigin, terrainSize, footstep.a.box.position, sizeFootsteps, activeAreas);
        setActiveAreaForObject(terrainOrigin, terrainSize, footstep.b.box.position, sizeFootsteps, activeAreas);
        setActiveAreaForObject(terrainOrigin, terrainSize, box2.position, box2.scale, activeAreas);
        glm::vec3 sizeTires = glm::vec3(2, 2, 2);
        setActiveAreaForObject(terrainOrigin, terrainSize, tire.position, sizeTires, activeAreas);
        timing.end("UPDATE_ACTIVE_AREAS");

        prevObsticleMap = obsticleMap;

        // Render shadow map
        timing.begin("RENDER_SHADOW_MAP");
        glm::mat4 worldToCameraLight = glm::lookAt(glm::vec3(20, 20, 20), glm::vec3(0, 0, 0), glm::normalize(glm::vec3(-1, 1, -1)));
        glViewport(0, 0, textureSizeSnowHeightmap, textureSizeSnowHeightmap);
        glBindFramebuffer(GL_FRAMEBUFFER, fboLight.fboId);
        glClearColor(0.5, 0.1, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        box.render(shaderProgram, worldToCameraLight, orthoFBOLight);
        box2.render(shaderProgram, worldToCameraLight, orthoFBOLight);
        footstep.render(shaderProgram, worldToCameraLight, orthoFBOLight);
        ground.render(shaderProgramTerrain, worldToCameraLight, orthoFBOLight);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glm::mat4 depthBiasMVP = biasMatrix * orthoFBOLight * worldToCameraLight;
        ground.depthBiasMVP = depthBiasMVP;
        timing.end("RENDER_SHADOW_MAP");

        timing.begin("COPY_TEXTURE");
        for (unsigned int i = 0; i < activeAreas.size(); i++) {
            copyTextureProgram.activeArea = activeAreas[i];
            copyTextureProgram.execute(prevObsticleMap, 0);
        }
        timing.end("COPY_TEXTURE");
        prevObsticleMap = copyTextureProgram.getTextureResult();

        // Render snow cover depth map
        timing.begin("RENDER_SNOW_DEPTH");
        glm::mat4 worldToCameraDepth = glm::lookAt(terrainOrigin, terrainOrigin + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
        glViewport(0, 0, textureSizeSnowHeightmap, textureSizeSnowHeightmap);
        glBindFramebuffer(GL_FRAMEBUFFER, fboSnowCoverDepth.fboId);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        box.render(shaderProgram, worldToCameraDepth, orthoFBO);
        box2.render(shaderProgram, worldToCameraDepth, orthoFBO);
        tire.render(shaderProgram, worldToCameraDepth, orthoFBO);
        footstep.render(shaderProgram, worldToCameraDepth, orthoFBO);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        timing.end("RENDER_SNOW_DEPTH");

        timing.begin("MULTIPLY");
        multiplyOperation.factor = snowCoverMaxHeight;
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            multiplyOperation.activeArea = activeAreas[i];
            multiplyOperation.execute(fboSnowCoverDepth.textureId, 0);
        }
        obsticleMap = multiplyOperation.getTextureResult(); // [0:10]
        timing.end("MULTIPLY");

        // Calculate new height map
        timing.begin("MIN");
        TextureOperation to = minTextureOperation1;
        if (to.fbo.textureId == ground.heightmap) {
            to = minTextureOperation2;
        }
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            to.activeArea = activeAreas[i];
            to.execute(ground.heightmap, multiplyOperation.getTextureResult());
        }
        
        timing.end("MIN");

        GLuint textureConsecutivePenetrationDiff;

        // Erosion
        timing.begin("DISTANCE_TRANSFORM_AND_DSTR");
        bool useErosion = true;
        if (useErosion) {
            for (unsigned i = 0; i < activeAreas.size(); i++) {
                subtract.activeArea = activeAreas[i];
                subtract.execute(to.getTextureResult(), ground.heightmap); // Result is between 0 and -10
                erosion.penetrationTexture = subtract.getTextureResult();
                buildDistanceMap(erosion, prevObsticleMap, activeAreas[i]);
                runDistribution(erosion, activeAreas[i]);
                addTextureOperation.activeArea = activeAreas[i];
                addTextureOperation.execute(to.getTextureResult(), erosion.distributedPenetratitionTexture);
            }
            ground.heightmap = addTextureOperation.getTextureResult();
        } else {
            ground.heightmap = to.getTextureResult();
        }
        timing.end("DISTANCE_TRANSFORM_AND_DSTR");

        timing.begin("EROSION");
        for (unsigned int i = 0; i < activeAreas.size(); i++) {
            runErosion(erosion, ground.heightmap, obsticleMap, activeAreas[i], numErosionStepsPerFrame);
        }        
        ground.heightmap = erosion.erosionStep1.getTextureResult();
        timing.end("EROSION");

        // Write updates to the FBO for the final heightmap
        timing.begin("COPY_TO_NEW_HEIGHTMAP");
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            copyTextureHeightMapProgram.activeArea = activeAreas[i];
            copyTextureHeightMapProgram.execute(ground.heightmap, 0);
        }
        timing.end("COPY_TO_NEW_HEIGHTMAP");
        ground.heightmap = copyTextureHeightMapProgram.getTextureResult();

        timing.begin("NORMAL_MACRO_BLUR");
        blurSnowHeightmap.timesRepeat = numBlurNormals;
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            calculateNormalsOperation.activeArea = activeAreas[i];
            calculateNormalsOperation.execute(ground.heightmap, 0);
            blurSnowHeightmap.activeArea = activeAreas[i];
            blurSnowHeightmap.execute(calculateNormalsOperation.getTextureResult(), 0);
        }
        timing.end("NORMAL_MACRO_BLUR");
        ground.normalmapMacro = blurSnowHeightmap.getTextureResult();

        timing.begin("SSAO");
        // Render to SSAO depth buffer
        // glViewport(0, 0, occlusionTextureWidth, occlusionTextureHeight);
        // glBindFramebuffer(GL_FRAMEBUFFER, fboDepthSSAO.fboId);
        // glClearColor(0.5, 0.1, 0.5, 1);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // box.render(shaderProgram, worldToCamera, perspective);
        // box2.render(shaderProgram, worldToCamera, perspective);
        // footstep.render(shaderProgram, worldToCamera, perspective);
        // ground.render(shaderProgramTerrain, worldToCamera, perspective);
        // glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // // Calculate occlusion texture
        // glViewport(0, 0, occlusionTextureWidth, occlusionTextureHeight);
        // glBindFramebuffer(GL_FRAMEBUFFER, fboOcclusionStep.fboId);
        // glClearColor(0.5, 0.1, 0.5, 1);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // box.render(shaderProgramOcclusionStep, worldToCamera, perspective);
        // box2.render(shaderProgramOcclusionStep, worldToCamera, perspective);
        // footstep.render(shaderProgramOcclusionStep, worldToCamera, perspective);
        // ground.render(shaderProgramOcclusionStepTerrain, worldToCamera, perspective);
        // glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // // Blur occlusion texture
        // blurOcclusion.execute(fboOcclusionStep.textureId, 0);
        timing.end("SSAO");

        ground.occlusionMap = blurOcclusion.getTextureResult();
        box.occlusionMap = blurOcclusion.getTextureResult();
        box2.occlusionMap = blurOcclusion.getTextureResult();

        // // Render to screen
        timing.begin("RENDER_TO_SCREEN");
        // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
        // glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
        ground.render(shaderProgramTerrain, worldToCamera, perspective);
        box.render(shaderProgram, worldToCamera, perspective);
        box2.render(shaderProgram, worldToCamera, perspective);
        tire.render(shaderProgram, worldToCamera, perspective);
        footstep.render(shaderProgram, worldToCamera, perspective);

        // Render helper quads
        // quad.textureId = minTextureOperation1.getTextureResult();
        // quad.textureId = ground.normalmapMacro;
        // quad.textureId = erosion.penetrationTexture;
        // quad.textureId = textureConsecutivePenetrationDiff;
        quad.textureId = fboSnowCoverDepth.textureId;
        // quad.render(shaderProgramQuad, unitMatrix, orthoUI);
        // quadLL.textureId = estimateVelocityProgram.getTextureResult();
        quadLL.textureId = ground.normalmapMacro;//.erosionStep1.getTextureResult();
        // quadLL.textureId = fboOcclusionStep.textureId;
        // quadLL.textureId = fboSnowCoverDepth.textureId;
        // quadLL.render(shaderProgramQuad, unitMatrix, orthoUI);

        timing.end("RENDER_TO_SCREEN");
      

        i++;

        keysPressed.clear();
        
        glfwPollEvents();
        timing.begin("SWAP_BUFFERS");
        glfwSwapBuffers(window);
        timing.end("SWAP_BUFFERS");

        if (i % 30 == 0) {
            timing.print();
        }
    }

    return 0;
}