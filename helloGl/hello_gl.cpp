#define GLEW_STATIC
#define DEBUG

#include <windows.h>

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include"VertexBuffer.h"
#include"IndexBuffer.h"
#include"VertexArray.h"
#include"VertexBufferLayout.h"
#include "texture.h"
#include"Shader.h"
#include "Camera.h"
#include "Framebuffer.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float deltaTime = 0.0f;
bool firstMouse = true;
float lastX;
float lastY;
int screen_width, screen_height;
struct ShaderState {
    int KeyPressed = 0;
    bool ReloadShader = false;
    bool debug = false;
};

ShaderState state;
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
using namespace Renderer;
void validate(Shader& shader, VertexArray& VAO);

int main(void)
{
    screen_width = GetSystemMetrics(SM_CXSCREEN);
    screen_height = GetSystemMetrics(SM_CYSCREEN);

    lastX = screen_width / 2.0f;
    lastY = screen_height / 2.0f;

    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(screen_width, screen_height, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() != GLEW_OK) std::cout << "error" << std::endl;

    {
        glEnable(GL_DEPTH_TEST);
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float cubeVertices[] = {
            // back face
           -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
           -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
           -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
           // front face
           -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
           -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
           -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
           // left face
           -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
           -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
           -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
           -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
           -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
           -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
           // right face
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
           // bottom face
           -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
           -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
           -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
           // top face
           -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
            1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
           -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
           -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        float planeVertices[] = {
            // positions            // normals         // texcoords
             25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
            -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
            -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

             25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
            -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
             25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
        };
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, 0.5f, 0.0f, 0.0f,
            -0.5f, 1.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, 1.0f, 0.0f,
        };

        VertexBuffer cubeVBO(cubeVertices, sizeof(cubeVertices));
        VertexBuffer planeVBO(planeVertices, sizeof(planeVertices));
        VertexBuffer debugVBO(quadVertices, sizeof(quadVertices));
        //VertexBuffer transparentVBO(transparentVertices, sizeof(transparentVertices));
        VertexBufferLayout layout1, layout2;
        layout1.push<float>(3);
        layout1.push<float>(3);
        layout1.push<float>(2);
        layout2.push<float>(2);
        layout2.push<float>(2);
// VAO
        VertexArray cubeVAO, planeVAO, debugVAO;
        cubeVAO.bind();
        cubeVAO.AddBuffer(cubeVBO, layout1);
        planeVAO.bind();
        planeVAO.AddBuffer(planeVBO, layout1);
        debugVAO.bind();
        debugVAO.AddBuffer(debugVBO, layout2);

//shader initializtion
        const std::string shader_vs = "src/shaders/blinn_phong.vert";
        const std::string shader_fs = "src/shaders/blinn_phong.frag";
        Shader shader(shader_vs, shader_fs);
        Shader shadowmap("src/shaders/shadowmap.shader");
        Shader DebugShader("src/shaders/display_map.shader");

//texture initializtion
        Texture cubeTexture("src/vendor/assets/container2.png");
        Texture floorTexture("src/vendor/assets/wood.png");
        //Texture windowTexture("src/vendor/assets/window.png");


//framebuffer
        Framebuffer depthMapFBO(screen_width, screen_height, Texturebuffer_depth);
        depthMapFBO.bind();
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        depthMapFBO.unbind();

//light transform
        float near_plane = 1.0f, far_plane =10.0f;
        glm::vec4 lightPos(-2.0f, 4.0f, -1.0f, 0.0);
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);    //directional light
        glm::mat4 lightView = glm::lookAt(glm::vec3(lightPos), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

//model_view
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 cube_model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
        cube_model = glm::scale(cube_model, glm::vec3(0.5f));

        glm::mat4 cube_model2 = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
        cube_model2 = glm::scale(cube_model2, glm::vec3(0.5f));

        glm::mat4 inv_model = glm::inverse(cube_model);
        glm::mat4 inv_model2 = glm::inverse(cube_model2);
        glm::mat3 inv_eye = glm::transpose(glm::inverse(glm::mat3(model)));

//shader config
    start:
        std::cout << "configuring shader\n";
        shader.bind();
        shader.SetUniform1i("diffuse1", 0);
        
        shader.SetUniform1i("shadowMap", 1);
        shader.SetUniformMat4f("lightSpaceMatrix", lightSpaceMatrix);
        shader.SetUniform4fv("lightPos", lightPos);
        shader.unbind();

        shadowmap.bind();
        shadowmap.SetUniformMat4f("lightSpaceMatrix", lightSpaceMatrix);
        shadowmap.unbind();

        DebugShader.bind();
        depthMapFBO.bindTexture(0);
        DebugShader.SetUniform1f("near_plane", near_plane);
        DebugShader.SetUniform1f("far_plane", far_plane);
        DebugShader.SetUniform1i("depthMap", 0);
        DebugShader.unbind();

        /* Loop until the user closes the window */
        float lastFrame = 0.0f, currentFrame = 0.0f;
        while (!glfwWindowShouldClose(window))
        {

            GlClearError();
            std::cout << std::flush;
            //getting frame time
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            //windows input
            processInput(window);

            //depth map
            depthMapFBO.bind();
            glClear(GL_DEPTH_BUFFER_BIT);
            shadowmap.bind();
            cubeVAO.bind();
            shadowmap.SetUniformMat4f("model", cube_model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            shadowmap.SetUniformMat4f("model", cube_model2);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            planeVAO.bind();
            shadowmap.SetUniformMat4f("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            depthMapFBO.unbind();

            //render the color
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            shader.bind();
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();
            shader.SetUniformMat4f("projection", projection);
            shader.SetUniformMat4f("view", view);
            shader.SetUniform3fv("viewPos", camera.Position);


            cubeTexture.bind(0);
            depthMapFBO.bindTexture(1);
            cubeVAO.bind();
            shader.SetUniformMat4f("model", cube_model);
            shader.SetUniformMat4f("inv_model", inv_model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            shader.SetUniformMat4f("model", cube_model2);
            shader.SetUniformMat4f("inv_model", inv_model2);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            floorTexture.bind(0);
            planeVAO.bind();
            shader.SetUniformMat4f("model", model);
            shader.SetUniformMat4f("inv_model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            cubeTexture.unbind();

            //display shadow map
            if (state.KeyPressed == 1) {
                DebugShader.bind();
                depthMapFBO.bindTexture(0);
                debugVAO.bind();
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            if (state.ReloadShader) {
                shader.unbind();
                std::cout << "reloading shader...\n";
                state.ReloadShader = false;
                Shader tmp(shader_vs, shader_fs);
                shader = tmp;
                goto start;
            }
            
        }
    }
    glfwTerminate();
    return 0;
}

void validate(Shader& shader, VertexArray& VAO) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.bind();
    VAO.bind();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.SetUniformMat4f("projection", projection);
    shader.SetUniformMat4f("view", view);
    shader.SetUniform3fv("viewPos", camera.Position);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
        state.ReloadShader = true;
    if (key == GLFW_KEY_0 && action == GLFW_PRESS)
        state.KeyPressed = 0;
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
        state.KeyPressed = 1;
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
        state.debug = !state.debug;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}