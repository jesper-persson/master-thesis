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

// Non-settings parameters
const int WINDOW_HEIGHT = 1200;
const int WINDOW_WIDTH = 1800;
const int frustumHeight = 30;
const int heightColumnScale = 10000;
const float boundingBoxMargin = 4.0f;
const bool useSSBO = true;

// Settings parameters
float terrainSize = 80.0f; // World space size of terrain mesh.
int numVerticesPerRow = 80; // Resolution of terrain mesh.
float heightmapSize = 500;
bool evenOutSteepSlopes = true;
float compression = 1.0f; // 0 => Nothing compressed
float roughness = 0.3f;
float slopeThreshold = 1.9f;
int numIterationsEvenOutSlopes = 10;
int numIterationsBlurNormals = 2;
int numIterationsDisplaceMaterialToContour = 5;

#include "texture.cpp"
#include "shader.cpp"
#include "box.cpp"
#include "quad.cpp"
#include "terrain.cpp"
#include "fbo.cpp"
#include "animatedFootsteps.cpp"
#include "textureOperations.cpp"
#include "timing.cpp"
#include "settings.hpp"

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
    glm::vec3 position = glm::vec3(-8.5f, 12.0f, 25.0f);
    glm::vec3 up = glm::vec3(0, 1.0f, 0);
    glm::vec3 forward = glm::normalize(glm::vec3(0.0f, 0, -1.0f));
};

void controlObject(glm::vec3 &position, glm::vec3 &forward, glm::vec3 &up, float dt) {
    float speed = 15.0f * dt;
    float rotationSpeed = speed / 8.0f;
    glm::vec3 left = glm::cross(up, forward);
    if (isKeyDown(GLFW_KEY_W)) {
        position = position + forward * speed;
    }
    if (isKeyDown(GLFW_KEY_S)) {
        position = position - forward * speed;
    }
    if (isKeyDown(GLFW_KEY_A)) {
        position = position + left * speed;
    }
    if (isKeyDown(GLFW_KEY_D)) {
        position = position - left * speed;
    }
    if (isKeyDown(GLFW_KEY_LEFT)) {
        forward = glm::normalize(glm::rotate(forward, rotationSpeed, glm::vec3(0, 1, 0)));
        up = glm::normalize(glm::rotate(up, rotationSpeed, glm::vec3(0, 1, 0)));
    }
    if (isKeyDown(GLFW_KEY_RIGHT)) {
        forward = glm::normalize(glm::rotate(forward, -rotationSpeed, glm::vec3(0, 1, 0)));
        up = glm::normalize(glm::rotate(up, -rotationSpeed, glm::vec3(0, 1, 0)));
    }
    if (isKeyDown(GLFW_KEY_UP)) {
        forward = glm::normalize(glm::rotate(forward, rotationSpeed, left));
        up = glm::normalize(glm::rotate(up, rotationSpeed, left));
    }
    if (isKeyDown(GLFW_KEY_DOWN)) {
        forward = glm::normalize(glm::rotate(forward, -rotationSpeed, left));
        up = glm::normalize(glm::rotate(up, -rotationSpeed, left));
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
    float ratio = heightmapSize / terrainSize;
    glm::vec3 diff = (positionOfObject - terrainOrigin) * ratio;
    int size = boundingBoxSize.x * ratio * boundingBoxMargin;

    int posX = (int)(diff.x);
    int posZ = (int)(diff.z);

    if (size % 2 != 0) {
        size += 1;
    }

    // // cout << posX << ", " << posZ << " , " << size << " ,  " << size << endl;

    activeAreas.push_back(ActiveArea(posX, posZ, size, size));
}

int main(int argc, char* argv[])
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

    // Simulation loop data
    int frameCounterGlobal = 0;
    double previousTimeFPS = glfwGetTime();
    double previousTime = glfwGetTime();
    int frameCounter = 0;
    float dt = 0;

    // Load terrain settings
    if (argc > 1) {
        applySettings(argv[1]);
    } else {
        cerr << "Missing settings file" << endl;
        return 1;
    }

    glm::vec3 terrainOrigin = glm::vec3(0, 0, 0);

    // Create objects in the scene
    Box box = Box::createBox();
    box.scale = glm::vec3(2, 4, 2);
    box.position = glm::vec3(5, 8.001f, 0);
    box.textureId = loadPNGTexture("images/gray.png");
    box.useNormalMapping = false;

    Box box2 = Box::createBox();
    box2.scale = glm::vec3(26, 4, 5.0f);
    box2.position = glm::vec3(-10.5f, 6.0f, 4.0f);
    box2.textureId = loadPNGTexture("images/red.png");
    box2.useNormalMapping = false;

    Box tire = loadUsingTinyObjLoader("tire.obj");
    tire.textureId = loadPNGTexture("images/gray.png");
    tire.position = glm::vec3(-15.0f, 5.5f, 0);
    tire.scale = glm::vec3(0.1f, 0.1f, 0.1f);

    Footstep footstep;

    Terrain ground(numVerticesPerRow);
    ground.scale = glm::vec3(terrainSize, 1, terrainSize);
    ground.position = terrainOrigin;
    ground.textureId = loadPNGTexture("images/white.png");
    ground.normalmap = loadPNGTexture("images/normalmap2.png");
    ground.heightColumnScale = heightColumnScale;
    ground.heightmap = createTextureForHeightmap(heightmapSize);
    // ground.heightmap = loadPNGTextureForHeightmap("images/heightmap2.png");

    Camera camera;

    // When true, the arrow keys are used to control a box instead of the camera
    bool controlBox = false;

    float quadSize = 300;
    Quad quad;
    quad.scale = glm::vec3(quadSize, quadSize, 1);
    quad.position = glm::vec3(WINDOW_WIDTH / 2.0f - quadSize / 2.0f - 20, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f + 20, 0);

    Quad quadLL;
    quadLL.scale = glm::vec3(quadSize, quadSize, 1);
    quadLL.position = glm::vec3(-WINDOW_WIDTH / 2.0f + quadSize / 2.0f + 10, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f + 10, 0);

    // Projection matrices
    glm::mat4 perspective = glm::perspectiveFov(1.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 1.0f, 500.0f);
    glm::mat4 orthoUI = glm::ortho(-(float)WINDOW_WIDTH/2.0f,(float)WINDOW_WIDTH/2.0f,-(float)WINDOW_HEIGHT/2.0f,(float)WINDOW_HEIGHT/2.0f,-20.0f,20.0f);
    glm::mat4 terrainDepthProjection = glm::ortho(-1 * terrainSize / 2.0f, 1 * terrainSize / 2.0f, -1 * terrainSize / 2.0f, 1 * terrainSize / 2.0f, 0.0f, static_cast<float>(frustumHeight));
    glm::mat4 unitMatrix = glm::mat4(1.0f);

    // Render to screen shaders
    GLuint shaderProgramDefault = createShaderProgram("shaders/basicVS.glsl", "shaders/basicFS.glsl");
    GLuint shaderProgramTerrain = createShaderProgram("shaders/terrainVS.glsl", "shaders/terrainTCS.glsl", "shaders/terrainTES.glsl", "shaders/terrainFS.glsl");
    GLuint shaderProgramQuad = createShaderProgram("shaders/basicVS.glsl", "shaders/quadFS.glsl");

    // Texture operation shaders
    GLuint shaderProgramLowpass = createShaderProgram("shaders/basicVS.glsl", "shaders/blurFS.glsl");
    GLuint shaderProgramCalculateNormals = createShaderProgram("shaders/basicVS.glsl", "shaders/calculateNormalsFS.glsl");
    GLuint shaderProgramCopyTexture = createShaderProgram("shaders/basicVS.glsl", "shaders/copyTextureFS.glsl");


    FBOWrapper fboSnowCoverDepth = createFBOForDepthTexture(heightmapSize, heightmapSize);
    glm::mat4 worldToCameraDepth = glm::lookAt(terrainOrigin, terrainOrigin + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));

    float heightScale = heightmapSize / terrainSize;
    CalculateNormalsOperation calculateNormalsOperation = CalculateNormalsOperation(heightmapSize, heightmapSize, shaderProgramCalculateNormals, heightScale, heightColumnScale);
    
    CreateInitialHeightmapTexture createInitialHeightmapTexture = CreateInitialHeightmapTexture(heightmapSize, heightmapSize, shaderProgramCopyTexture, TextureFormat::R32UI, frustumHeight, heightColumnScale);
    // When updating the scene, only a subpart of the heightmap is updates. So we must
    // begin by storing the entire heightmap in this framebuffer
    createInitialHeightmapTexture.execute(ground.heightmap, 0);
    ground.heightmap = createInitialHeightmapTexture.getTextureResult();

    DistributeSnowSSBO buildDeltaSnowSSBOBufferOperation = DistributeSnowSSBO(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/distributeSnowSSBOFS.glsl"), TextureFormat::RGBA16F, compression);

    // Tell it not to create a texture tho
    TextureOperation combineSSBOWithHeightmap = TextureOperation(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/combineSSBOWithHeightmapFS.glsl"), TextureFormat::R32UI);
    combineSSBOWithHeightmap.fbo.setOutputTexture(createInitialHeightmapTexture.getTextureResult());
    if (useSSBO) {
        ground.heightmap = combineSSBOWithHeightmap.getTextureResult();
    }


    ErosionOperation calcAvgHeight = ErosionOperation(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/erosionCalcAvgHeightFS.glsl"), TextureFormat::RGBA32UI, frustumHeight, heightColumnScale);
    calcAvgHeight.terrainSize = terrainSize;
    calcAvgHeight.slopeThreshold = slopeThreshold;
    calcAvgHeight.roughness = roughness;
    ErosionOperation evenOutHeights = ErosionOperation(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/erosionFS.glsl"), TextureFormat::RG32UI, frustumHeight, heightColumnScale);
    evenOutHeights.terrainSize = terrainSize;
    evenOutHeights.slopeThreshold = slopeThreshold;
    TextureOperation erosionResultToHeightmap = TextureOperation(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/RGBA32UI_TO_R32UI.glsl"), TextureFormat::R32UI);
    erosionResultToHeightmap.fbo.setOutputTexture(createInitialHeightmapTexture.getTextureResult());
    if (!useSSBO) {
        ground.heightmap = erosionResultToHeightmap.getTextureResult();
    }

    // SSBO even out steep slopes
    ErosionOperation ssboEvenOutSlopes = ErosionOperation(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/evenOutSteepSlopesSSBO.glsl"), TextureFormat::RGBA32UI, frustumHeight, heightColumnScale);
    ssboEvenOutSlopes.terrainSize = terrainSize;
    ssboEvenOutSlopes.slopeThreshold = slopeThreshold;
    ssboEvenOutSlopes.roughness = roughness;

    // Iterative move penetrated terrain material
    CalculateNumNeighbors calculateNumNeighborsWithLessContourValue = CalculateNumNeighbors(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/calculateNumNeighborsWithLessContourValue.glsl"), TextureFormat::RGBA32I, compression);
    PingPongTextureOperation distributeToLowerContourValues = PingPongTextureOperation(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/distributeToLowerContourValues.glsl"), numIterationsDisplaceMaterialToContour, TextureFormat::RGBA32I);
    PrepareTextureForCalcAvg combineDistributedTerrainWithHeightmap = PrepareTextureForCalcAvg(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/combineDistributedTerrainWithHeightmap.glsl"), TextureFormat::RG32UI, frustumHeight, heightColumnScale);

    // Turn int heightmap into float
    IntHeightmapToFloat intHeightmapToFloat = IntHeightmapToFloat(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/intHeightmapToFloat.glsl"), TextureFormat::R32F, heightColumnScale);
    intHeightmapToFloat.execute(ground.heightmap, 0);


    int jumpFloodIterations = log2(heightmapSize);
    JumpFloodingMainOperation jumpFloodMainOperation = JumpFloodingMainOperation(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFloodFS.glsl"), jumpFloodIterations);

    GLuint createPenetrationTextureShader = createShaderProgram("shaders/basicVS.glsl", "shaders/createPenetrationTextureFS.glsl");
    CreatePenetrationTexture createPenetartionTextureOperation = CreatePenetrationTexture(heightmapSize, heightmapSize, createPenetrationTextureShader, TextureFormat::RGBA32I, frustumHeight, heightColumnScale);
    PingPongTextureOperation blurSnowHeightmap = PingPongTextureOperation(heightmapSize, heightmapSize, shaderProgramLowpass, 4, TextureFormat::RGBA16F);
    blurSnowHeightmap.sampling = GL_LINEAR;
    blurSnowHeightmap.inputTexturesAreFloat = true;

    calculateNormalsOperation.execute(ground.heightmap, 0);
    blurSnowHeightmap.execute(calculateNormalsOperation.getTextureResult(), 0);
    ground.normalmapMacro = blurSnowHeightmap.getTextureResult();

    // SSBO for snow distribution
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);
    unsigned int* ssboDefaultValue = (unsigned int*)malloc(heightmapSize * heightmapSize * sizeof(unsigned int));
    std::memset(ssboDefaultValue, 0, heightmapSize * heightmapSize * sizeof(unsigned int));
    glBufferData(GL_SHADER_STORAGE_BUFFER, heightmapSize * heightmapSize * sizeof(unsigned int), ssboDefaultValue, GL_DYNAMIC_DRAW);
    free(ssboDefaultValue);

    // Active areas
    vector<ActiveArea> activeAreas;
    activeAreas.push_back(ActiveArea(0, 0, heightmapSize, heightmapSize));
    // activeAreas.push_back(ActiveArea(0, 0, heightmapSize, heightmapSize));
    // activeAreas.push_back(ActiveArea(0, 0, heightmapSize, heightmapSize));
    // activeAreas.push_back(ActiveArea(0, 0, heightmapSize, heightmapSize));

    cout << "Initial volume of terrain: " << endl;
    readBackAndAccumulatePixelValue(createInitialHeightmapTexture.fbo.fboId, heightmapSize, TextureFormat::R32UI);


    cout << "Initial volume of terrain SSBO: " << endl;
    readBackAndAccumulatePixelValue(combineSSBOWithHeightmap.fbo.fboId, heightmapSize, TextureFormat::R32UI);


    Timing timing{};

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(145/255.0f, 189/255.0f, 224/255.0f, 1);
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
        glm::vec3 cameraLookAtPosition = camera.position + camera.forward * 10.0f;
        glm::mat4 worldToCamera = glm::lookAt(camera.position, cameraLookAtPosition, camera.up);

        if (isKeyPressed(GLFW_KEY_T)) {
            controlBox = !controlBox;
        }

        if (controlBox) {
            controlObject(box.position, box.forward, box.up, dt * 0.7f);
            glm::vec3 fromPosition = box.position - box.forward * 16.0f + box.up * 14.0f;
            worldToCamera = glm::lookAt(fromPosition, box.position, box.up);
        } else {
            controlObject(camera.position, camera.forward, camera.up, dt);
            // box.position.y = 6.2f + 1 * sin(frameCounterGlobal * 0.01f);
            // box.position.x = 4 + 6 * cos(frameCounterGlobal * 0.01f);
            // box.position.z = 2 + 6 * sin(frameCounterGlobal * 0.01f);
            // box.rotation = glm::rotate(glm::mat4(1.0f), frameCounterGlobal * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
        }

        // footstep.update(dt*1.0f);

        // box2.rotation = glm::rotate(glm::mat4(1.0f), frameCounterGlobal * 0.081f, glm::vec3(0.0f, 1.0f, 0.0f));
        // box2.forward = glm::rotate(box2.forward, dt * 0.9f, glm::vec3(0, 1, 0));

        // tire.position.y = 6.5f;
        // tire.position.z = -10.0f;
        // tire.position.x += 1.0f * dt;
        // tire.up = glm::rotate(tire.up, dt * 0.5f, glm::vec3(0, 0, -1));
        timing.end("UPDATE_OBJECTS");

        // Update active areas
        timing.begin("UPDATE_ACTIVE_AREAS");
        // activeAreas.clear();
        // // activeAreas.push_back(ActiveArea(0, 0, heightmapSize, heightmapSize));
        // // activeAreas.push_back(ActiveArea(0, 0, heightmapSize/8.1999f, heightmapSize/8.1999f));
        // setActiveAreaForObject(terrainOrigin, terrainSize, box.position, box.scale, activeAreas);
        // setActiveAreaForObject(terrainOrigin, terrainSize, box2.position, box2.scale, activeAreas);

        // glm::vec3 eight = glm::vec3(8, -1, -1);
        // glm::vec3 four = glm::vec3(4, -1, -1);
        // // setActiveAreaForObject(terrainOrigin, terrainSize, box.position, eight, activeAreas);
        // // setActiveAreaForObject(terrainOrigin, terrainSize, box.position, four, activeAreas);


        // // setActiveAreaForObject(terrainOrigin, terrainSize, box.position, hejhej2, activeAreas);
        // // setActiveAreaForObject(terrainOrigin, terrainSize, box.position, hejhej, activeAreas);
        // // setActiveAreaForObject(terrainOrigin, terrainSize, box.position, box.scale, activeAreas);
        // // setActiveAreaForObject(terrainOrigin, terrainSize, box.position, box2.scale, activeAreas);
        // glm::vec3 sizeFootsteps = glm::vec3(2, 2, 2);
        // setActiveAreaForObject(terrainOrigin, terrainSize, footstep.box.position, sizeFootsteps, activeAreas);
        // // setActiveAreaForObject(terrainOrigin, terrainSize, footstep.box.position, sizeFootsteps, activeAreas);
        // // setActiveAreaForObject(terrainOrigin, terrainSize, box2.position, box2.scale, activeAreas);
        // glm::vec3 sizeTires = glm::vec3(2, 2, 2);
        // setActiveAreaForObject(terrainOrigin, terrainSize, tire.position, sizeTires, activeAreas);
        timing.end("UPDATE_ACTIVE_AREAS");

        // Render snow cover depth map
        timing.begin("RENDER_SNOW_DEPTH");
        glViewport(0, 0, heightmapSize, heightmapSize);
        glBindFramebuffer(GL_FRAMEBUFFER, fboSnowCoverDepth.fboId);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        box.render(shaderProgramDefault, worldToCameraDepth, terrainDepthProjection);
        box2.render(shaderProgramDefault, worldToCameraDepth, terrainDepthProjection);
        tire.render(shaderProgramDefault, worldToCameraDepth, terrainDepthProjection);
        footstep.render(shaderProgramDefault, worldToCameraDepth, terrainDepthProjection);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        timing.end("RENDER_SNOW_DEPTH");

        GLuint obsticleMap = fboSnowCoverDepth.textureId;

        // Distance transform and distribute snow to boundaries
        timing.begin("PENETRATION_TEXTURE_CALC");
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            createPenetartionTextureOperation.activeArea = activeAreas[i];
            createPenetartionTextureOperation.execute(fboSnowCoverDepth.textureId, ground.heightmap);
        }
        timing.end("PENETRATION_TEXTURE_CALC");

        timing.begin("JUMPFLOODING");
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            jumpFloodMainOperation.activeArea = activeAreas[i];
            jumpFloodMainOperation.passIndex = 0;
            jumpFloodMainOperation.execute(createPenetartionTextureOperation.getTextureResult(), 0);
        }
        timing.end("JUMPFLOODING");

        

        // SSBO approach
        if (useSSBO) {
            timing.begin("SSBO_MOVE_TO_CONTOUR");
            for (unsigned i = 0; i < activeAreas.size(); i++) {
                buildDeltaSnowSSBOBufferOperation.activeArea = activeAreas[i];
                buildDeltaSnowSSBOBufferOperation.execute(jumpFloodMainOperation.getTextureResult(), ground.heightmap);
            }
            timing.end("SSBO_MOVE_TO_CONTOUR");

            timing.begin("SSBO_EVEN_OUT");
            for (unsigned i = 0; i < activeAreas.size(); i++) {
                for (int j = 0; j < numIterationsEvenOutSlopes; j++) {
                    ssboEvenOutSlopes.activeArea = activeAreas[i];
                    ssboEvenOutSlopes.execute(obsticleMap, 0);
                }
            }

            for (unsigned i = 0; i < activeAreas.size(); i++) {
                combineSSBOWithHeightmap.activeArea = activeAreas[i];
                combineSSBOWithHeightmap.execute(0, 0);
            }
            ground.heightmap = combineSSBOWithHeightmap.getTextureResult();
            timing.end("SSBO_EVEN_OUT");
        }

        // Iterative approach
        if (!useSSBO) {
            timing.begin("ITERATIVE_MOVE_TO_CONTOUR");
            for (unsigned i = 0; i < activeAreas.size(); i++) {
                calculateNumNeighborsWithLessContourValue.activeArea = activeAreas[i];
                calculateNumNeighborsWithLessContourValue.execute(jumpFloodMainOperation.getTextureResult(), 0);
            }
            for (unsigned i = 0; i < activeAreas.size(); i++) {
                distributeToLowerContourValues.activeArea = activeAreas[i];
                distributeToLowerContourValues.execute(calculateNumNeighborsWithLessContourValue.getTextureResult(), 0);
            }
            for (unsigned i = 0; i < activeAreas.size(); i++) {
                combineDistributedTerrainWithHeightmap.activeArea = activeAreas[i];
                combineDistributedTerrainWithHeightmap.obstacleMap = obsticleMap;
                combineDistributedTerrainWithHeightmap.execute(distributeToLowerContourValues.getTextureResult(), ground.heightmap);
            }
            // TEMP
            // for (unsigned i = 0; i < activeAreas.size(); i++) {
                // erosionResultToHeightmap.activeArea = activeAreas[i];
                // erosionResultToHeightmap.execute(combineDistributedTerrainWithHeightmap.getTextureResult(), 0);
                // ground.heightmap = erosionResultToHeightmap.getTextureResult();
            // }
            // TEMP
            timing.end("ITERATIVE_MOVE_TO_CONTOUR");

            timing.begin("ITERATIVE_EVEN_OUT");
            if (evenOutSteepSlopes) {
                for (unsigned int i = 0; i < activeAreas.size(); i++) {
                    calcAvgHeight.activeArea = activeAreas[i];
                    evenOutHeights.activeArea = activeAreas[i];

                    GLuint calcAvgHeightInput = combineDistributedTerrainWithHeightmap.getTextureResult();
                    for (int j = 0; j < numIterationsEvenOutSlopes; j++) {
                        calcAvgHeight.execute(calcAvgHeightInput, 0);
                        evenOutHeights.execute(calcAvgHeight.getTextureResult(), 0);
                        calcAvgHeightInput = evenOutHeights.getTextureResult();
                    }

                    erosionResultToHeightmap.activeArea = activeAreas[i];
                    erosionResultToHeightmap.execute(evenOutHeights.getTextureResult(), 0);
                }
                ground.heightmap = erosionResultToHeightmap.getTextureResult();
            }
            timing.end("ITERATIVE_EVEN_OUT");
        }

        timing.begin("NORMAL_MACRO_BLUR");
        blurSnowHeightmap.timesRepeat = numIterationsBlurNormals;
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            calculateNormalsOperation.activeArea = activeAreas[i];
            calculateNormalsOperation.execute(ground.heightmap, 0);
            blurSnowHeightmap.activeArea = activeAreas[i];
            blurSnowHeightmap.execute(calculateNormalsOperation.getTextureResult(), 0);
        }
        ground.normalmapMacro = blurSnowHeightmap.getTextureResult();
        timing.end("NORMAL_MACRO_BLUR");

        timing.begin("INT_HEIGHTMAP_TO_FLOAT");
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            intHeightmapToFloat.activeArea = activeAreas[i];
            intHeightmapToFloat.execute(ground.heightmap, 0);
        }
        timing.end("INT_HEIGHTMAP_TO_FLOAT");

        // Render to screen
        timing.begin("RENDER_TO_SCREEN");
        // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
        ground.render(shaderProgramTerrain, worldToCamera, perspective, true, intHeightmapToFloat.getTextureResult());
        // glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
        box.render(shaderProgramDefault, worldToCamera, perspective);
        box2.render(shaderProgramDefault, worldToCamera, perspective);
        tire.render(shaderProgramDefault, worldToCamera, perspective);
        footstep.render(shaderProgramDefault, worldToCamera, perspective);

        // Render helper quads
        quad.textureId = createPenetartionTextureOperation.getTextureResult();
        quad.render(shaderProgramQuad, unitMatrix, orthoUI);
        quadLL.textureId = createPenetartionTextureOperation.getTextureResult();
        quadLL.render(shaderProgramQuad, unitMatrix, orthoUI);
        timing.end("RENDER_TO_SCREEN");

        keysPressed.clear();

        glfwPollEvents();
        timing.begin("SWAP_BUFFERS");
        glfwSwapBuffers(window);
        timing.end("SWAP_BUFFERS");

        if (frameCounterGlobal == 0) {
            // readBackAndAccumulatePixelValue(createInitialHeightmapTexture.fbo.fboId, heightmapSize, TextureFormat::R32UI);
        }

        if (frameCounterGlobal % 1000 == 0) {
            timing.print();
            readBackAndAccumulatePixelValue(createInitialHeightmapTexture.fbo.fboId, heightmapSize, TextureFormat::R32UI);
        }

        frameCounterGlobal++;
    }

    return 0;
}