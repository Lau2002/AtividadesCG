#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Entity.h"

const GLuint WIDTH = 1000, HEIGHT = 1000;
int selectedEntityIndex = 0;
std::vector<Entity> entities;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Desafio Módulo 3 - Laura Skorupski", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Cria entidades
   entities.emplace_back(
        -0.5f, 0.0f, 0.0f,
        glm::vec3(1.0f, 0.0f, 0.0f),
        0.3f,
        "../assets/Modelos3D/Cube.obj",
        "../assets/Modelos3D/Cube.mtl",
        "../assets/tex/pixelWall.png",
        glm::vec3(0.0f, glm::radians(180.0f), 0.0f)  
   );

    entities.emplace_back(
        0.5f, 0.0f, 0.5f,
        glm::vec3(0.0f, 0.0f, 1.0f),
        0.3f,
        "../assets/Modelos3D/Suzanne.obj",
        "../assets/Modelos3D/Suzanne.mtl",
        "../assets/tex/Suzanne.png",
        glm::vec3(0.0f, glm::radians(180.0f), 0.0f) 
    );


    for (auto& entity : entities) {
        entity.initialize();
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto &entity : entities)
        {
            entity.draw();
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        Entity &selectedEntity = entities[selectedEntityIndex];

        switch (key)
        {
        case GLFW_KEY_X:
            selectedEntity.toggleRotateX();
            break;
        case GLFW_KEY_Y:
            selectedEntity.toggleRotateY();
            break;
        case GLFW_KEY_Z:
            selectedEntity.toggleRotateZ();
            break;
        case GLFW_KEY_I:
            selectedEntity.scaleUp();
            break;
        case GLFW_KEY_O:
            selectedEntity.scaleDown();
            break;
        case GLFW_KEY_W:
            selectedEntity.moveUp();
            break;
        case GLFW_KEY_S:
            selectedEntity.moveDown();
            break;
        case GLFW_KEY_A:
            selectedEntity.moveLeft();
            break;
        case GLFW_KEY_D:
            selectedEntity.moveRight();
            break;
        case GLFW_KEY_U:
            selectedEntity.moveForward();
            break;
        case GLFW_KEY_J:
            selectedEntity.moveBackward();
            break;
        case GLFW_KEY_C:
            selectedEntityIndex = (selectedEntityIndex + 1) % entities.size();
            break;
        default:
            break;
        }
    }
}
