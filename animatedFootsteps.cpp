class Footstep {
public:
    Model shoes;
    float totalTime;
    float offset;
    bool leftFoot;

    Footstep(bool leftFoot) {
        offset = 0;
        totalTime = 0;
        shoes = loadUsingTinyObjLoader("resources/shoe.obj");
        float y = 6.50f;
        shoes.position = glm::vec3(-45.1f + 30.0f,y, 20.3f - 10.0f);
        this->leftFoot = leftFoot;
        if (leftFoot) {
            shoes.position = glm::vec3(-48.0f + 30.0f,y, 20.0f- 10.0f);
            shoes.startIndex = shoes.numIndices / 2;
        }
        shoes.numIndices = shoes.numIndices / 2;
        shoes.textureId = loadPNGTexture("resources/gray.png");
        shoes.scale = glm::vec3(0.006f, 0.006f, 0.006f);
        shoes.forward = glm::rotate(shoes.forward, -1.67f, shoes.up);
    }

    void render(GLuint shaderProgram, glm::mat4 worldToCamera, glm::mat4 projection) {
        shoes.render(shaderProgram, worldToCamera, projection);
    }

    void update(float dt) {
        totalTime += dt;

        // Update y position
        float height = 1.5f;
        if (this->leftFoot) {
            shoes.position.y = height * pow(cos(totalTime), 10) + 4.3f;
        } else {
            shoes.position.y = height * pow(sin(totalTime), 10) + 4.3f;
        }

        // The x position is given by the integral of the y position function
        float speed = 5.0f;
        auto integral = [this, &speed](float x) ->float {
            if (this->leftFoot) {
                return speed *(2520 *x + 2100 *sin(2 *x) + 600 *sin(4 *x) + 150 *sin(6 *x) + 25 *sin(8 *x) + 2 *sin(10 *x))/10240;
            } else {
                return speed * (2520 * x - 2100* sin(2 * x) + 600* sin(4* x) - 150 *sin(6* x) + 25* sin(8* x) - 2 *sin(10* x))/10240.0f;
            }
        };
        shoes.position.x = integral(totalTime);
    }
};