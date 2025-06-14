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
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Desafio Módulo 5 - Laura Skorupski", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
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

    for (auto &entity : entities) {
        entity.initialize();
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(80.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

        for (auto &entity : entities) {
            entity.setViewProjection(view, projection, camera.position);
            entity.draw();
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_W)    camera.processKeyboard('W');
        if (key == GLFW_KEY_S)  camera.processKeyboard('S');
        if (key == GLFW_KEY_A)  camera.processKeyboard('A');
        if (key == GLFW_KEY_D) camera.processKeyboard('D');

        Entity &selectedEntity = entities[selectedEntityIndex];

        if (key == GLFW_KEY_X) selectedEntity.toggleRotateX();
        if (key == GLFW_KEY_Y) selectedEntity.toggleRotateY();
        if (key == GLFW_KEY_Z) selectedEntity.toggleRotateZ();

        if (key == GLFW_KEY_I) selectedEntity.scaleUp();
        if (key == GLFW_KEY_O) selectedEntity.scaleDown();

        if (key == GLFW_KEY_U) selectedEntity.moveForward();
        if (key == GLFW_KEY_J) selectedEntity.moveBackward();

        if (key == GLFW_KEY_C)
            selectedEntityIndex = (selectedEntityIndex + 1) % entities.size();
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
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
