#include "glad.h"
#include <GLFW/glfw3.h>

#include "Game.h"
#include "ResourceManager.h"
#include "GameSettings.h"

#include <iostream>
#include <chrono>
#include <thread>

// GLFW function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// The Width of the screen
const unsigned int SCREEN_WIDTH = 800;
// The height of the screen
const unsigned int SCREEN_HEIGHT = 900;

Game* StackerGame;

int main(int argc, char *argv[])
{
    try {
        GameSettings settings("settings.toml");
        Game game{SCREEN_WIDTH, SCREEN_HEIGHT, settings};
        StackerGame = &game;

        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        glfwWindowHint(GLFW_RESIZABLE, false);
        glfwWindowHint(GLFW_REFRESH_RATE, 60);

        GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "StackerGame", nullptr, nullptr);
        glfwMakeContextCurrent(window);

        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }

        glfwSetKeyCallback(window, key_callback);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // OpenGL configuration
        // --------------------
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // initialize game
        // ---------------
        game.Init();

        // deltaTime variables
        // -------------------
        float deltaTime = 0.0f;
        float lastFrame = 0.0f;

        while (!glfwWindowShouldClose(window))
        {
            // calculate delta time
            // --------------------
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            glfwPollEvents();

            // manage user input
            // -----------------
            game.ProcessInput(deltaTime);

            // update game state
            // -----------------
            game.Update(deltaTime);

            // render
            // ------
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            game.Render();

            glfwSwapBuffers(window);
        }

        // delete all resources as loaded using the resource manager
        // ---------------------------------------------------------
        ResourceManager::Clear();

        glfwTerminate();
        return 0;
    }
    catch (const toml::parse_error& err)
    {
        std::cerr << "Parsing settings file failed:\n" << err << "\n";
        return 1;
    }
    catch (const std::exception& err) {
        std::cerr << "Error:\n" << err.what() << "\n";
        return 1;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // when a user presses the escape key, we set the WindowShouldClose property to true, closing the application
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            StackerGame->Keys[key] = true;
        else if (action == GLFW_RELEASE)
        {
            StackerGame->Keys[key] = false;
            StackerGame->KeysProcessed[key] = false;
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
