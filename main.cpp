#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include "glew.h"
#include "glfw3.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/vec3.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/euler_angles.hpp"

#include "texture.cpp"
#include "shader.cpp"
#include "box.cpp"
#include "quad.cpp"
#include "terrain.cpp"
#include "fbo.cpp"
#include "textureOperations.cpp"
#include "animatedFootsteps.cpp"

using namespace std;

const int WINDOW_HEIGHT = 800;
const int WINDOW_WIDTH = 1200;

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
    float speed = 20.0f * dt;
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
    const float textureSize = 300;

    GLuint shaderProgram = createShaderProgram("shaders/basicVS.glsl", "shaders/basicFS.glsl");
    GLuint shaderProgramQuad = createShaderProgram("shaders/basicVS.glsl", "shaders/quadFS.glsl");
    GLuint shaderProgramFBO = createShaderProgram("shaders/basicVS.glsl", "shaders/fboFS.glsl");
    GLuint shaderProgramTerrain = createShaderProgram("shaders/terrainVS.glsl", "shaders/terrainTCS.glsl", "shaders/terrainTES.glsl", "shaders/basicFS.glsl");
    
    
    GLuint shaderProgramLowpass = createShaderProgram("shaders/basicVS.glsl", "shaders/blurFS.glsl");
    GLuint shaderProgramCalcNormals = createShaderProgram("shaders/basicVS.glsl", "shaders/calcNormalFS.glsl");
    FBOWrapper fboNormalsDiff = createFrameBufferSingleTexture(textureSize);

    Quad quadTextureOperations;
    GLuint shaderProgramPixelDiff = createShaderProgram("shaders/basicVS.glsl", "shaders/subtractFS.glsl");
    GLuint shaderProgramPixelMin = createShaderProgram("shaders/basicVS.glsl", "shaders/minFS.glsl");
    FBOWrapper fboDepthDiff = createFrameBufferSingleTexture(textureSize);

    glm::mat4 perspective = glm::perspectiveFov(1.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 0.01f, 1000.0f);
    glm::mat4 orthoUI = glm::ortho(-(float)WINDOW_WIDTH/2.0f,(float)WINDOW_WIDTH/2.0f,-(float)WINDOW_HEIGHT/2.0f,(float)WINDOW_HEIGHT/2.0f,-20.0f,20.0f);
    glm::mat4 perspectiveFBO = glm::perspectiveFov(1.0f, aspectRatio * 20.0f, aspectRatio * 20.0f, 0.01f, 20.0f);
    glm::mat4 orthoFBO = glm::ortho(-1 * 15.0f,1 * 15.0f,-1 * 15.0f,1 * 15.0f,0.0f,10.0f);
    
    glm::mat4 unitMatrix = glm::mat4(1.0f);

    Camera camera;
    Box box;
    box.scale = glm::vec3(4, 4, 2);
    box.position = glm::vec3(0, 12.5f, 0);
    box.textureId = loadPNGTexture("images/red.png");
    Terrain ground;
    ground.scale = glm::vec3(1, 1, 1);
    ground.position = glm::vec3(-15.0f, 0.0f, -15.0f);
    ground.textureId = loadPNGTexture("images/white.png");

    float quadSize = 300;
    // Quad lower right
    Quad quad;
    quad.scale = glm::vec3(quadSize, quadSize, 1);
    quad.position = glm::vec3(WINDOW_WIDTH / 2.0f - quadSize / 2.0f, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f, 0);
    // Quad lower left
    Quad quadLL;
    quadLL.scale = glm::vec3(quadSize, quadSize, 1);
    quadLL.position = glm::vec3(-WINDOW_WIDTH / 2.0f + quadSize / 2.0f, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f, 0);


    FBOWrapper fboLowpass1 = createFrameBufferSingleTexture(textureSize);
    FBOWrapper fboLowpass2 = createFrameBufferSingleTexture(textureSize);
    FBOWrapper fbo = createFrameBufferSingleTexture(textureSize);
    FBOWrapper fboNewHeightmap1 = createFrameBufferSingleTexture(textureSize);
    FBOWrapper fboNewHeightmap2 = createFrameBufferSingleTexture(textureSize);
    FBOWrapper fboDepth = createFBOForDepthTexture(textureSize);

    GLuint heightmap = createTextureForHeightmap(textureSize);
    ground.heightmap = heightmap;
    // ground.normalmap = fboNormalsDiff.textureId;

    int i = 0;

    double previousTimeFPS = glfwGetTime();
    double previousTime = glfwGetTime();
    int frameCounter = 0;
    float dt = 0;

    Footstep footstep;
    footstep.box.position.x = -15;
    // footstep.box.scale = glm::vec3(1, 1, 1f);

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

        // Render to texture
        glm::mat4 worldToCameraDepth = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
        glViewport(0, 0, textureSize, textureSize);
        glBindFramebuffer(GL_FRAMEBUFFER, fboDepth.fboId);
        glClearColor(0.5, 0.1, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        box.render(shaderProgram, worldToCameraDepth, orthoFBO);
        footstep.render(shaderProgram, worldToCameraDepth, orthoFBO);
        // ground.render(shaderProgramTerrain, worldToCameraDepth, orthoFBO);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Calculate offset map
        glViewport(0, 0, textureSize, textureSize);
        textureOperation(shaderProgramPixelDiff, quadTextureOperations, fboDepthDiff, ground.heightmap, fboDepth.textureId, textureSize);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        // Calculate new height map
        glViewport(0, 0, textureSize, textureSize);
        if (fboNewHeightmap1.textureId == ground.heightmap) {
            textureOperation(shaderProgramPixelMin, quadTextureOperations, fboNewHeightmap2, ground.heightmap, fboDepth.textureId, textureSize);
            ground.heightmap = fboNewHeightmap2.textureId;
        } else {
            textureOperation(shaderProgramPixelMin, quadTextureOperations, fboNewHeightmap1, ground.heightmap, fboDepth.textureId, textureSize);
            ground.heightmap = fboNewHeightmap1.textureId;
        }
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        // Calculate normal map
        glViewport(0, 0, textureSize, textureSize);
        textureOperation(shaderProgramCalcNormals, quadTextureOperations, fboNormalsDiff, ground.heightmap, 0, textureSize);
        textureOperationRepeat(2, shaderProgramLowpass, quadTextureOperations, fboLowpass1, fboLowpass2, fboNormalsDiff.textureId, 0, textureSize);
        ground.normalmap = fboNormalsDiff.textureId;
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);


        // Render to screen
        // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
        ground.render(shaderProgramTerrain, worldToCamera, perspective);
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
        box.render(shaderProgram, worldToCamera, perspective);
        footstep.render(shaderProgram, worldToCamera, perspective);
        quad.textureId = fboDepth.textureId;
        quad.render(shaderProgramQuad, unitMatrix, orthoUI);
        quadLL.textureId = fboLowpass2.textureId;
        quadLL.render(shaderProgramQuad, unitMatrix, orthoUI);

        box.position.y = 10.2f + 2 * sin(i*0.01f);
        box.position.x = 4 + 6 * cos(i*0.01f);
        box.position.z = 2 + 6 * sin(i*0.01f);

        i++;
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return 0;
}