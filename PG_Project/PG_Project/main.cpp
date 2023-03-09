#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp" //core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "Skybox.hpp"

#include <iostream>

#define NUMBER_OF_LIGHTS 13

const unsigned int SHADOW_WIDTH = 15048;
const unsigned int SHADOW_HEIGHT = 15048;

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir[NUMBER_OF_LIGHTS];
glm::vec3 lightColor[NUMBER_OF_LIGHTS];
GLint lightEnable[NUMBER_OF_LIGHTS];
glm::vec3 lightRotation;
GLint color[10];

GLuint shadowMapFBO;
GLuint depthMapTexture;
bool showDepthMap;


// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;
GLuint lightEnableLoc;
GLuint enableDiscardLoc;
GLuint colorLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));
gps::Camera myCameraPresentation(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.7f;

GLboolean pressedKeys[1024];

// models
gps::Model3D nanosuit;
gps::Model3D nanosuit2;
gps::Model3D lightCube;
gps::Model3D lightCubes[10];
gps::Model3D screenQuad;
gps::Model3D scene1;
gps::Model3D scene2;
gps::Model3D scene3;
gps::Model3D windmill;
gps::Model3D water[2000];

GLfloat angleY, lightAngle, windAngle;

// shaders
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

//skybox
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
GLuint textureID;

//mouse
bool firstMouse = true;
double lastX, lastY, mouseSensitivity = 0.1f;
double yaw = -90.0f, pitch = 0.0f;

glm::vec3 aux, aux2, water_drops[2000];
bool presentation = 0;

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


int steps;
void startPresentation()
{
    steps = 0;
    myCameraPresentation.set(glm::vec3(15.645752f, 2.912375f, 70.467758f), glm::vec3(15.415703f, 2.912375f, 69.494576f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void progress()
{
    if (steps < 80)
    {
        myCameraPresentation.move(gps::MOVE_BACKWARD, cameraSpeed);
    }

    if(steps == 80) myCameraPresentation.set(glm::vec3(66.805054f, 16.160664f, 165.931976f), glm::vec3(65.867821f, 16.059607f, 165.598236f), glm::vec3(0.0f, 1.0f, 0.0f));
        
    if (steps > 80 && steps < 150)
    {
        myCameraPresentation.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    if (steps == 150) myCameraPresentation.set(glm::vec3(81.684181f, 5.313303f, 34.770737f), glm::vec3(81.323334f, 5.198366f, 35.696255f), glm::vec3(0.0f, 1.0f, 0.0f));

    if (steps > 150 && steps < 220)
    {
        myCameraPresentation.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    if (steps == 220) myCameraPresentation.set(glm::vec3(-146.814026f, 6.735712f, 113.727684f), glm::vec3(-145.862518f, 6.676405f, 113.425850f), glm::vec3(0.0f, 1.0f, 0.0f));

    if (steps > 220 && steps < 300)
    {
        myCameraPresentation.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    if(steps == 300)
    {
        presentation = 0;
    }
    steps++;
}

void windowResizeCallback(GLFWwindow* window, int width, int height) 
{
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) 
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
        lightEnable[0] = !lightEnable[0];
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
        lightEnable[1] = !lightEnable[1];
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
        lightEnable[2] = !lightEnable[2];
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
    {
        presentation = !presentation;
        startPresentation();
    }

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
        {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (presentation)return;
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

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
}

void processMovement()
{
    if (presentation) return;
    if (pressedKeys[GLFW_KEY_W])
    {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_S])
    {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_A])
    {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_D])
    {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_SPACE])
    {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_LEFT_CONTROL])
    {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_Q])
    {
        lightAngle -= 1.00f;
        if (lightAngle < 0.0f) lightAngle += 360.0f;
    }

    if (pressedKeys[GLFW_KEY_E])
    {
        lightAngle += 1.00f;
        if (lightAngle > 360.0f) lightAngle -= 360.0f;
    }

    if (pressedKeys[GLFW_KEY_Z]) 
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (pressedKeys[GLFW_KEY_X]) 
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (pressedKeys[GLFW_KEY_C]) 
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
}

void initOpenGLWindow() 
{
    myWindow.Create(1920, 1080, "OpenGL Project Core");
}

void setWindowCallbacks() 
{
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() 
{
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

    glEnable(GL_FRAMEBUFFER_SRGB);
}

void initModels()
{
    scene1.LoadModel("models/scene/scene1.obj");
    scene2.LoadModel("models/scene/scene2.obj");
    scene3.LoadModel("models/scene/scene3.obj");
    lightCube.LoadModel("models/cube/cube.obj");
    lightCubes[0].LoadModel("models/cubes/cube1.obj");
    lightCubes[1].LoadModel("models/cubes/cube2.obj");
    lightCubes[2].LoadModel("models/cubes/cube3.obj");
    lightCubes[3].LoadModel("models/cubes/cube4.obj");
    lightCubes[4].LoadModel("models/cubes/cube5.obj");
    lightCubes[5].LoadModel("models/cubes/cube6.obj");
    lightCubes[6].LoadModel("models/cubes/cube7.obj");
    lightCubes[7].LoadModel("models/cubes/cube8.obj");
    lightCubes[8].LoadModel("models/cubes/cube9.obj");
    lightCubes[9].LoadModel("models/cubes/cube10.obj");
    screenQuad.LoadModel("models/quad/quad.obj");
    windmill.LoadModel("models/windmill/windmill.obj");
    water[0].LoadModel("models/water/water.obj");
    srand(time(0));

    for (int i = 1; i < 2000; i++)
    {
        water[i] = water[i-1];
    }

    for (int i = 0; i < 2000; i++)
    {
        float x = (rand() / (float)RAND_MAX) * 30 * 9 - 15;
        float y = (rand() / (float)RAND_MAX) * 10;
        float z = (rand() / (float)RAND_MAX) * 25 * 9 - 3;
        water_drops[i] = glm::vec3(x, y, z);
    }
        
    faces.push_back("models/skybox/right.tga");
    faces.push_back("models/skybox/left.tga");
    faces.push_back("models/skybox/top.tga");
    faces.push_back("models/skybox/bottom.tga");
    faces.push_back("models/skybox/back.tga");
    faces.push_back("models/skybox/front.tga");

    mySkyBox.Load(faces);
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");

    skyboxShader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 2000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));
}

void initShaders() 
{
    myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    myCustomShader.useShaderProgram();
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    lightShader.useShaderProgram();
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    screenQuadShader.useShaderProgram();
    depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
    depthMapShader.useShaderProgram();
}

void initUniforms() 
{
    myCustomShader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 2000.0f);
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    enableDiscardLoc = glGetUniformLocation(myCustomShader.shaderProgram, "enableDiscard");
    glUniform1i(enableDiscardLoc, 0);

    //set the light direction (direction towards the light)
    lightRotation = glm::vec3(0.0f, 12.0f, -17.0f);
    lightDir[0] = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightRotation, 1.0f));
    lightDir[1] = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightRotation, 1.0f));
    lightDir[2] = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightRotation, 1.0f));

    lightDir[3] = glm::vec3(-1.82107f, 0.227888f, 11.8556f);
    lightDir[4] = glm::vec3(2.04074f, 0.227888f, 11.8556f);
    lightDir[5] = glm::vec3(-1.82107f, 0.227888f, -8.54741f);
    lightDir[6] = glm::vec3(2.04074f, 0.227888f, -8.54741f);
    lightDir[7] = glm::vec3(-1.82107f, 0.227888f, 5.66662f);
    lightDir[8] = glm::vec3(2.04074f, 0.227888f, 5.66662f);
    lightDir[9] = glm::vec3(-1.82107f, 0.227888f, 2.54881f);
    lightDir[10] = glm::vec3(2.04074f, 0.227888f, 2.54881f);
    lightDir[11] = glm::vec3(-4.56042f, 0.227888f, 2.00436f);
    lightDir[12] = glm::vec3(8.42223f, 0.227888f, 2.00436f);


    lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, NUMBER_OF_LIGHTS, glm::value_ptr(lightDir[0]));

    //set light color
    lightColor[0] = glm::vec3(1.0f, 0.0f, 0.0f); //red light
    lightColor[1] = glm::vec3(0.0f, 1.0f, 0.0f); //green light
    lightColor[2] = glm::vec3(0.0f, 0.0f, 1.0f); //blue light
    for (int i = 3; i < NUMBER_OF_LIGHTS; i++)
    {
        lightColor[i] = glm::vec3(1.0f, 0.0f, 0.0f); //red light
    }
    lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, NUMBER_OF_LIGHTS, glm::value_ptr(lightColor[0]));

    //set which lights are on
    lightEnable[0] = 1;
    lightEnable[1] = 1;
    lightEnable[2] = 1;
    for (int i = 3; i < NUMBER_OF_LIGHTS; i++)
    {
        lightEnable[i] = 0;
    }
    lightEnableLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightEnable");
    glUniform1iv(lightEnableLoc, NUMBER_OF_LIGHTS, lightEnable);

    //cubes
    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    colorLoc = glGetUniformLocation(lightShader.shaderProgram, "color");
    for (int i = 0; i < 10; i++)
    {
        color[i] = 0;
    }
    glUniform1i(colorLoc, 0);
}

void initFBO() 
{
    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
    glm::mat4 lightView = glm::lookAt(lightDir[0], glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightProjection = glm::ortho(-125.0f, 125.0f, -100.0f, 120.0f, 1.0f, 180.0f);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}

float delta = 0;
float movementSpeed = 10; // units per second
void updateDelta(double elapsedSeconds)
{
    delta = delta + movementSpeed * elapsedSeconds;
}
double lastTimeStamp = glfwGetTime();

void drawObjects(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();
    if (!depthPass) glUniform1i(enableDiscardLoc, 0);

    // scene
    if (!depthPass) glUniform1i(enableDiscardLoc, 1);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(9.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    scene1.Draw(shader);
    scene2.Draw(shader);
    scene3.Draw(shader);

    // get current time
    double currentTimeStamp = glfwGetTime();
    updateDelta(currentTimeStamp - lastTimeStamp);
    lastTimeStamp = currentTimeStamp;

    model = glm::translate(model, glm::vec3(-0.374719f, 1.66209f, -0.749788f));
    model = glm::rotate(model, glm::radians(delta), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(0.374719f, -1.66209f, 0.749788f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    windmill.Draw(shader);

    for (int i = 0; i < 2000; i++)
    {
        water_drops[i].y -= 0.10f;
        if (water_drops[i].y < -2)
        {
            water_drops[i] = glm::vec3((rand() / (float)RAND_MAX) * 30 * 9 - 15 * 9, (rand() / (float)RAND_MAX) * 7, (rand() / (float)RAND_MAX) * 25 * 9 - 3 * 9);
        }
        
        model = glm::translate(glm::mat4(1.0f), water_drops[i]);
        model = glm::scale(model, glm::vec3(1/90.0f));

        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        water[i].Draw(shader);
    }
}

void renderSceneToDepthBuffer()
{
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    drawObjects(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene()
{
    // render depth map on screen - toggled with the M key
    if (showDepthMap)
    {
        lightDir[0] = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightRotation, 1.0f));

        renderSceneToDepthBuffer();

        glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();

        //bind the depth map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }
    else
    {

        renderSceneToDepthBuffer();

        // final scene rendering pass (with shadows)

        glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myCustomShader.useShaderProgram();

        if (!presentation)view = myCamera.getViewMatrix();
        else
        {
            progress();
            view = myCameraPresentation.getViewMatrix();
        }
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        lightDir[0] = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightRotation, 1.0f));
        lightDir[1] = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightRotation, 1.0f));
        lightDir[2] = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightRotation, 1.0f));
        glUniform3fv(lightDirLoc, NUMBER_OF_LIGHTS, glm::value_ptr(lightDir[0]));

        glUniform1iv(lightEnableLoc, NUMBER_OF_LIGHTS, lightEnable);

        //bind the shadow map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

        glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(computeLightSpaceTrMatrix()));

        drawObjects(myCustomShader, false);

        //draw a white cube around the light

        lightShader.useShaderProgram();

        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(9.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        for (int i = 0; i < 10; i++)
        {
            float auxx = myCamera.getDistance(lightDir[i + 3]);
            if (auxx < 5.0f)
            {
                //glUniform1i(colorLoc, color[i]);
                glUniform1i(colorLoc, 1);
                //lightEnable[i] = 1;
            }
            else
            {
                glUniform1i(colorLoc, 0);
                //lightEnable[i] = 0;
            }
            lightCubes[i].Draw(lightShader);
        }

        model = glm::translate(model, 1.0f * lightDir[0]);
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        lightCube.Draw(lightShader);

        // skybox
        skyboxShader.useShaderProgram();
        mySkyBox.Draw(skyboxShader, view, projection);
    }
}

void cleanup()
{
    myWindow.Delete();
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);

}

int main(int argc, const char* argv[])
{
    try
    {
        initOpenGLWindow();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    initFBO();
    setWindowCallbacks();

    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);



    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow()))
    {
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}
