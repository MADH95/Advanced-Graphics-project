/*

Name: Martin Harvey

Student ID: B00329330

I declare that the following code was produced by Martin Harvey as a individual assignment for the AGP module and that this is my own work.

I am aware of the penalties incurred by submitting in full or in part work that is not our own and that was developed by third parties that are not appropriately acknowledged.

*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void assignLight(Shader shader);
void renderFloor(const Shader& shader);
void renderQuad();

const unsigned int WINDOW_WIDTH = 1920;
const unsigned int WINDOW_HEIGHT = 1080;

// Camera
Camera camera(glm::vec3(-1.5f, 1.75f, 20.0f));
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f; // Time between current and last frame
float lastFrame = 0.0f; // Time of last frame

glm::vec3 lightDir(1.0f, -2.0f, 1.0f);

unsigned int planeVAO;

int main()
{
    //  Initialize and configure glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create glfw window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Graphics Project", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Capture users mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // Configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // Build and compile shader programs
    Shader modelShader("Data/Shaders/Model/model_loading.vs", "Data/Shaders/Model/model_loading.fs");
    Shader floorShader("Data/Shaders/Floor/floor.vs", "Data/Shaders/Floor/floor.fs");
    Shader depthShader("Data/Shaders/Shadow_Mapping/shadow_mapping_depth.vs", "Data/Shaders/Shadow_Mapping/shadow_mapping_depth.fs");
    Shader debugDepthShader("Data/Shaders/Shadow_Mapping/debug.vs", "Data/Shaders/Shadow_Mapping/debug.fs");

    // Load models
    Model mannequin("Data/Models/mannequin/mannequin_model.fbx");

    // Plane
    float planeVertices[] = {
        // positions            // normals         // texcoords
         25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };

    unsigned int planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int woodTexture = loadTexture("Data/Textures/wood.png");


    //Depth map
    const unsigned int SHADOW_WIDTH = 8192, SHADOW_HEIGHT = 8192;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindRenderbuffer(GL_RENDERBUFFER, depthMap);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SHADOW_WIDTH, SHADOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthMap);

    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        throw new std::exception("Can't initialize an FBO render texture. FBO initialization failed.");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // shader configuration
    modelShader.use();
    modelShader.setInt("shadowMap", 1);

    floorShader.use();
    floorShader.setInt("shadowMap", 1);

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per frame logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process player input
        processInput(window);

        // Render
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //set up matricies
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        

        // Render depth of scene to texture
        float near_plane = -12.0f, far_plane = 14.0f;
        glm::mat4 lightProjection = ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
        glm::mat4 lightView = lookAt(-lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);


        // render the loaded model
        for (int y = -5; y < 5; y++)
        {
            for (int x = -5; x < 5; x++)
            {
                model = glm::mat4(1.0f);
                model = translate(model, glm::vec3((float)x * 3, 0.01f, (float)y * 3));
                model = scale(model, glm::vec3(0.0035f));
                mannequin.draw(depthShader, model);
            }
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        renderFloor(depthShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate floor shader
        floorShader.use();
        floorShader.setVec3("viewPos", camera.Position);
        floorShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        assignLight(floorShader);
        floorShader.setFloat("material.shininess", 8.0f);
        floorShader.setMat4("view", view);
        floorShader.setMat4("projection", projection);

        // draw floor
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderFloor(floorShader);

        // Activate model shader 
        modelShader.use();
        modelShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        modelShader.setVec3("viewPos", camera.Position);
        assignLight(modelShader);
        modelShader.setFloat("material.shininess", 8.0f);
        modelShader.setMat4("view", view);
        modelShader.setMat4("projection", projection);

        for (int y = -5; y < 5; y++)
        {
            for (int x = -5; x < 5; x++)
            {
                model = glm::mat4(1.0f);
                model = translate(model, glm::vec3((float)x * 3, 0.01f, (float)y * 3));
                model = scale(model, glm::vec3(0.0035f));
                mannequin.draw(modelShader, model);
            }
        }

        debugDepthShader.use();
        debugDepthShader.setFloat("near_plane", near_plane);
        debugDepthShader.setFloat("far_plane", far_plane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        //renderQuad();

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // de-allocate resources
    glDeleteBuffers(1, &planeVBO);

    // Clear all glfw resources
    glfwTerminate();
    return 0;
}

// Render scene
void renderFloor(const Shader& shader)
{
    // floor
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(0.0f, -0.01f, 0.0));
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

//Render quad
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

bool sprinting = false;
// Process all input
void processInput(GLFWwindow* window)
{
    // Close window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // Player movement
    const float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime, sprinting);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime, sprinting);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime, sprinting);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime, sprinting);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        sprinting = true;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS)
        sprinting = false;

}

// Runs if window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Make sure viewport matches window dimensions
    glViewport(0, 0, width, height);
}

// Runs when the mouse moves
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // y coords go bottom to top
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Runs when scroll wheel scrolls
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// load texture
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = 0;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

//Function to assign point light values in a shader
void assignLight(Shader shader)
{
    shader.setVec3("light.direction", lightDir);
    shader.setVec3("light.ambient", glm::vec3(0.8f));
    shader.setVec3("light.diffuse", glm::vec3(0.5f));
    shader.setVec3("light.specular", glm::vec3(0.5f));
    shader.setFloat("light.constant", 1.0f);
    shader.setFloat("light.linear", 0.09f);
    shader.setFloat("light.quadratic", 0.032f);
}