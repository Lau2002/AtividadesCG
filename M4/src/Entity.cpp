#include "Entity.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Entity::Entity(float x, float y, float z,
               glm::vec3 baseColor,
               float initialScale,
               const std::string &objFilePath,
               const std::string &mtlFilePath,
               const std::string &textureFilePath,
               glm::vec3 initialRotation)
    : position(x, y, z), baseColor(baseColor), scaleFactor(initialScale),
      rotateX(false), rotateY(false), rotateZ(false),
      VAO(0), textureID(0), nVertices(0), shaderProgram(0),
      objFilePath(objFilePath), mtlFilePath(mtlFilePath), textureFilePath(textureFilePath),
      initialRotation(initialRotation),
      ka(0.1f), kd(0.5f), ks(0.5f), shininess(10.0f)
{
}

void Entity::initialize()
{
    loadMaterial(mtlFilePath);

    VAO = loadModelWithTexture(objFilePath, mtlFilePath, textureFilePath, nVertices, textureID);
    if (VAO == -1)
    {
        std::cerr << "Failed to load model from " << objFilePath << std::endl;
    }
    setupShaders();
}


int Entity::loadModelWithTexture(const std::string &objFilePath,
                                 const std::string &mtlFilePath,
                                 const std::string &textureFilePath,
                                 int &outVertices,
                                 GLuint &outTextureID)
{
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;

    std::ifstream file(objFilePath);
    if (!file.is_open())
    {
        std::cerr << "Error opening OBJ file: " << objFilePath << std::endl;
        return -1;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        }
        else if (prefix == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (prefix == "vn") {
            glm::vec3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            temp_normals.push_back(norm);
        }
        else if (prefix == "f") {
            for (int i = 0; i < 3; ++i) {
                std::string vertexStr;
                ss >> vertexStr;
                std::replace(vertexStr.begin(), vertexStr.end(), '/', ' ');
                std::istringstream viss(vertexStr);
                unsigned int v, vt, vn;
                viss >> v >> vt >> vn;
                vertexIndices.push_back(v - 1);
                uvIndices.push_back(vt - 1);
                normalIndices.push_back(vn - 1);
            }
        }
    }
    file.close();

    std::vector<GLfloat> vertexData;
    for (size_t i = 0; i < vertexIndices.size(); ++i) {
        glm::vec3 pos = temp_positions[vertexIndices[i]];
        glm::vec2 uv = temp_uvs[uvIndices[i]];
        glm::vec3 norm = temp_normals[normalIndices[i]];

        vertexData.insert(vertexData.end(), {pos.x, pos.y, pos.z, uv.x, uv.y, norm.x, norm.y, norm.z});
    }

    if (!textureFilePath.empty()) {
        outTextureID = loadTexture(textureFilePath);
        if (outTextureID == 0) {
            std::cerr << "Failed to load texture: " << textureFilePath << std::endl;
        }
    } else {
        outTextureID = 0;
    }

    GLuint VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    outVertices = (int)(vertexIndices.size());
    return VAO;
}

void Entity::loadMaterial(const std::string& mtlFilePath) {
    std::ifstream file(mtlFilePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open MTL file: " << mtlFilePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "Ka") {
            float r, g, b;
            ss >> r >> g >> b;
            ka = (r + g + b) / 3.0f;
        }
        else if (prefix == "Kd") {
            float r, g, b;
            ss >> r >> g >> b;
            kd = (r + g + b) / 3.0f;
        }
        else if (prefix == "Ks") {
            float r, g, b;
            ss >> r >> g >> b;
            ks = (r + g + b) / 3.0f;
        }
        else if (prefix == "Ns") {
            ss >> shininess;
        }
    }

    file.close();
}


GLuint Entity::loadTexture(const std::string &texturePath)
{
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cerr << "Failed to load texture image: " << texturePath << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);

    GLenum format = GL_RGB;
    if (nrChannels == 1) format = GL_RED;
    else if (nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

void Entity::setupShaders()
{
    const GLchar *vertexShaderSource = R"glsl(
        #version 410 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;
        layout(location = 2) in vec3 normal;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        out vec2 TexCoord;
        out vec3 FragPos;
        out vec3 Normal;

        void main() {
            FragPos = vec3(model * vec4(position, 1.0));
            Normal = mat3(transpose(inverse(model))) * normal;
            TexCoord = texCoord;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )glsl";

    const GLchar *fragmentShaderSource = R"glsl(
        #version 410 core
        in vec2 TexCoord;
        in vec3 FragPos;
        in vec3 Normal;

        out vec4 FragColor;

        uniform sampler2D texture1;
        uniform vec3 lightPos;
        uniform vec3 camPos;
        uniform float ka;
        uniform float kd;
        uniform float ks;
        uniform float q;

        void main() {
            vec3 color = texture(texture1, TexCoord).rgb;
            vec3 norm = normalize(Normal);
            vec3 lightColor = vec3(1.0);
            vec3 ambient = ka * lightColor;
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = kd * diff * lightColor;
            vec3 viewDir = normalize(camPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), q);
            vec3 specular = ks * spec * lightColor;
            vec3 result = (ambient + diffuse) * color + specular;
            FragColor = vec4(result, 1.0);
        }
    )glsl";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Entity::checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "| ERROR::SHADER-COMPILATION-ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "| ERROR::PROGRAM-LINKING-ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

void Entity::draw()
{
    glUseProgram(shaderProgram);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(scaleFactor));
    model = glm::rotate(model, initialRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, initialRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, initialRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    float angle = static_cast<float>(glfwGetTime());
    if (rotateX) model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotateY) model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotateZ) model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 1.0f, 1.2f, -0.5f);
    glUniform3f(glGetUniformLocation(shaderProgram, "camPos"), 0.0f, 0.0f, 3.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "ka"), ka);
    glUniform1f(glGetUniformLocation(shaderProgram, "kd"), kd);
    glUniform1f(glGetUniformLocation(shaderProgram, "ks"), ks);
    glUniform1f(glGetUniformLocation(shaderProgram, "q"), shininess);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, nVertices);
    glBindVertexArray(0);
}

void Entity::toggleRotateX()
{
    rotateX = !rotateX;
    rotateY = false;
    rotateZ = false;
}

void Entity::toggleRotateY()
{
    rotateX = false;
    rotateY = !rotateY;
    rotateZ = false;
}

void Entity::toggleRotateZ()
{
    rotateX = false;
    rotateY = false;
    rotateZ = !rotateZ;
}

void Entity::scaleUp()
{
    scaleFactor = std::min(scaleFactor + 0.1f, 0.8f);
}

void Entity::scaleDown()
{
    scaleFactor = std::max(scaleFactor - 0.1f, 0.1f);
}

void Entity::moveRight()
{
    position.x += TRANSLATION_SPEED;
}

void Entity::moveLeft()
{
    position.x -= TRANSLATION_SPEED;
}

void Entity::moveUp()
{
    position.y += TRANSLATION_SPEED;
}

void Entity::moveDown()
{
    position.y -= TRANSLATION_SPEED;
}

void Entity::moveForward()
{
    position.z -= TRANSLATION_SPEED;
}

void Entity::moveBackward()
{
    position.z += TRANSLATION_SPEED;
}
