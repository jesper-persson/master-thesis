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

#include "texture.cpp"
#include "shader.cpp"
#include "quad.cpp"
#include "fbo.cpp"
#include "textureOperations.cpp"

using namespace std;

map<int, bool> keysDown{};
map<int, bool> keysPressed{};

bool isKeyDown(int key)
{
    return keysDown[key];
}

bool isKeyPressed(int key)
{
    return keysPressed[key];
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
        keysPressed[key] = true;
    }
    if (action == GLFW_RELEASE)
    {
        keysDown[key] = false;
    }
}

class LucasKanadeProgram : public TextureOperation {
public:
    GLuint dxTexture;
    GLuint dyTexture;
    GLuint dtTexture;
    GLuint current;

    LucasKanadeProgram(int textureWidth, int textureHeight, GLuint shaderProgram) :  TextureOperation(textureWidth, textureHeight, shaderProgram) {
    }

    void bindUniforms() override {
        TextureOperation::bindUniforms();
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, dxTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "dxTexture"), 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, dyTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "dyTexture"), 3);
    
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, dtTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "dtTexture"), 4);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, current);
        glUniform1i(glGetUniformLocation(shaderProgram, "current"), 5);
    }
};

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
    float derivativeTextureSize = 300;

    GLuint shaderProgramQuad = createShaderProgram("shaders/basicVS.glsl", "shaders/quadFS.glsl");
    GLuint shaderProgramLowpass = createShaderProgram("shaders/basicVS.glsl", "shaders/blurFS.glsl");

    GLuint shaderDerivativeX = createShaderProgram("shaders/basicVS.glsl", "shaders/derivativeXFS.glsl");
    GLuint shaderDerivativeY = createShaderProgram("shaders/basicVS.glsl", "shaders/derivativeYFS.glsl");
    GLuint shaderDerivativeT = createShaderProgram("shaders/basicVS.glsl", "shaders/derivativeTFS.glsl");
    GLuint shaderLucasKanade = createShaderProgram("shaders/basicVS.glsl", "shaders/lucasKanadeFS.glsl");

    TextureOperation programDerivativeX = TextureOperation(derivativeTextureSize, derivativeTextureSize, shaderDerivativeX);
    TextureOperation programDerivativeY = TextureOperation(derivativeTextureSize, derivativeTextureSize, shaderDerivativeY);
    TextureOperation programDerivativeT = TextureOperation(derivativeTextureSize, derivativeTextureSize, shaderDerivativeT);
    LucasKanadeProgram programLucasKanade = LucasKanadeProgram(derivativeTextureSize, derivativeTextureSize, shaderLucasKanade);
    PingPongTextureOperation blur10 = PingPongTextureOperation(derivativeTextureSize, derivativeTextureSize, shaderProgramLowpass, 2);
    PingPongTextureOperation blur11 = PingPongTextureOperation(derivativeTextureSize, derivativeTextureSize, shaderProgramLowpass, 2);
    PingPongTextureOperation blur12 = PingPongTextureOperation(derivativeTextureSize, derivativeTextureSize, shaderProgramLowpass, 2);

    GLuint textureId1 = loadPNGTexture("images/A.png");
    GLuint textureId2 = loadPNGTexture("images/B.png");
    programDerivativeX.execute(textureId2, 0);
    programDerivativeY.execute(textureId2, 0);
    programDerivativeT.execute(textureId1, textureId2);

    blur10.execute(programDerivativeX.getTextureResult(), 0);
    blur11.execute(programDerivativeY.getTextureResult(), 0);
    blur12.execute(programDerivativeT.getTextureResult(), 0);

    programLucasKanade.current = textureId2;
    programLucasKanade.dxTexture = programDerivativeX.getTextureResult();
    programLucasKanade.dyTexture = programDerivativeY.getTextureResult();
    programLucasKanade.dtTexture = programDerivativeT.getTextureResult();
    programLucasKanade.execute(0, 0);

    glm::mat4 orthoUI = glm::ortho(-(float)WINDOW_WIDTH/2.0f,(float)WINDOW_WIDTH/2.0f,-(float)WINDOW_HEIGHT/2.0f,(float)WINDOW_HEIGHT/2.0f,-20.0f,20.0f);
    glm::mat4 unitMatrix = glm::mat4(1.0f);

    Camera camera;

    float quadSize = 300;
    Quad quad;
    quad.scale = glm::vec3(quadSize, quadSize, 1);
    quad.position = glm::vec3(WINDOW_WIDTH / 2.0f - quadSize / 2.0f - 20, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f + 20, 0);
    Quad quadLL;
    quadLL.scale = glm::vec3(quadSize, quadSize, 1);
    quadLL.position = glm::vec3(-WINDOW_WIDTH / 2.0f + quadSize / 2.0f + 10, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f + 10, 0);
    Quad quadCC;
    quadCC.scale = glm::vec3(quadSize, quadSize, 1);
    quadCC.position = glm::vec3(-WINDOW_WIDTH / 2.0f + quadSize / 2.0f + quadSize, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f + 10, 0);
    Quad quadC2;
    quadC2.scale = glm::vec3(quadSize, quadSize, 1);
    quadC2.position = glm::vec3(-WINDOW_WIDTH / 2.0f + quadSize / 2.0f + 2*quadSize, -WINDOW_HEIGHT / 2.0f + quadSize / 2.0f + 10, 0);

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

        // Render helper quads
        quadLL.textureId = blur10.getTextureResult();;
        quadLL.render(shaderProgramQuad, unitMatrix, orthoUI);

        quadCC.textureId = blur11.getTextureResult();
        quadCC.render(shaderProgramQuad, unitMatrix, orthoUI);

        quadC2.textureId = blur12.getTextureResult();
        quadC2.render(shaderProgramQuad, unitMatrix, orthoUI);

        quad.textureId = programLucasKanade.getTextureResult();
        quad.render(shaderProgramQuad, unitMatrix, orthoUI);

        i++;
        keysPressed.clear();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return 0;
}