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

// stb_image para carregar texturas
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Construtor
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
      initialRotation(initialRotation)
{
}


// Inicializa o modelo: carrega .obj, .mtl, textura, shaders
void Entity::initialize()
{
    VAO = loadModelWithTexture(objFilePath, mtlFilePath, textureFilePath, nVertices, textureID);
    if (VAO == -1)
    {
        std::cerr << "Failed to load model from " << objFilePath << std::endl;
    }
    setupShaders();
}

// Função para carregar modelo + textura (obj + mtl + png)
int Entity::loadModelWithTexture(const std::string &objFilePath,
                                 const std::string &mtlFilePath,
                                 const std::string &textureFilePath,
                                 int &outVertices,
                                 GLuint &outTextureID)
{
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_uvs;
    std::vector<unsigned int> vertexIndices, uvIndices;

    // Passo 1: carregar .obj (mesmo que antes)
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

        if (prefix == "v")
        {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        }
        else if (prefix == "vt")
        {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (prefix == "f")
        {
            for (int i = 0; i < 3; ++i)
            {
                std::string vertexStr;
                ss >> vertexStr;

                std::replace(vertexStr.begin(), vertexStr.end(), '/', ' ');

                std::istringstream viss(vertexStr);
                unsigned int vIndex = 0, uvIndex = 0;
                viss >> vIndex;
                viss >> uvIndex;

                if (vIndex > 0)
                    vertexIndices.push_back(vIndex - 1);
                if (uvIndex > 0)
                    uvIndices.push_back(uvIndex - 1);
            }
        }
    }
    file.close();

    if (vertexIndices.size() != uvIndices.size())
    {
        std::cerr << "Warning: number of vertex indices != number of uv indices" << std::endl;
    }

    std::vector<GLfloat> vertexData;
    for (size_t i = 0; i < vertexIndices.size(); i++)
    {
        glm::vec3 pos = temp_positions[vertexIndices[i]];
        vertexData.push_back(pos.x);
        vertexData.push_back(pos.y);
        vertexData.push_back(pos.z);

        if (i < uvIndices.size())
        {
            glm::vec2 uv = temp_uvs[uvIndices[i]];
            vertexData.push_back(uv.x);
            vertexData.push_back(uv.y);
        }
        else
        {
            vertexData.push_back(0.0f);
            vertexData.push_back(0.0f);
        }
    }

    // Passo 2: carregar textura usando textureFilePath recebido como parâmetro
    if (!textureFilePath.empty())
    {
        outTextureID = loadTexture(textureFilePath);
        if (outTextureID == 0)
        {
            std::cerr << "Failed to load texture: " << textureFilePath << std::endl;
        }
    }
    else
    {
        std::cerr << "No texture file path provided." << std::endl;
        outTextureID = 0;
    }

    // Passo 3: criar VAO e VBO
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    outVertices = (int)(vertexIndices.size());

    return VAO;
}

// Função para ler o .mtl e extrair nome da textura (map_Kd)
std::string Entity::parseMTLForTexture(const std::string &mtlFilePath)
{
    std::ifstream file(mtlFilePath);
    if (!file.is_open())
    {
        std::cerr << "Error opening MTL file: " << mtlFilePath << std::endl;
        return "";
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.substr(0, 7) == "map_Kd ")
        {
            std::string textureFile = line.substr(7);
            file.close();
            return textureFile;
        }
    }
    file.close();
    return "";
}

// Função que carrega a textura usando stb_image
GLuint Entity::loadTexture(const std::string &texturePath)
{
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // inverte verticalmente para OpenGL
    unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cerr << "Failed to load texture image: " << texturePath << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);

    GLenum format = GL_RGB;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // configurações de filtro e wrap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

// Configura shaders para usar textura
void Entity::setupShaders()
{
    const GLchar *vertexShaderSource = R"glsl(
        #version 410 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;

        uniform mat4 model;

        out vec2 TexCoord;

        void main() {
            gl_Position = model * vec4(position, 1.0);
            TexCoord = texCoord;
        }
    )glsl";

    const GLchar *fragmentShaderSource = R"glsl(
        #version 410 core
        in vec2 TexCoord;
        out vec4 FragColor;

        uniform sampler2D texture1;

        void main() {
            FragColor = texture(texture1, TexCoord);
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

// Função para checar erros de compilação e linkagem
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

// Método draw atualizado para bindar a textura e passar uniform sampler2D
void Entity::draw()
{
    glUseProgram(shaderProgram);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(scaleFactor));

    // Aplicar rotação inicial (em radianos)
    model = glm::rotate(model, initialRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, initialRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, initialRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    // Aplicar rotação animada (quando ativada)
    float angle = static_cast<float>(glfwGetTime());
    if (rotateX)
        model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotateY)
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotateZ)
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLint texLoc = glGetUniformLocation(shaderProgram, "texture1");
    glUniform1i(texLoc, 0);

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
