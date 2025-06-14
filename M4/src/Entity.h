#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Entity
{
public:
    Entity(float x, float y, float z, glm::vec3 baseColor, float initialScale,
           const std::string &objFilePath, const std::string &mtlFilePath, const std::string &textureFilePath,
           glm::vec3 initialRotation = glm::vec3(0.0f, 0.0f, 0.0f));

    void initialize();
    void draw();

    void toggleRotateX();
    void toggleRotateY();
    void toggleRotateZ();

    void scaleUp();
    void scaleDown();

    void moveRight();
    void moveLeft();
    void moveUp();
    void moveDown();
    void moveForward();
    void moveBackward();

private:
    glm::vec3 position;
    glm::vec3 baseColor;
    glm::vec3 initialRotation;

    float scaleFactor;

    bool rotateX, rotateY, rotateZ;

    GLuint VAO;
    GLuint textureID;
    int nVertices;
    GLuint shaderProgram;

    std::string objFilePath;
    std::string mtlFilePath;
    std::string textureFilePath;

    float ka;         
    float kd;        
    float ks;         
    float shininess;

    int loadModelWithTexture(const std::string &objFilePath,
                             const std::string &mtlFilePath,
                             const std::string &textureFilePath,
                             int &outVertices,
                             GLuint &outTextureID);
                             
    void loadMaterial(const std::string &mtlFilePath);

    GLuint loadTexture(const std::string &texturePath);

    void setupShaders();
    void checkCompileErrors(GLuint shader, std::string type);
    

    static constexpr float TRANSLATION_SPEED = 0.1f;
};

#endif
