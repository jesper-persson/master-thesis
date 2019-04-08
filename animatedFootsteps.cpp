class Footstep {
public:
    Box box;
    float totalTime;
    float offset;

    Footstep() {
        offset = 0;
        totalTime = 0;
        box = loadUsingTinyObjLoader("shoe.obj");
        box.textureId = loadPNGTexture("images/gray.png");
        box.scale = glm::vec3(0.006f, 0.006f, 0.006f);
        box.position = glm::vec3(-48.0f, 4.5f, 20.0f);
        box.forward = glm::rotate(box.forward, -1.67f, box.up);
    }

    void render(GLuint shaderProgram, glm::mat4 worldToCamera, glm::mat4 projection) {
        box.render(shaderProgram, worldToCamera, projection);
    }

    void update(float dt) {
        totalTime += dt;
        float speed = 4.6f * dt;
        box.position.x += speed * pow(sin(totalTime), 4);
        box.position.y += speed * (pow(sin(totalTime + 3.1416f/4.0f), 2) - 0.5f);
    }
};