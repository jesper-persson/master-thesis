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

#include "shader.cpp"
#include "box.cpp"
#include "quad.cpp"
#include "fbo.cpp"

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
    glm::vec3 position = glm::vec3(10.0f, 0, 10.0f);
    glm::vec3 up = glm::vec3(0, 1.0f, 0);
    glm::vec3 forward = glm::normalize(glm::vec3(-1.0f, 0, -1.0f));
};

void updateCamera(Camera &camera, float dt)
{
    float speed = 1.0f * dt;
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
    GLuint shaderProgramFBO = createShaderProgram("shaders/basicVS.glsl", "shaders/fboFS.glsl");
    glm::mat4 perspective = glm::perspectiveFov(1.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 0.01f, 1000.0f);
    glm::mat4 orthoUI = glm::ortho(-(float)WINDOW_WIDTH/2.0f,(float)WINDOW_WIDTH/2.0f,-(float)WINDOW_HEIGHT/2.0f,(float)WINDOW_HEIGHT/2.0f,-20.0f,20.0f);
    glm::mat4 perspectiveFBO = glm::perspectiveFov(1.0f, aspectRatio * 20.0f, aspectRatio * 20.0f, 0.01f, 20.0f);
    glm::mat4 orthoFBO = glm::ortho(-aspectRatio * 20.0f,aspectRatio * 20.0f,-aspectRatio * 20.0f,aspectRatio * 20.0f,0.0f,10.0f);
    
    glm::mat4 unitMatrix = glm::mat4(1.0f);

    Camera camera;
    Box box;
    box.scale = glm::vec3(4, 4, 4);
    box.position = glm::vec3(0, 5, 0);
    box.textureId = loadPNGTexture("images/red.png");
    Box ground;
    ground.scale = glm::vec3(30, 1, 30);
    ground.position = glm::vec3(0, 5, 0);
    Quad quad;
    float quadSize = 300;
    quad.scale = glm::vec3(quadSize, quadSize, 1);
    quad.position = glm::vec3(WINDOW_WIDTH / 2.0f - quadSize / 2.0f, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f, 0);

    const float textureSize = 1024;
    FBOWrapper fbo = createFrameBufferSingleTexture(textureSize);
    FBOWrapper fboDepth = createFBOForDepthTexture(textureSize);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1, 0.5, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update
        float dt = 0.016;
        updateCamera(camera, dt);
        glm::vec3 cameraLookAtPosition = camera.position + camera.forward * 10.0f;
        glm::mat4 worldToCamera = glm::lookAt(camera.position, cameraLookAtPosition, camera.up);

        // Render to texture
        glm::mat4 worldToCameraDepth = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
        glViewport(0, 0, textureSize, textureSize);
        glBindFramebuffer(GL_FRAMEBUFFER, fboDepth.fboId);
        glClearColor(0.5, 0.1, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        box.render(shaderProgram, worldToCameraDepth, orthoFBO);
        ground.render(shaderProgram, worldToCameraDepth, orthoFBO);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render to screen
        ground.render(shaderProgram, worldToCamera, perspective);
        box.render(shaderProgram, worldToCamera, perspective);
        quad.textureId = fboDepth.textureId;
        quad.render(shaderProgram, unitMatrix, orthoUI);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return 0;
}