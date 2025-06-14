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
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Desafio Módulo 4 - Laura Skorupski", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    entities.emplace_back(
        0.0f, 0.0f, 0.0f,
        glm::vec3(0.0f, 0.0f, 1.0f),
        0.3f,
        "../assets/Modelos3D/Suzanne.obj",
        "../assets/Modelos3D/Suzanne.mtl",
        "../assets/tex/Suzanne.png",
        glm::vec3(0.0f, glm::radians(0.0f), 0.0f));

    for (auto &entity : entities)
    {
        entity.initialize();
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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

        if (key == GLFW_KEY_X)
        {
            selectedEntity.toggleRotateX();
        }
        if (key == GLFW_KEY_Y)
        {
            selectedEntity.toggleRotateY();
        }
        if (key == GLFW_KEY_Z)
        {
            selectedEntity.toggleRotateZ();
        }
        if (key == GLFW_KEY_I)
        {
            selectedEntity.scaleUp();
        }
        if (key == GLFW_KEY_O)
        {
            selectedEntity.scaleDown();
        }
        if (key == GLFW_KEY_W)
        {
            selectedEntity.moveUp();
        }
        if (key == GLFW_KEY_S)
        {
            selectedEntity.moveDown();
        }
        if (key == GLFW_KEY_A)
        {
            selectedEntity.moveLeft();
        }
        if (key == GLFW_KEY_D)
        {
            selectedEntity.moveRight();
        }
        if (key == GLFW_KEY_U)
        {
            selectedEntity.moveForward();
        }
        if (key == GLFW_KEY_J)
        {
            selectedEntity.moveBackward();
        }
        if (key == GLFW_KEY_C)
        {
            selectedEntityIndex = (selectedEntityIndex + 1) % entities.size();
        }
    }
}
