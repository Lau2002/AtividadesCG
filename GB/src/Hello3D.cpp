#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Entity.h"
#include "Camera.h"

const GLuint WIDTH = 1000, HEIGHT = 1000;
int selectedEntityIndex = 0;
std::vector<Entity> entities;

Camera camera;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Leitor e Visualizador de Cenas - Laura Skorupski", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    entities.emplace_back(
        0.0f, -1.5f, 0.0f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.9f,
        "../assets/Modelos3D/patch-grass.obj",
        "../assets/Modelos3D/patch-grass.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(00.0f), 0.0f));

    entities.emplace_back(
        -1.2f, -1.3f, 1.0f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.23f,
        "../assets/Modelos3D/tower-complete-large.obj",
        "../assets/Modelos3D/tower-complete-large.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(125.0f), 0.0f));

    entities.emplace_back(
        -1.0f, -1.3f, 0.05f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.3f,
        "../assets/Modelos3D/platform-planks.obj",
        "../assets/Modelos3D/platform-planks.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(-50.0f), 0.0f));

    entities.emplace_back(
        -0.8f, -1.3f, -0.9f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.23f,
        "../assets/Modelos3D/tower-complete-small.obj",
        "../assets/Modelos3D/tower-complete-small.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(35.0f), 0.0f));

    entities.emplace_back(
        -0.3f, -2.5f, -2.5f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.3f,
        "../assets/Modelos3D/ship-pirate-medium.obj",
        "../assets/Modelos3D/ship-pirate-medium.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(60.0f), 0.0f));

    entities.emplace_back(
        1.5f, -1.5f, 0.5f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.3f,
        "../assets/Modelos3D/palm-detailed-bend.obj",
        "../assets/Modelos3D/palm-detailed-bend.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(-55.0f), 0.0f));

    entities.emplace_back(
        1.0f, -1.3f, 0.5f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.3f,
        "../assets/Modelos3D/chest.obj",
        "../assets/Modelos3D/chest.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(195.0f), 0.0f));

    entities.emplace_back(
        0.0f, -1.3f, 1.0f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.3f,
        "../assets/Modelos3D/structure-roof.obj",
        "../assets/Modelos3D/structure-roof.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(200.0f), 0.0f));

    entities.emplace_back(
        0.0f, -1.3f, 1.0f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.3f,
        "../assets/Modelos3D/crate-bottles.obj",
        "../assets/Modelos3D/crate-bottles.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(200.0f), 0.0f));

    entities.emplace_back(
        0.3f, -1.3f, -1.1f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.3f,
        "../assets/Modelos3D/cannon-mobile.obj",
        "../assets/Modelos3D/cannon-mobile.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(180.0f), 0.0f));

    entities.emplace_back(
        1.3f, -2.0f, -1.0f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.3f,
        "../assets/Modelos3D/structure-platform.obj",
        "../assets/Modelos3D/structure-platform.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(50.0f), 0.0f));

    entities.emplace_back(
        1.8f, -2.0f, -1.55f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.3f,
        "../assets/Modelos3D/structure-platform.obj",
        "../assets/Modelos3D/structure-platform.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(50.0f), 0.0f));

    entities.emplace_back(
        2.3f, -2.0f, -2.1f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.3f,
        "../assets/Modelos3D/structure-platform.obj",
        "../assets/Modelos3D/structure-platform.mtl",
        "../assets/tex/colormap.png",
        glm::vec3(0.0f, glm::radians(50.0f), 0.0f));

    entities[4].loadTrajectory("../assets/trajeto.txt");

    for (auto &entity : entities)
    {
        entity.initialize();
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.3f, 0.6f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(100.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

        for (auto &entity : entities)
        {
            entity.updateTrajectory();
            entity.setViewProjection(view, projection, camera.position);
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
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (key == GLFW_KEY_W)
            camera.processKeyboard('W');
        if (key == GLFW_KEY_S)
            camera.processKeyboard('S');
        if (key == GLFW_KEY_A)
            camera.processKeyboard('A');
        if (key == GLFW_KEY_D)
            camera.processKeyboard('D');

        Entity &selectedEntity = entities[selectedEntityIndex];

        if (key == GLFW_KEY_X)
            selectedEntity.toggleRotateX();
        if (key == GLFW_KEY_Y)
            selectedEntity.toggleRotateY();
        if (key == GLFW_KEY_Z)
            selectedEntity.toggleRotateZ();

        if (key == GLFW_KEY_I)
            selectedEntity.scaleUp();
        if (key == GLFW_KEY_O)
            selectedEntity.scaleDown();

        if (key == GLFW_KEY_U)
            selectedEntity.moveForward();
        if (key == GLFW_KEY_J)
            selectedEntity.moveBackward();

        if (key == GLFW_KEY_C)
            selectedEntityIndex = (selectedEntityIndex + 1) % entities.size();
    }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}
