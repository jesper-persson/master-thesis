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

// Non-configurable parameters
const int heightColumnScale = 10000; // Keep in mind that some shaders use a value that is semi-dependant on this

// These are set in a settings file
float activeAreaMargin;
int windowHeight;
int windowWidth;
int frustumHeight;
bool useSSBO;
bool useMultipleTargets;
int penetrationDivider;
bool fullscreen;
float terrainSize; // World space size of terrain mesh.
int numVerticesPerRow; // Resolution of terrain mesh.
float heightmapSize;
float compression; // 0 implies nothing compressed
float roughness;
float slopeThreshold;
int numIterationsEvenOutSlopes;
int numIterationsBlurNormals;
int numIterationsDisplaceMaterialToContour;

#include "texture.cpp"
#include "shader.cpp"
#include "model.cpp"
#include "quad.cpp"
#include "terrain.cpp"
#include "barTerrain.cpp"
#include "fbo.cpp"
#include "animatedFootsteps.cpp"
#include "textureOperations.cpp"
#include "timing.cpp"
#include "settings.cpp"

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
    glm::vec3 position = glm::vec3( 13.41851043701171875000, 22.74206542968750000000, 0.00000000000000000000);
    glm::vec3 forward = glm::normalize(glm::vec3(-0.72402071952819824219, -0.68977826833724975586, 0.00000000000000000000));
    glm::vec3 up = glm::normalize(glm::vec3(-0.68977886438369750977, 0.72402012348175048828, 0.00000000000000000000));
};

class RigidBody {
public:
    glm::vec3 velocity = glm::vec3(0, 0,0 );
};

void controlRigidBody(glm::vec3 &position, glm::vec3 &forward, glm::vec3 &up, RigidBody &rb,float dt) {
     glm::vec3 carForward = -1.0f * forward;
    float speed = 18.0f * dt;
    glm::vec3 carLeft = glm::cross(up, carForward);
    
    if (isKeyDown(GLFW_KEY_W)) {
        rb.velocity = rb.velocity + carForward * speed;
    }
    if (isKeyDown(GLFW_KEY_S)) {
        rb.velocity = rb.velocity - carForward * speed;
    }
    if (glm::length(rb.velocity) > 15.0f) {
        rb.velocity = glm::normalize(rb.velocity) * 15.0f;
    }
    if (glm::length(rb.velocity) > 0.00001) {
        float friction = 0.08f * dt;
        rb.velocity = rb.velocity + glm::normalize(rb.velocity) * glm::length(rb.velocity) * -1.0f * friction;
    }

    position = position + rb.velocity * dt;

    float rotationSpeed =  glm::length(rb.velocity) * 0.009f;
    rotationSpeed = min(rotationSpeed, 0.01f);
    if (isKeyDown(GLFW_KEY_LEFT)) {
        forward = glm::normalize(glm::rotate(forward, rotationSpeed, glm::vec3(0, 1, 0)));
        up = glm::normalize(glm::rotate(up, rotationSpeed, glm::vec3(0, 1, 0)));
    }
    if (isKeyDown(GLFW_KEY_RIGHT)) {
        forward = glm::normalize(glm::rotate(forward, -rotationSpeed, glm::vec3(0, 1, 0)));
        up = glm::normalize(glm::rotate(up, -rotationSpeed, glm::vec3(0, 1, 0)));
    }

}

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
    int size = boundingBoxSize.x * ratio * activeAreaMargin;

    int posX = (int)(diff.x);
    int posZ = (int)(diff.z);

    if (size % 2 != 0) {
        size += 1;
    }

    activeAreas.push_back(ActiveArea(posX, posZ, size, size));
}

int main(int argc, char* argv[]) {
    // Load terrain settings
    if (argc > 1) {
        applySettings(argv[1]);
    } else {
        cerr << "Missing settings file" << endl;
        return 1;
    }

    // Set up OpenGL
    if (!glfwInit()) {
        cerr << "glfwInit() failed" << endl;
    }

    string title = "Terrain deformation";
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    GLFWwindow *window;
    if (fullscreen == true) {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        window = glfwCreateWindow(mode->width, mode->height, title.c_str(), monitor, NULL);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        windowWidth = mode->width;
        windowHeight = mode->height;
    } else {
        window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), NULL, NULL);
    }
    if (!window) {
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

    glm::vec3 terrainOrigin = glm::vec3(0, 0, 0);

    // Create objects in the scene
    Model car1 = loadUsingTinyObjLoader("resources/Jeep.obj");
    car1.scale = glm::vec3(0.07f, 0.07f, 0.07f);
    car1.position = glm::vec3(30.0f, 4.0f, 30.0f);
    car1.textureId = loadPNGTexture("resources/darkgray.png");
    car1.useNormalMapping = false;
    RigidBody carRigidBody;

    Model box = Box::createBox();
    box.scale = glm::vec3(8, 6, 8);
    box.position = glm::vec3(0, 6.0f, 0);
    box.forward = glm::normalize(glm::vec3(1, 0, 1));
    box.textureId = loadPNGTexture("resources/blue.png");
    box.useNormalMapping = false;

    Footstep footstep1(false);
    Footstep footstep2(true);

    Terrain ground(numVerticesPerRow);
    // BarTerrain ground(heightmapSize);
    ground.scale = glm::vec3(terrainSize, 1, terrainSize);
    ground.position = terrainOrigin;
    ground.textureId = loadPNGTexture("resources/white.png");
    ground.normalmap = loadPNGTexture("resources/normalmap2.png");
    ground.heightColumnScale = heightColumnScale;
    ground.heightmap = createTextureForHeightmap(heightmapSize);
    // ground.heightmap = loadPNGTextureForHeightmap("resources/terrain1.png");

    Camera camera;

    // When 0 = camera, 1 = box, 2 = car
    int objectControl = 2;

    float quadSize = 300;
    Quad quad;
    quad.scale = glm::vec3(quadSize, quadSize, 1);
    quad.position = glm::vec3(windowWidth / 2.0f - quadSize / 2.0f - 20, -windowHeight / 2.0f + quadSize / 2.0f + 20, 0);

    Quad quadLL;
    quadLL.scale = glm::vec3(quadSize, quadSize, 1);
    quadLL.position = glm::vec3(-windowWidth / 2.0f + quadSize / 2.0f + 10, -windowHeight / 2.0f + quadSize / 2.0f + 10, 0);

    // Projection matrices
    glm::mat4 perspective = glm::perspectiveFov(1.0f, (float)windowWidth, (float)windowHeight, 1.0f, 500.0f);
    glm::mat4 orthoUI = glm::ortho(-(float)windowWidth/2.0f,(float)windowWidth/2.0f,-(float)windowHeight/2.0f,(float)windowHeight/2.0f,-20.0f,20.0f);
    glm::mat4 terrainDepthProjection = glm::ortho(-1 * terrainSize / 2.0f, 1 * terrainSize / 2.0f, -1 * terrainSize / 2.0f, 1 * terrainSize / 2.0f, 0.0f, static_cast<float>(frustumHeight));
    glm::mat4 unitMatrix = glm::mat4(1.0f);

    // Render to screen shaders
    GLuint shaderProgramDefault = createShaderProgram("shaders/basicVS.glsl", "shaders/basicFS.glsl");
    GLuint shaderProgramTerrain = createShaderProgram("shaders/terrainVS.glsl", "shaders/terrainTCS.glsl", "shaders/terrainTES.glsl", "shaders/terrainFS.glsl");
    GLuint shaderProgramBarTerrain = createShaderProgram("shaders/barTerrainVS.glsl", "shaders/barTerrainTCS.glsl", "shaders/barTerrainTES.glsl", "shaders/barTerrainFS.glsl");
    GLuint shaderProgramQuad = createShaderProgram("shaders/basicVS.glsl", "shaders/quad.glsl");

    FBOWrapper FBODepthTexture = createFBOForDepthTexture(heightmapSize, heightmapSize);
    glm::mat4 worldToCameraDepth = glm::lookAt(terrainOrigin, terrainOrigin + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));

    float heightScale = heightmapSize / terrainSize;
    
    // Create texture operation programs
    CalculateNormals calculateNormals = CalculateNormals(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/calculateNormals.glsl"), heightScale, heightColumnScale);
    CreateInitialHeightmapTexture createInitialHeightmapTexture = CreateInitialHeightmapTexture(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/createInitialHeightmapTexture.glsl"), TextureFormat::R32UI, frustumHeight, heightColumnScale);
    DisplaceMaterialSSBO displaceMaterialSSBO = DisplaceMaterialSSBO(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/displaceMaterialSSBO.glsl"), TextureFormat::R32I, compression);
    DisplaceMaterialSSBOMultipleTargets displaceMaterialSSBOMultipleTargets = DisplaceMaterialSSBOMultipleTargets(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/displaceMaterialSSBOMultipleTargets.glsl"), TextureFormat::R32I, compression, penetrationDivider);
    TextureOperation moveSBBOValuesToHeightmap = TextureOperation(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/moveSBBOValuesToHeightmap.glsl"), TextureFormat::R32UI);
    EvenOutSteepSlopes evenOutSteepSlopesPart1 = EvenOutSteepSlopes(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/evenOutSteepSlopesPart1.glsl"), TextureFormat::RGBA32UI, frustumHeight, heightColumnScale, terrainSize, slopeThreshold, roughness);
    EvenOutSteepSlopes evenOutSteepSlopesPart2 = EvenOutSteepSlopes(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/evenOutSteepSlopesPart2.glsl"), TextureFormat::RG32UI, frustumHeight, heightColumnScale, terrainSize, slopeThreshold, roughness);
    TextureOperation moveEvenedOutHeightsToHeightmap = TextureOperation(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/moveEvenedOutHeightsToHeightmap.glsl"), TextureFormat::R32UI);
    EvenOutSteepSlopes evenOutSteepSlopesSSBO = EvenOutSteepSlopes(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/evenOutSteepSlopesSSBO.glsl"), TextureFormat::R32I, frustumHeight, heightColumnScale, terrainSize, slopeThreshold, roughness);
    CalculateNumNeighborsWithLessContourValue calculateNumNeighborsWithLessContourValue = CalculateNumNeighborsWithLessContourValue(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/calculateNumNeighborsWithLessContourValue.glsl"), TextureFormat::RGBA32I, compression);
    PingPongTextureOperation displaceMaterialToLowerContourValues = PingPongTextureOperation(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/displaceMaterialToLowerContourValues.glsl"), numIterationsDisplaceMaterialToContour, TextureFormat::RGBA32I);
    CombineDisplacedMaterialWithHeightmap combineDisplacedMaterialWithHeightmap = CombineDisplacedMaterialWithHeightmap(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/combineDisplacedMaterialWithHeightmap.glsl"), TextureFormat::RG32UI, frustumHeight, heightColumnScale);
    IntHeightmapToFloat intHeightmapToFloat = IntHeightmapToFloat(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/intHeightmapToFloat.glsl"), TextureFormat::R32F, heightColumnScale);
    JumpFlooding jumpFlooding = JumpFlooding(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/jumpFlooding.glsl"), log2(heightmapSize));
    GLuint createPenetrationTextureShader = createShaderProgram("shaders/basicVS.glsl", "shaders/createPenetrationTexture.glsl");
    CreatePenetrationTexture createPenetrationTextureOperation = CreatePenetrationTexture(heightmapSize, heightmapSize, createPenetrationTextureShader, TextureFormat::RGBA32I, frustumHeight, heightColumnScale);
    PingPongTextureOperation blur = PingPongTextureOperation(heightmapSize, heightmapSize, createShaderProgram("shaders/basicVS.glsl", "shaders/blur.glsl"), 4, TextureFormat::RGBA16F);
    blur.timesRepeat = numIterationsBlurNormals;
    blur.sampling = GL_LINEAR;
    blur.inputTexturesAreFloat = true;

    // Scales heightmap to correct range
    createInitialHeightmapTexture.execute(ground.heightmap, 0);
    ground.heightmap = createInitialHeightmapTexture.getTextureResult();

    // When updating the scene, only a subpart of the heightmap is updates. So we must
    // begin by storing the entire heightmap.
    if (useSSBO) {
        moveSBBOValuesToHeightmap.fbo.setOutputTexture(createInitialHeightmapTexture.getTextureResult());
        ground.heightmap = moveSBBOValuesToHeightmap.getTextureResult();
    } else {
        moveEvenedOutHeightsToHeightmap.fbo.setOutputTexture(createInitialHeightmapTexture.getTextureResult());
        ground.heightmap = moveEvenedOutHeightsToHeightmap.getTextureResult();
    }
    intHeightmapToFloat.execute(ground.heightmap, 0);
    calculateNormals.execute(ground.heightmap, 0);
    blur.execute(calculateNormals.getTextureResult(), 0);
    ground.normalmapMacro = blur.getTextureResult();

    // SSBO for material displacement
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

    // cout << "Initial volume of terrain: " << endl;
    // readBackAndAccumulatePixelValue(createInitialHeightmapTexture.fbo.fboId, heightmapSize, TextureFormat::R32UI);

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
            objectControl = (objectControl + 1) % 3;
            // cout << camera.position.x << ", " << camera.position.y <<  ", " << camera.position.z << endl;
            // cout << camera.forward.x << ", " << camera.forward.y <<  ", " << camera.forward.z << endl;
            // cout << camera.up.x << ", " << camera.up.y <<  ", " << camera.up.z << endl;
        }
        if (isKeyPressed(GLFW_KEY_ESCAPE)) {
            break;
        }

        if (objectControl == 1) {
            controlObject(box.position, box.forward, box.up, dt);
            glm::vec3 fromPosition = box.position - box.forward * 16.0f + box.up * 14.0f;
            worldToCamera = glm::lookAt(fromPosition, box.position, box.up);
        } else if (objectControl == 0) {
            controlObject(camera.position, camera.forward, camera.up, dt);
        } else if (objectControl == 2) {
            controlRigidBody(car1.position, car1.forward, car1.up, carRigidBody, dt);
            glm::vec3 targetPosition = car1.position;
            // glm::vec3 targetPosition = (footstep1.shoes.position + footstep2.shoes.position) / 2.0f;;
            float cameraSpeed = 10.0f * dt;
            glm::vec3 toCar = glm::normalize(targetPosition - camera.position);
            if (glm::length(targetPosition - camera.position) > 4) {
                camera.position = camera.position + toCar * cameraSpeed;
            }
            glm::vec3 fromPosition = camera.position + glm::vec3(0,1,0) * 30.0f;
            worldToCamera = glm::lookAt(fromPosition, targetPosition, glm::vec3(0,1,0));
        }

        footstep1.update(dt);
        footstep2.update(dt);

        timing.end("UPDATE_OBJECTS");

        // Update active areas
        timing.begin("UPDATE_ACTIVE_AREAS");
        activeAreas.clear();
        glm::vec3 footArea = glm::vec3(3, 6, 6);
        glm::vec3 carArea = glm::vec3(6, 6, 6);
        setActiveAreaForObject(terrainOrigin, terrainSize, car1.position, carArea, activeAreas);
        // setActiveAreaForObject(terrainOrigin, terrainSize, footstep1.shoes.position, footArea, activeAreas);
        // setActiveAreaForObject(terrainOrigin, terrainSize, box.position, box.scale, activeAreas);
        timing.end("UPDATE_ACTIVE_AREAS");

        // Render depth texture
        timing.begin("RENDER_DEPTH_TEXTURE");
        glViewport(0, 0, heightmapSize, heightmapSize);
        glBindFramebuffer(GL_FRAMEBUFFER, FBODepthTexture.fboId);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // box.render(shaderProgramDefault, worldToCameraDepth, terrainDepthProjection);
        car1.render(shaderProgramDefault, worldToCameraDepth, terrainDepthProjection);
        // footstep1.render(shaderProgramDefault, worldToCameraDepth, terrainDepthProjection);
        // footstep2.render(shaderProgramDefault, worldToCameraDepth, terrainDepthProjection);
        glViewport(0, 0, windowWidth, windowHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        timing.end("RENDER_DEPTH_TEXTURE");

        GLuint obstacleMap = FBODepthTexture.textureId;

        timing.begin("CREATE_PENETRATION_TEXTURE");
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            createPenetrationTextureOperation.activeArea = activeAreas[i];
            createPenetrationTextureOperation.execute(FBODepthTexture.textureId, ground.heightmap);
        }
        timing.end("CREATE_PENETRATION_TEXTURE");

        timing.begin("JUMP_FLOODING");
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            jumpFlooding.activeArea = activeAreas[i];
            jumpFlooding.passIndex = 0;
            jumpFlooding.execute(createPenetrationTextureOperation.getTextureResult(), 0);
        }
        timing.end("JUMP_FLOODING");

        // SSBO approach
        if (useSSBO) {
            timing.begin("SSBO_MOVE_TO_CONTOUR");
            if (useMultipleTargets) {
                for (unsigned i = 0; i < activeAreas.size(); i++) {
                    displaceMaterialSSBOMultipleTargets.activeArea = activeAreas[i];
                    displaceMaterialSSBOMultipleTargets.execute(jumpFlooding.getTextureResult(), ground.heightmap);
                }
            } else {
                for (unsigned i = 0; i < activeAreas.size(); i++) {
                    displaceMaterialSSBO.activeArea = activeAreas[i];
                    displaceMaterialSSBO.execute(jumpFlooding.getTextureResult(), ground.heightmap);
                }
            }
            timing.end("SSBO_MOVE_TO_CONTOUR");

            timing.begin("SSBO_EVEN_OUT");
            for (unsigned i = 0; i < activeAreas.size(); i++) {
                for (int j = 0; j < numIterationsEvenOutSlopes; j++) {
                    evenOutSteepSlopesSSBO.activeArea = activeAreas[i];
                    evenOutSteepSlopesSSBO.execute(obstacleMap, 0);
                }
            }

            for (unsigned i = 0; i < activeAreas.size(); i++) {
                moveSBBOValuesToHeightmap.activeArea = activeAreas[i];
                moveSBBOValuesToHeightmap.execute(0, 0);
            }
            ground.heightmap = moveSBBOValuesToHeightmap.getTextureResult();
            timing.end("SSBO_EVEN_OUT");
        }

        // Iterative approach
        if (!useSSBO) {
            timing.begin("ITERATIVE_MOVE_TO_CONTOUR");
            for (unsigned i = 0; i < activeAreas.size(); i++) {
                calculateNumNeighborsWithLessContourValue.activeArea = activeAreas[i];
                calculateNumNeighborsWithLessContourValue.execute(jumpFlooding.getTextureResult(), 0);
            }
            for (unsigned i = 0; i < activeAreas.size(); i++) {
                displaceMaterialToLowerContourValues.activeArea = activeAreas[i];
                displaceMaterialToLowerContourValues.execute(calculateNumNeighborsWithLessContourValue.getTextureResult(), 0);
            }
            for (unsigned i = 0; i < activeAreas.size(); i++) {
                combineDisplacedMaterialWithHeightmap.activeArea = activeAreas[i];
                combineDisplacedMaterialWithHeightmap.obstacleMap = obstacleMap;
                combineDisplacedMaterialWithHeightmap.execute(displaceMaterialToLowerContourValues.getTextureResult(), ground.heightmap);
            }
            timing.end("ITERATIVE_MOVE_TO_CONTOUR");

            timing.begin("ITERATIVE_EVEN_OUT");
            for (unsigned int i = 0; i < activeAreas.size(); i++) {
                evenOutSteepSlopesPart1.activeArea = activeAreas[i];
                evenOutSteepSlopesPart2.activeArea = activeAreas[i];

                GLuint calcAvgHeightInput = combineDisplacedMaterialWithHeightmap.getTextureResult();
                for (int j = 0; j < numIterationsEvenOutSlopes; j++) {
                    evenOutSteepSlopesPart1.execute(calcAvgHeightInput, 0);
                    evenOutSteepSlopesPart2.execute(evenOutSteepSlopesPart1.getTextureResult(), 0);
                    calcAvgHeightInput = evenOutSteepSlopesPart2.getTextureResult();
                }

                moveEvenedOutHeightsToHeightmap.activeArea = activeAreas[i];
                moveEvenedOutHeightsToHeightmap.execute(evenOutSteepSlopesPart2.getTextureResult(), 0);
            }
            ground.heightmap = moveEvenedOutHeightsToHeightmap.getTextureResult();
            timing.end("ITERATIVE_EVEN_OUT");
        }

        timing.begin("CALC_NORMALS");
        for (unsigned i = 0; i < activeAreas.size(); i++) {
            calculateNormals.activeArea = activeAreas[i];
            calculateNormals.execute(ground.heightmap, 0);
        }
        if (numIterationsBlurNormals > 0) {
            for (unsigned i = 0; i < activeAreas.size(); i++) {
                blur.activeArea = activeAreas[i];
                blur.execute(calculateNormals.getTextureResult(), 0);
            }
            ground.normalmapMacro = blur.getTextureResult();
        } else {
            ground.normalmapMacro = calculateNormals.getTextureResult();
        }
        timing.end("CALC_NORMALS");

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
        car1.render(shaderProgramDefault, worldToCamera, perspective);
        // box.render(shaderProgramDefault, worldToCamera, perspective);
        // footstep1.render(shaderProgramDefault, worldToCamera, perspective);
        // footstep2.render(shaderProgramDefault, worldToCamera, perspective);

        // Render helper quads
        // quad.textureId = createPenetrationTextureOperation.getTextureResult();
        // quad.render(shaderProgramQuad, unitMatrix, orthoUI);
        // quadLL.textureId = createPenetrationTextureOperation.getTextureResult();
        // quadLL.render(shaderProgramQuad, unitMatrix, orthoUI);
        timing.end("RENDER_TO_SCREEN");

        keysPressed.clear();

        glfwPollEvents();
        timing.begin("SWAP_BUFFERS");
        glfwSwapBuffers(window);
        timing.end("SWAP_BUFFERS");

        timing.numMeasurements += 1;

        if (frameCounterGlobal == 0) {
            // readBackAndAccumulatePixelValue(createInitialHeightmapTexture.fbo.fboId, heightmapSize, TextureFormat::R32UI);
        }

        if (frameCounterGlobal % 20 == 0) {
            // timing.print();
        }

        frameCounterGlobal++;
    }

    return 0;
}