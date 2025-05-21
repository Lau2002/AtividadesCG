#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
const GLuint WIDTH = 1000, HEIGHT = 1000;
int selectedEntityIndex = 0;

class Entity {
public:
    Entity(float x, float y, float z, glm::vec3 baseColor, float initialScale, const std::string& objFilePath)
        : position(x, y, z), baseColor(baseColor), scaleFactor(initialScale), rotateX(false), rotateY(false), rotateZ(false) {
        VAO = loadColoredCube(objFilePath, nVertices);
        if (VAO == -1) {
            std::cerr << "Failed to load model from " << objFilePath << std::endl;
        }
        setupShaders();
    }

    void draw() {
        glUseProgram(shaderProgram);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(scaleFactor));

        float angle = static_cast<GLfloat>(glfwGetTime());
        if (rotateX) {
            model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
        }
        if (rotateY) {
            model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        if (rotateZ) {
            model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        }

        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, nVertices);
        glBindVertexArray(0);
    }

    const float TRANSLATION_SPEED = 0.1f;

    void toggleRotateX() { rotateX = !rotateX; rotateY = false; rotateZ = false; }
    void toggleRotateY() { rotateX = false; rotateY = !rotateY; rotateZ = false; }
    void toggleRotateZ() { rotateX = false; rotateY = false; rotateZ = !rotateZ; }
    void scaleUp() { scaleFactor = std::min(scaleFactor + 0.1f, 0.8f); }
    void scaleDown() { scaleFactor = std::max(0.1f, scaleFactor - 0.1f); }
    void moveRight() { position.x += TRANSLATION_SPEED; }
    void moveLeft() { position.x -= TRANSLATION_SPEED; }
    void moveUp() { position.y += TRANSLATION_SPEED; }
    void moveDown() { position.y -= TRANSLATION_SPEED; }
    void moveForward() { position.z -= TRANSLATION_SPEED; }
    void moveBackward() { position.z += TRANSLATION_SPEED; }

private:
    GLuint VAO;
    int nVertices;
    glm::vec3 position;
    glm::vec3 baseColor;
    float scaleFactor;
    bool rotateX, rotateY, rotateZ;
    GLuint shaderProgram;

    glm::vec3 getColorByNormal(const glm::vec3& normal) {
        glm::vec3 norm = glm::normalize(normal);
        
        if (norm.x > 0.9f)        return glm::vec3(1.0f, 0.0f, 0.0f);
        else if (norm.x < -0.9f)  return glm::vec3(0.0f, 1.0f, 0.0f);
        else if (norm.y > 0.9f)   return glm::vec3(0.0f, 0.0f, 1.0f);
        else if (norm.y < -0.9f)  return glm::vec3(1.0f, 1.0f, 0.0f);
        else if (norm.z > 0.9f)   return glm::vec3(1.0f, 0.0f, 1.0f);
        else if (norm.z < -0.9f)  return glm::vec3(0.0f, 1.0f, 1.0f);
        
        return glm::vec3(0.5f, 0.5f, 0.5f);
    }

    int loadColoredCube(const std::string& filePath, int &nVertices) {
        std::vector<glm::vec3> vertices;
        std::vector<GLfloat> vertexData;

        std::ifstream arqEntrada(filePath.c_str());
        if (!arqEntrada.is_open()) {
            std::cerr << "Erro ao tentar ler o arquivo " << filePath << std::endl;
            return -1;
        }

        std::string line;
        while (std::getline(arqEntrada, line)) {
            std::istringstream ssline(line);
            std::string word;
            ssline >> word;

            if (word == "v") {
                glm::vec3 vertice;
                ssline >> vertice.x >> vertice.y >> vertice.z;
                vertices.push_back(vertice);
            }
        }

        arqEntrada.clear();
        arqEntrada.seekg(0, std::ios::beg);

        const glm::vec3 faceColors[] = {
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 1.0f, 1.0f)
        };

        std::map<int, int> faceIndices;
        int currentFaceColor = 0;

        while (std::getline(arqEntrada, line)) {
            std::istringstream ssline(line);
            std::string word;
            ssline >> word;

            if (word == "f") {
                std::vector<int> faceVerts;
                while (ssline >> word) {
                    int vi = 0;
                    std::istringstream ss(word);
                    std::string index;

                    if (std::getline(ss, index, '/')) vi = !index.empty() ? std::stoi(index) - 1 : 0;

                    faceVerts.push_back(vi);
                }

                glm::vec3 v1 = vertices[faceVerts[0]];
                glm::vec3 v2 = vertices[faceVerts[1]];
                glm::vec3 v3 = vertices[faceVerts[2]];
                
                glm::vec3 edge1 = v2 - v1;
                glm::vec3 edge2 = v3 - v1;
                glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
                
                glm::vec3 faceColor;
                
                if (glm::abs(normal.x) > glm::abs(normal.y) && glm::abs(normal.x) > glm::abs(normal.z)) {
                    faceColor = (normal.x > 0) ? faceColors[0] : faceColors[1];
                } else if (glm::abs(normal.y) > glm::abs(normal.x) && glm::abs(normal.y) > glm::abs(normal.z)) {
                    faceColor = (normal.y > 0) ? faceColors[2] : faceColors[3];
                } else {
                    faceColor = (normal.z > 0) ? faceColors[4] : faceColors[5];
                }

                for (int i = 0; i < faceVerts.size() - 2; i++) {
                    vertexData.push_back(vertices[faceVerts[0]].x);
                    vertexData.push_back(vertices[faceVerts[0]].y);
                    vertexData.push_back(vertices[faceVerts[0]].z);
                    vertexData.push_back(faceColor.r);
                    vertexData.push_back(faceColor.g);
                    vertexData.push_back(faceColor.b);

                    vertexData.push_back(vertices[faceVerts[i+1]].x);
                    vertexData.push_back(vertices[faceVerts[i+1]].y);
                    vertexData.push_back(vertices[faceVerts[i+1]].z);
                    vertexData.push_back(faceColor.r);
                    vertexData.push_back(faceColor.g);
                    vertexData.push_back(faceColor.b);

                    vertexData.push_back(vertices[faceVerts[i+2]].x);
                    vertexData.push_back(vertices[faceVerts[i+2]].y);
                    vertexData.push_back(vertices[faceVerts[i+2]].z);
                    vertexData.push_back(faceColor.r);
                    vertexData.push_back(faceColor.g);
                    vertexData.push_back(faceColor.b);
                }
            }
        }

        arqEntrada.close();

        GLuint VBO, VAO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        nVertices = vertexData.size() / 6;
        return VAO;
    }

    void setupShaders() {
        const GLchar* vertexShaderSource = "#version 410\n"
            "layout (location = 0) in vec3 position;\n"
            "layout (location = 1) in vec3 vertexColor;\n"
            "uniform mat4 model;\n"
            "out vec3 fragmentColor;\n"
            "void main()\n"
            "{\n"
            "gl_Position = model * vec4(position, 1.0);\n"
            "fragmentColor = vertexColor;\n"
            "}\0";

        const GLchar* fragmentShaderSource = "#version 410\n"
            "in vec3 fragmentColor;\n"
            "out vec4 color;\n"
            "void main()\n"
            "{\n"
            "color = vec4(fragmentColor, 1.0);\n"
            "}\n\0";

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
};

std::vector<Entity> entities;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Desafio Módulo 2 - Laura Skorupski", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    entities.emplace_back(-0.5f, 0.0f, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f), 0.3f, "../assets/Modelos3D/Cube.obj");
    entities.emplace_back(0.5f, 0.0f, 0.5f, glm::vec3(0.0f, 0.0f, 1.0f), 0.3f, "../assets/Modelos3D/Cube.obj");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto& entity : entities) {
            entity.draw();
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        Entity& selectedEntity = entities[selectedEntityIndex];

        if (key == GLFW_KEY_X) {
            selectedEntity.toggleRotateX();
        }
        if (key == GLFW_KEY_Y) {
            selectedEntity.toggleRotateY();
        }
        if (key == GLFW_KEY_Z) {
            selectedEntity.toggleRotateZ();
        }
        if (key == GLFW_KEY_I) {
            selectedEntity.scaleUp();
        }
        if (key == GLFW_KEY_O) {
            selectedEntity.scaleDown();
        }
        if (key == GLFW_KEY_W) {
            selectedEntity.moveUp();
        }
        if (key == GLFW_KEY_S) {
            selectedEntity.moveDown();
        }
        if (key == GLFW_KEY_A) {
            selectedEntity.moveLeft();
        }
        if (key == GLFW_KEY_D) {
            selectedEntity.moveRight();
        }
        if (key == GLFW_KEY_U) {
            selectedEntity.moveForward();
        }
        if (key == GLFW_KEY_J) {
            selectedEntity.moveBackward();
        }
        if (key == GLFW_KEY_C) {
            selectedEntityIndex = (selectedEntityIndex + 1) % entities.size();
        }
    }
}
