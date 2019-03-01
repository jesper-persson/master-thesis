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
const float textureSizeSnowHeightmap = 300;
const int occlusionTextureWidth = WINDOW_WIDTH / 1.0;
const int occlusionTextureHeight = WINDOW_HEIGHT / 1.0;

#include "texture.cpp"
#include "shader.cpp"
#include "box.cpp"
#include "quad.cpp"
#include "terrain.cpp"
#include "fbo.cpp"
#include "animatedFootsteps.cpp"
#include "ssao.cpp"
#include "textureOperations.cpp"

using namespace std;



map<int, bool> keysDown{};

bool isKeyDown(int key)
{
    return keysDown[key];
}

class Camera {
public:
    glm::vec3 position = glm::vec3(0.0f, 10.0f, 10.0f);
    glm::vec3 up = glm::vec3(0, 1.0f, 0);
    glm::vec3 forward = glm::normalize(glm::vec3(0.0f, 0, -1.0f));
};

void updateCamera(Camera &camera, float dt)
{
    float speed = 15.0f * dt;
    float rotationSpeed = speed / 8.0f;
    glm::vec3 left = glm::cross(camera.up, camera.forward);
    if (isKeyDown(GLFW_KEY_W))
    {
        camera.position = camera.position + camera.forward * speed;
    }
    if (isKeyDown(GLFW_KEY_S))
    {
        camera.position = camera.position - camera.forward * speed;
    }
    if (isKeyDown(GLFW_KEY_A))
    {
        camera.position = camera.position + left * speed;
    }
    if (isKeyDown(GLFW_KEY_D))
    {
        camera.position = camera.position - left * speed;
    }
    if (isKeyDown(GLFW_KEY_LEFT))
    {
        camera.forward = glm::normalize(glm::rotate(camera.forward, rotationSpeed, glm::vec3(0, 1, 0)));
        camera.up = glm::normalize(glm::rotate(camera.up, rotationSpeed, glm::vec3(0, 1, 0)));
    }
    if (isKeyDown(GLFW_KEY_RIGHT))
    {
        camera.forward = glm::normalize(glm::rotate(camera.forward, -rotationSpeed, glm::vec3(0, 1, 0)));
        camera.up = glm::normalize(glm::rotate(camera.up, -rotationSpeed, glm::vec3(0, 1, 0)));
    }
    if (isKeyDown(GLFW_KEY_UP))
    {
        camera.forward = glm::normalize(glm::rotate(camera.forward, rotationSpeed, left));
        camera.up = glm::normalize(glm::rotate(camera.up, rotationSpeed, left));
    }
    if (isKeyDown(GLFW_KEY_DOWN))
    {
        camera.forward = glm::normalize(glm::rotate(camera.forward, -rotationSpeed, left));
        camera.up = glm::normalize(glm::rotate(camera.up, -rotationSpeed, left));
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        keysDown[key] = true;
    }
    if (action == GLFW_RELEASE)
    {
        keysDown[key] = false;
    }
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

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

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
    GLuint shaderProgramCalcNormals = createShaderProgram("shaders/basicVS.glsl", "shaders/calcNormalFS.glsl");

    glm::mat4 perspective = glm::perspectiveFov(1.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 1.0f, 100.0f);
    glm::mat4 orthoUI = glm::ortho(-(float)WINDOW_WIDTH/2.0f,(float)WINDOW_WIDTH/2.0f,-(float)WINDOW_HEIGHT/2.0f,(float)WINDOW_HEIGHT/2.0f,-20.0f,20.0f);
    glm::mat4 orthoFBO = glm::ortho(-1 * 15.0f,1 * 15.0f,-1 * 15.0f,1 * 15.0f,0.0f,10.0f);
    glm::mat4 unitMatrix = glm::mat4(1.0f);

    Camera camera;
    Box box;
    box.scale = glm::vec3(2, 4, 2);
    box.position = glm::vec3(0, 10.5f, 0);
    box.textureId = loadPNGTexture("images/red.png");
    box.normalMap = loadPNGTexture("images/normalmap3.png");
    box.useNormalMapping = true;
    Box box2;
    box2.scale = glm::vec3(2, 2, 2);
    box2.position = glm::vec3(-6.5f, 9.0f, 2.0f);
    box2.textureId = loadPNGTexture("images/red.png");
    box2.normalMap = loadPNGTexture("images/normalmap3.png");
    box2.useNormalMapping = true;
    Terrain ground;
    ground.scale = glm::vec3(1, 1, 1);
    ground.position = glm::vec3(-15.0f, 0.0f, -15.0f);
    ground.textureId = loadPNGTexture("images/white.png");
    ground.normalmap = loadPNGTexture("images/normalmap5.png");
    ground.heightmap = createTextureForHeightmap(textureSizeSnowHeightmap);

    float quadSize = 300;
    Quad quad;
    quad.scale = glm::vec3(quadSize, quadSize, 1);
    quad.position = glm::vec3(WINDOW_WIDTH / 2.0f - quadSize / 2.0f, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f, 0);
    Quad quadLL;
    quadLL.scale = glm::vec3(quadSize, quadSize, 1);
    quadLL.position = glm::vec3(-WINDOW_WIDTH / 2.0f + quadSize / 2.0f, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f, 0);

    FBOWrapper fboDepthSSAO = createFBOForDepthTexture(occlusionTextureWidth, occlusionTextureHeight);
    FBOWrapper fboOcclusionStep = createFrameBufferSingleTexture(occlusionTextureWidth, occlusionTextureHeight);
    FBOWrapper fboSnowCoverDepth = createFBOForDepthTexture(textureSizeSnowHeightmap, textureSizeSnowHeightmap);

    PingPongTextureOperation blurOcclusion = PingPongTextureOperation(occlusionTextureHeight, occlusionTextureHeight, shaderProgramLowpass, 4);
    PingPongTextureOperation blurSnowHeightmap = PingPongTextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramLowpass, 4);
    TextureOperation calcNormals = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramCalcNormals);
    TextureOperation minTextureOperation1 = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramPixelMin);
    TextureOperation minTextureOperation2 = TextureOperation(textureSizeSnowHeightmap, textureSizeSnowHeightmap, shaderProgramPixelMin);

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

    Footstep footstep;
    footstep.box.position.x = -15;
    footstep.box.ssaoMap = fboDepthSSAO.textureId;
    footstep.box.samples = samples;
    footstep.box.numSamples = kernelSize;
    footstep.box.randomTexture = noiseTexture;

    int i = 0;
    double previousTimeFPS = glfwGetTime();
    double previousTime = glfwGetTime();
    int frameCounter = 0;
    float dt = 0;

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1, 0.5, 0.5, 1);
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
        updateCamera(camera, dt);
        glm::vec3 cameraLookAtPosition = camera.position + camera.forward * 10.0f;
        glm::mat4 worldToCamera = glm::lookAt(camera.position, cameraLookAtPosition, camera.up);
        footstep.update(dt);
        box.position.y = 10.2f + 2 * sin(i*0.01f);
        box.position.x = 4 + 6 * cos(i*0.01f);
        box.position.z = 2 + 6 * sin(i*0.01f);
        box.rotation = glm::rotate(glm::mat4(1.0f), i * 0.001f, glm::vec3(0.0f, 1.0f, 0.0f));

        // Render snow cover depth map
        glm::mat4 worldToCameraDepth = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
        glViewport(0, 0, textureSizeSnowHeightmap, textureSizeSnowHeightmap);
        glBindFramebuffer(GL_FRAMEBUFFER, fboSnowCoverDepth.fboId);
        glClearColor(0.5, 0.1, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        box.render(shaderProgram, worldToCameraDepth, orthoFBO);
        box2.render(shaderProgram, worldToCameraDepth, orthoFBO);
        footstep.render(shaderProgram, worldToCameraDepth, orthoFBO);
        // ground.render(shaderProgramTerrain, worldToCameraDepth, orthoFBO);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

         // Calculate new height map
        TextureOperation to = minTextureOperation1;
        if (to.fbo.textureId == ground.heightmap) {
            to = minTextureOperation2;
        } 
        to.execute(ground.heightmap, fboSnowCoverDepth.textureId);
        ground.heightmap = to.getTextureResult();

        calcNormals.execute(ground.heightmap, 0);
        blurSnowHeightmap.execute(calcNormals.getTextureResult(), 0);
        ground.normalmapMacro = blurSnowHeightmap.getTextureResult();

        // Render to SSAO depth buffer
        glViewport(0, 0, occlusionTextureWidth, occlusionTextureHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, fboDepthSSAO.fboId);
        glClearColor(0.5, 0.1, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        box.render(shaderProgram, worldToCamera, perspective);
        box2.render(shaderProgram, worldToCamera, perspective);
        footstep.render(shaderProgram, worldToCamera, perspective);
        ground.render(shaderProgramTerrain, worldToCamera, perspective);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Calculate occlusion texture
        glViewport(0, 0, occlusionTextureWidth, occlusionTextureHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, fboOcclusionStep.fboId);
        glClearColor(0.5, 0.1, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        box.render(shaderProgramOcclusionStep, worldToCamera, perspective);
        box2.render(shaderProgramOcclusionStep, worldToCamera, perspective);      
        footstep.render(shaderProgramOcclusionStep, worldToCamera, perspective);
        ground.render(shaderProgramOcclusionStepTerrain, worldToCamera, perspective);      
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Blur occlusion texture
        blurOcclusion.execute(fboOcclusionStep.textureId, 0);

        ground.occlusionMap = blurOcclusion.getTextureResult();
        box.occlusionMap = blurOcclusion.getTextureResult();
        box2.occlusionMap = blurOcclusion.getTextureResult();
        footstep.box.occlusionMap = blurOcclusion.getTextureResult();

        // Render to screen
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
        ground.render(shaderProgramTerrain, worldToCamera, perspective);
        box.render(shaderProgram, worldToCamera, perspective);
        box2.render(shaderProgram, worldToCamera, perspective);
        footstep.render(shaderProgram, worldToCamera, perspective);
        
        // Render helper quads
        quad.textureId = minTextureOperation1.getTextureResult();
        // quad.textureId = ground.normalmapMacro;
        quad.render(shaderProgramQuad, unitMatrix, orthoUI);
        quadLL.textureId = fboOcclusionStep.textureId;
        // quadLL.textureId = fboSnowCoverDepth.textureId;
        quadLL.render(shaderProgramQuad, unitMatrix, orthoUI);

        i++;
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return 0;
}