class Footstep {
public:
    Box box;
    float totalTime;
    int state;
    float lengthTravelled;
    Footstep() {
        state = 0;
        lengthTravelled = 0;
    }

    void render(GLuint shaderProgram, glm::mat4 worldToCamera, glm::mat4 projection) {
        box.render(shaderProgram, worldToCamera, projection);
    }
    
    void update(float dt) {
        totalTime += dt;
        
        if (state == 0) {
            box.position.y = 10 + sin(totalTime);
            if (box.position.y < 9.1) {
                state = 1;
            }
        }

        if (state == 1) {
            box.position.y = 10 + sin(totalTime);
            if (box.position.y > 10.9) {
                state = 2;
                lengthTravelled = 0;
            }
        }
         
        if (state == 2) {
            box.position.x += dt;
            lengthTravelled += dt;
            if (lengthTravelled > 3) {
                lengthTravelled = 0;
                state = 0;
            }
        }
        
    }
};