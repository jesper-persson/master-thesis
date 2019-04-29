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
float terrainSize = 80.0f; // World space size of terrain mesh.
int numVerticesPerRow = 80; // Resolution of terrain mesh.

// Tesselation is manually set in Tessellation control shader.

const int heightColumnScale = 1000;

float textureSizeSnowHeightmap = 500;
const float boundingBoxMargin = 4.0f;
bool useErosion = true;
float compression = 1.0f; // 1 => nothing compressed
float roughness = 0.3f;
float slopeThreshold = 1.9f;
int numErosionStepsPerFrame = 11; // Odd number
int numBlurNormals = 2; // [1,-]

int numTimesToRunDistributeToCoutour = 5;

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

    // glm::vec3 position = glm::vec3(-5.0f, 10.0f, 10.0f);
    // glm::vec3 up = glm::vec3(0, 0.0f, -1.0);
    // glm::vec3 forward = glm::normalize(glm::vec3(0.0f, -1.0f, 0));
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
    float ratio = textureSizeSnowHeightmap / terrainSize;
    glm::vec3 diff = (positionOfObject - terrainOrigin) * ratio;
    int size = boundingBoxSize.x * ratio * boundingBoxMargin;
    
    if (size % 2 != 0) {
        size += 1;
    }
    activeAreas.push_back(ActiveArea(diff.x, diff.z, size, size));
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
    ground.heightmap = createTextureForHeightmap(textureSizeSnowHeightmap);
    // ground.heightmap = loadPNGTexture("images/heightmap2.png");

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
    glm::mat4 terrainDepthProjection = glm::ortho(-1 * terrainSize / 2.0f, 1 * terrainSize / 2.0f, -1 * terrainSize / 2.0f, 1 * terrainSize / 2.0f, 0.0f, snowCoverMaxHeight);
    glm::mat4 unitMatrix = glm::mat4(1.0f);

    // Render to screen shaders
    GLuint shaderProgramDefault = createShaderProgram("shaders/basicVS.glsl", "shaders/basicFS.glsl");
    GLuint shaderProgramTerrain = createShaderProgram("shaders/terrainVS.glsl", "shaders/terrainTCS.glsl", "shaders/terrainTES.glsl", "shaders/terrainFS.glsl");
    GLuint shaderProgramQuad = createShaderProgram("shaders/basicVS.glsl", "shaders/quadFS.glsl");

    // Texture operation shaders
    GLuint shaderProgramLowpass = createShaderProgram("shaders/basicVS.glsl", "shaders/blurFS.glsl");
    GLuint shaderProgramCalculateNormals = createShaderProgram("shaders/basicVS.glsl", "shaders/calculateNormalsFS.glsl");
    GLuint shaderProgramCopyTexture = createShaderProgram("shaders/basicVS.glsl", "shaders/copyTextureFS.glsl");


    FBOWrapper fboSnowCoverDepth = createFBOForDepthTexture(textureSizeSnowHeightmap, textureSizeSnowHeightmap);
glm::mat4 worldToCameraDepth = glm::lookAt(terrainOrigin, terrainOrigin + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));

    float heightScale = textureSizeSnowHeightmap / terrainSize;
    CalculateNormalsOperation calculateNormalsOperation = CalculateNormalsOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramCalculateNormals, heightScale);
    TextureOperation copyTextureProgram = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramCopyTexture, TextureFormat::RGBA16F);
    TextureOperation copyTextureHeightMapProgram = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramCopyTexture, TextureFormat::R32UI);

    TextureOperation buildDeltaSnowSSBOBufferOperation = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createShaderProgram("shaders/basicVS.glsl", "shaders/distributeSnowSSBOFS.glsl"), TextureFormat::RGBA16F);
    buildDeltaSnowSSBOBufferOperation.doClear = false;

    TextureOperation combineSSBOWithHeightmap = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createShaderProgram("shaders/basicVS.glsl", "shaders/combineSSBOWithHeightmapFS.glsl"), TextureFormat::R32UI);

    TextureOperation setChannels = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createShaderProgram("shaders/basicVS.glsl", "shaders/setChannels.glsl"), TextureFormat::RG32UI);
    setChannels.doClear = false;
    ErosionOperation calcAvgHeight = ErosionOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createShaderProgram("shaders/basicVS.glsl", "shaders/erosionCalcAvgHeightFS.glsl"), TextureFormat::RGBA32UI);
    calcAvgHeight.doClear = false;
    calcAvgHeight.terrainSize = terrainSize;
    calcAvgHeight.slopeThreshold = slopeThreshold;
    calcAvgHeight.roughness = roughness;
    ErosionOperation evenOutHeights1 = ErosionOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createShaderProgram("shaders/basicVS.glsl", "shaders/erosionFS.glsl"), TextureFormat::RG32UI);
    evenOutHeights1.doClear = false;
    evenOutHeights1.terrainSize = terrainSize;
    evenOutHeights1.slopeThreshold = slopeThreshold;
    ErosionOperation evenOutHeights2 = ErosionOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createShaderProgram("shaders/basicVS.glsl", "shaders/erosionFS.glsl"), TextureFormat::RG32UI);
    evenOutHeights2.doClear = false;
    evenOutHeights2.terrainSize = terrainSize;
    evenOutHeights2.slopeThreshold = slopeThreshold;
    TextureOperation erosionResultToHeightmap = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createShaderProgram("shaders/basicVS.glsl", "shaders/RGBA32UI_TO_R32UI.glsl"), TextureFormat::R32UI);
    erosionResultToHeightmap.doClear = false;

    // Iterative move penetrated terrain material
    TextureOperation calculateNumNeighborsWithLessContourValue = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createShaderProgram("shaders/basicVS.glsl", "shaders/calculateNumNeighborsWithLessContourValue.glsl"), TextureFormat::RGBA32I);
    calculateNumNeighborsWithLessContourValue.doClear = false;
    PingPongTextureOperation distributeToLowerContourValues = PingPongTextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createShaderProgram("shaders/basicVS.glsl", "shaders/distributeToLowerContourValues.glsl"), numTimesToRunDistributeToCoutour, TextureFormat::RGBA32I);
    distributeToLowerContourValues.doClear = false;
    TextureOperation combineDistributedTerrainWithHeightmap = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createShaderProgram("shaders/basicVS.glsl", "shaders/combineDistributedTerrainWithHeightmap.glsl"), TextureFormat::R32UI);
    combineDistributedTerrainWithHeightmap.doClear = false;



    TextureOperation jumpFloodInitOperation = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFloodStartStepFS.glsl"), TextureFormat::RGBA32I);
    jumpFloodInitOperation.doClear = true;
    int jumpFloodIterations = log2(textureSizeSnowHeightmap);
    JumpFloodingMainOperation jumpFloodMainOperation = JumpFloodingMainOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFloodFS.glsl"), jumpFloodIterations);

    GLuint createPenetrationTextureShader = createShaderProgram("shaders/basicVS.glsl", "shaders/createPenetrationTextureFS.glsl");
    TextureOperation createPenetartionTextureOperation = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, createPenetrationTextureShader, TextureFormat::RGBA32I);
    PingPongTextureOperation blurSnowHeightmap = PingPongTextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramLowpass, 4, TextureFormat::RGBA16F);

    // When updating the scene, only a subpart of the heightmap is updates. So we must
    // begin by storing the entire heightmap in this framebuffer
    copyTextureHeightMapProgram.execute(ground.heightmap, 0);
    ground.heightmap = copyTextureHeightMapProgram.getTextureResult();
    copyTextureHeightMapProgram.doClear = false;

    calculateNormalsOperation.execute(ground.heightmap, 0);
    blurSnowHeightmap.execute(calculateNormalsOperation.getTextureResult(), 0);
    ground.normalmapMacro = blurSnowHeightmap.getTextureResult();
    calculateNormalsOperation.doClear = false;
    blurSnowHeightmap.doClear = false;

    calculateNormalsOperation.doClear = false;
    copyTextureProgram.doClear = false; // true?
    copyTextureHeightMapProgram.doClear = false;
    blurSnowHeightmap.doClear = false;    

    // SSBO for snow distribution
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);
    int* ssboDefaultValue = (int*)malloc(textureSizeSnowHeightmap * textureSizeSnowHeightmap * sizeof(int));
    std::memset(ssboDefaultValue, 0, textureSizeSnowHeightmap * textureSizeSnowHeightmap * sizeof(int));
    glBufferData(GL_SHADER_STORAGE_BUFFER, textureSizeSnowHeightmap * textureSizeSnowHeightmap * sizeof(int), ssboDefaultValue, GL_DYNAMIC_DRAW);
    free(ssboDefaultValue);

    // Active areas
    vector<ActiveArea> activeAreas;
    activeAreas.push_back(ActiveArea(0, 0, textureSizeSnowHeightmap, textureSizeSnowHeightmap));
    // activeAreas.push_back(ActiveArea(0, 0, textureSizeSnowHeightmap, textureSizeSnowHeightmap));
    // activeAreas.push_back(ActiveArea(0, 0, textureSizeSnowHeightmap, textureSizeSnowHeightmap));
    // activeAreas.push_back(ActiveArea(0, 0, textureSizeSnowHeightmap, textureSizeSnowHeightmap));

    cout << "Initial volume of terrain: " << endl;
    readBackAndAccumulatePixelValue(copyTextureHeightMapProgram.fbo.fboId, textureSizeSnowHeightmap, TextureFormat::R32UI);

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
        // setActiveAreaForObject(terrainOrigin, terrainSize, box.position, box.scale, activeAreas);
        // glm::vec3 sizeFootsteps = glm::vec3(2, 2, 2);
        // setActiveAreaForObject(terrainOrigin, terrainSize, footstep.box.position, sizeFootsteps, activeAreas);
        // setActiveAreaForObject(terrainOrigin, terrainSize, footstep.box.position, sizeFootsteps, activeAreas);
        // setActiveAreaForObject(terrainOrigin, terrainSize, box2.position, box2.scale, activeAreas);
        // glm::vec3 sizeTires = glm::vec3(2, 2, 2);
        // setActiveAreaForObject(terrainOrigin, terrainSize, tire.position, sizeTires, activeAreas);
        timing.end("UPDATE_ACTIVE_AREAS");

        // Render snow cover depth map
        timing.begin("RENDER_SNOW_DEPTH");
        glViewport(0, 0, textureSizeSnowHeightmap, textureSizeSnowHeightmap);
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
        timing.begin("DISTANCE_TRANSFORM_AND_DSTR");
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            createPenetartionTextureOperation.activeArea = activeAreas[i];
            createPenetartionTextureOperation.doClear = false;
            createPenetartionTextureOperation.execute(fboSnowCoverDepth.textureId, ground.heightmap);
        }

        for (unsigned i = 0; i < activeAreas.size(); i++) {
            // jumpFloodInitOperation.activeArea = activeAreas[i];
            // jumpFloodInitOperation.execute(createPenetartionTextureOperation.getTextureResult(), obsticleMap);
            jumpFloodMainOperation.activeArea = activeAreas[i];
            jumpFloodMainOperation.passIndex = 0;
            jumpFloodMainOperation.execute(createPenetartionTextureOperation.getTextureResult(), 0);
        }

        // SSBO approach to move to contour
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            buildDeltaSnowSSBOBufferOperation.activeArea = activeAreas[i];
            buildDeltaSnowSSBOBufferOperation.execute(jumpFloodMainOperation.getTextureResult(), 0);

            combineSSBOWithHeightmap.activeArea = activeAreas[i];
            combineSSBOWithHeightmap.execute(ground.heightmap, 0);
        }
        ground.heightmap = combineSSBOWithHeightmap.getTextureResult();

        // Iterative approach to move to contour
        // for (unsigned i = 0; i < activeAreas.size(); i++) {
        //     calculateNumNeighborsWithLessContourValue.activeArea = activeAreas[i];
        //     calculateNumNeighborsWithLessContourValue.execute(jumpFloodMainOperation.getTextureResult(), 0);

        //     distributeToLowerContourValues.activeArea = activeAreas[i];
        //     distributeToLowerContourValues.execute(calculateNumNeighborsWithLessContourValue.getTextureResult(), 0);

        //     combineDistributedTerrainWithHeightmap.activeArea = activeAreas[i];
        //     combineDistributedTerrainWithHeightmap.execute(distributeToLowerContourValues.getTextureResult(), ground.heightmap);
        // }
        // ground.heightmap = combineDistributedTerrainWithHeightmap.getTextureResult();
        

        timing.end("DISTANCE_TRANSFORM_AND_DSTR");
        // Even out steep slopes SSBO

            // Move heightdata to SSBO
            // Iteratively run shader on ssbo <-- ping pong
            // combine ssbo back to heightmap

        // Even out steep slopes (2 step process)
        timing.begin("EROSION");
        if (useErosion) {
            GLuint hmap;
            for (unsigned int i = 0; i < activeAreas.size(); i++) {
                setChannels.activeArea = activeAreas[i];
                calcAvgHeight.activeArea = activeAreas[i];
                evenOutHeights1.activeArea = activeAreas[i];
                evenOutHeights2.activeArea = activeAreas[i];

                setChannels.execute(ground.heightmap, obsticleMap);

                GLuint calcAvgHeightInput = setChannels.getTextureResult();
                hmap = ground.heightmap;
                for (int j = 0; j < numErosionStepsPerFrame; j++) {
                    calcAvgHeight.execute(calcAvgHeightInput, 0);
                    GLuint avgHeightTexture = calcAvgHeight.getTextureResult();

                    ErosionOperation activeEvenOutHeightOperation = evenOutHeights1;
                    if (activeEvenOutHeightOperation.getTextureResult() == hmap) {
                        activeEvenOutHeightOperation = evenOutHeights2;
                    }
                    activeEvenOutHeightOperation.execute(avgHeightTexture, 0);
                    GLuint erodedTexture = activeEvenOutHeightOperation.getTextureResult();
                    calcAvgHeightInput = erodedTexture;
                    hmap = erodedTexture;
                }
            }

            erosionResultToHeightmap.execute(hmap, 0);
            ground.heightmap = erosionResultToHeightmap.getTextureResult();
        }
        timing.end("EROSION");

        // Write updates to the FBO for the final heightmap
        timing.begin("COPY_TO_NEW_HEIGHTMAP");
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            copyTextureHeightMapProgram.activeArea = activeAreas[i];
            copyTextureHeightMapProgram.execute(ground.heightmap, 0);
        }
        ground.heightmap = copyTextureHeightMapProgram.getTextureResult();
        timing.end("COPY_TO_NEW_HEIGHTMAP");

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

        // Render to screen
        timing.begin("RENDER_TO_SCREEN");
        // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
        // glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
        ground.render(shaderProgramTerrain, worldToCamera, perspective);
        box.render(shaderProgramDefault, worldToCamera, perspective);
        box2.render(shaderProgramDefault, worldToCamera, perspective);
        tire.render(shaderProgramDefault, worldToCamera, perspective);
        footstep.render(shaderProgramDefault, worldToCamera, perspective);

        // Render helper quads
        // quad.textureId = ...
        quad.render(shaderProgramQuad, unitMatrix, orthoUI);
        quadLL.textureId = calculateNumNeighborsWithLessContourValue.getTextureResult();
        quadLL.render(shaderProgramQuad, unitMatrix, orthoUI);
        timing.end("RENDER_TO_SCREEN");

        keysPressed.clear();

        glfwPollEvents();
        timing.begin("SWAP_BUFFERS");
        glfwSwapBuffers(window);
        timing.end("SWAP_BUFFERS");

        if (frameCounterGlobal == 0) {
        }
            readBackAndAccumulatePixelValue(copyTextureHeightMapProgram.fbo.fboId, textureSizeSnowHeightmap, TextureFormat::R32UI);

        if (frameCounterGlobal % 100 == 0) {
            timing.print();
            readBackAndAccumulatePixelValue(copyTextureHeightMapProgram.fbo.fboId, textureSizeSnowHeightmap, TextureFormat::R32UI);
        }

        frameCounterGlobal++;
    }

    return 0;
}