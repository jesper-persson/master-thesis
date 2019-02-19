#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

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

using namespace std;

const int WINDOW_HEIGHT = 800;
const int WINDOW_WIDTH = 1200;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

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

    float vertices[9] = {0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, -1.0f, 0.0f};

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);

    GLuint shaderProgram = createShaderProgram("shaders/basicVS.glsl", "shaders/basicFS.glsl");
    glUseProgram(shaderProgram);

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(10, 10, 1));
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 modelToWorld = translate * scale;
    // glm::mat4 worldToCamera = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0, 0)));
    glm::mat4 perspective = glm::perspectiveFov(1.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 0.01f, 1000.0f);

    glm::vec3 cameraPosition = glm::vec3(10.0f, 0, 10.0f);
    glm::vec3 cameraUp = glm::vec3(0, 1.0f, 0);
    glm::vec3 cameraForward = glm::normalize(glm::vec3(-1.0f, 0, -1.0f));
    glm::vec3 cameraLookAtPosition = cameraPosition + cameraForward * 10.0f;

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.5, 0.5, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 worldToCamera = glm::lookAt(cameraPosition, cameraLookAtPosition, cameraUp);

        glBindBuffer(GL_ARRAY_BUFFER, vao);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelToWorld"), 1, GL_FALSE, glm::value_ptr(modelToWorld));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "worldToCamera"), 1, GL_FALSE, glm::value_ptr(worldToCamera));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(perspective));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return 0;
}