class Footstep {
public:
    Model shoes;
    float totalTime;
    float offset;

    Footstep() {
        offset = 0;
        totalTime = 0;
        shoes = loadUsingTinyObjLoader("resources/shoe.obj");
        shoes.textureId = loadPNGTexture("resources/gray.png");
        shoes.scale = glm::vec3(0.006f, 0.006f, 0.006f);
        shoes.position = glm::vec3(-48.0f, 10.5f, 20.0f);
        shoes.forward = glm::rotate(shoes.forward, -1.67f, shoes.up);
    }

    void render(GLuint shaderProgram, glm::mat4 worldToCamera, glm::mat4 projection) {
        shoes.render(shaderProgram, worldToCamera, projection);
    }

    void update(float dt) {
        totalTime += dt;
        float speed = 4.6f * dt;
        shoes.position.x += speed * pow(sin(totalTime), 4);
        shoes.position.y += speed * (pow(sin(totalTime + 3.1416f/4.0f), 2) - 0.5f);
    }
};