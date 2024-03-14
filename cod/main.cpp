#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

glm::vec3 lightPointPos;
glm::vec3 lightPointColor;

float color = 1.0f;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint lightPosLoc;
GLint fogDensityLoc;

// camera
gps::Camera myCamera(
    glm::vec3(-35.0f, 10.0f, 20.0f),
    glm::vec3(2.0f, 1.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;
GLfloat fogDensity;
float cameraSensitivity = 0.6;

GLboolean pressedKeys[1024];

// models
gps::Model3D scena;
gps::Model3D tractor;
GLfloat angle;
int retina_width, retina_height;

// shaders
gps::Shader myBasicShader;
gps::Shader myCustomShader;

gps::SkyBox mySkyBox; 
gps::Shader skyboxShader;
std::vector<const GLchar*> day;
std::vector<const GLchar*> night;

bool day_night = true;
bool flag = true;

GLenum glCheckError_(const char *file, int line)
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

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    // get the new dimensions of the window
    glfwGetFramebufferSize(window, &retina_width, &retina_height);

    // recompute the projection matrix and send it to the shader
    myCustomShader.useShaderProgram();
    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // redraw the window
    glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (!flag) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        if (key >= 0 && key < 1024) {
            if (action == GLFW_PRESS) {
                pressedKeys[key] = true;
            }
            else if (action == GLFW_RELEASE) {
                pressedKeys[key] = false;
            }
        }
        if (pressedKeys[GLFW_KEY_Z]) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //solid
        }
        if (pressedKeys[GLFW_KEY_X]) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //wireframe
        }
        if (pressedKeys[GLFW_KEY_C]) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); //poligonal
        }

        if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) { //crestere viteza
            cameraSpeed = 0.35f;
        }
        else {
            cameraSpeed = 0.15f;
        }
        if (pressedKeys[GLFW_KEY_P]) { //elimina cursor mouse
            glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        if (pressedKeys[GLFW_KEY_O]) { //activeaza cursor mouse
            glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        if (pressedKeys[GLFW_KEY_K]) {
            day_night = not(day_night);
            if (day_night) {
                mySkyBox.Load(day);
                myBasicShader.useShaderProgram();
                lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
                glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
            }
            else {
                mySkyBox.Load(night);
                myBasicShader.useShaderProgram();
                lightColor = glm::vec3(1.0f, 1.0f, 1.0f) * 0.1f;
                glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

            }
        }

        if (pressedKeys[GLFW_KEY_1]) { //scade lumina
            myBasicShader.useShaderProgram();
            if (color > 0.0f) {
                color -= 0.1f;
            }
            lightColor = glm::vec3(1.0f, 1.0f, 1.0f) * color;
            glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
        }
        if (pressedKeys[GLFW_KEY_2]) { //creste lumina
            myBasicShader.useShaderProgram();
            if (color < 2.0f) {
                color += 0.1f;
            }
            lightColor = glm::vec3(1.0f, 1.0f, 1.0f) * color;
            glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
        }


        if (pressedKeys[GLFW_KEY_3]) {
            myBasicShader.useShaderProgram();
            fogDensity += 0.001;
            glUniform1fv(fogDensityLoc, 1, &fogDensity);
        }
        if (pressedKeys[GLFW_KEY_4]) {
            myBasicShader.useShaderProgram();
            if (fogDensity > 0) {
                fogDensity -= 0.001;
            }
            glUniform1fv(fogDensityLoc, 1, &fogDensity);
        }


    }
}

bool firstMove = true;
double xprev = 0, yprev = 0;
float pitch, yaw = -90.0f;

void processRotation() {
    myCamera.rotate(pitch, yaw);
    // Actualizați view matrix pentru a reflecta modificările
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // Calculați matricea normală pentru teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!flag) {
        if (firstMove) {
            xprev = xpos;
            yprev = ypos;
            firstMove = false;
        }

        float xoffset = xpos - xprev;
        float yoffset = yprev - ypos;
        xprev = xpos;
        yprev = ypos;

        xoffset *= cameraSensitivity;
        yoffset *= cameraSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // Limitați unghiurile pitch pentru a evita probleme cu camera
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }

        processRotation();
    }
}

void processMovement() {
        if (pressedKeys[GLFW_KEY_W]) {
            myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
            //update view matrix
            view = myCamera.getViewMatrix();
            myBasicShader.useShaderProgram();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            // compute normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        }

        if (pressedKeys[GLFW_KEY_S]) {
            myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
            //update view matrix
            view = myCamera.getViewMatrix();
            myBasicShader.useShaderProgram();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            // compute normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        }

        if (pressedKeys[GLFW_KEY_A]) {
            myCamera.move(gps::MOVE_LEFT, cameraSpeed);
            //update view matrix
            view = myCamera.getViewMatrix();
            myBasicShader.useShaderProgram();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            // compute normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        }

        if (pressedKeys[GLFW_KEY_D]) {
            myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
            //update view matrix
            view = myCamera.getViewMatrix();
            myBasicShader.useShaderProgram();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            // compute normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        }

        if (pressedKeys[GLFW_KEY_Q]) {
            angle -= 1.0f;
            // update model matrix for teapot
            model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
            // update normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        }

        if (pressedKeys[GLFW_KEY_E]) {
            angle += 1.0f;
            // update model matrix for teapot
            model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
            // update normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        }
 }



void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    scena.LoadModel("models/teapot/scena.obj");
    tractor.LoadModel("models/teapot/tractor.obj");
}

void initSkybox() {
    day.push_back("skybox/day/left.png");
    day.push_back("skybox/day/right.png");
    day.push_back("skybox/day/top.png");
    day.push_back("skybox/day/bottom.png");
    day.push_back("skybox/day/back.png");
    day.push_back("skybox/day/front.png");

    night.push_back("skybox/night/left.png");
    night.push_back("skybox/night/right.png");
    night.push_back("skybox/night/top.png");
    night.push_back("skybox/night/bottom.png");
    night.push_back("skybox/night/back.png");
    night.push_back("skybox/night/front.png");


    mySkyBox.Load(day);

}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightPointPos = glm::vec3(29.3f, -5.7f, 2.7f); // pozitia luminii punctiforme
    lightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "PointPosition");
    // send light color to shader
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPointPos));

    lightPointColor = glm::vec3(1.0f, 1.0f, 1.0f); // culoarea luminii punctiforme
    GLint lightPosColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPointColor");
    // send light color to shader
    glUniform3fv(lightPosColorLoc, 1, glm::value_ptr(lightPointColor));

    fogDensity = 0.0f;
    fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    glUniform1fv(fogDensityLoc, 1, &fogDensity);
}

void renderScene(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    scena.Draw(shader);
}

float rotationTractor = 180.0f;;
float tractorAnimatie = 0.0f;
int count = 0;
bool ok = true;
void renderTractor(gps::Shader shader) {

    shader.useShaderProgram();
    glm::mat4 model2 = glm::mat4(1.0f);
    if (ok) {
        tractorAnimatie += 0.05f;
        count++;
    }
    else {
        tractorAnimatie -= 0.05f;
        count++;
    }


    if (count % 520 == 0) {
        ok = ok ? false : true;
        count = 0;
    }

    if (!ok) {
        model2 = glm::translate(model2, glm::vec3(-21.75, 0.85, 36-(0.05f*count)));
        model2 = glm::rotate(model2, glm::radians(rotationTractor), glm::vec3(0.0f, 1.0f, 0.0f));
        model2 = glm::translate(model2, -glm::vec3(-21.75, 0.85, 36 - (0.05f * count)));
    }
    model2 = glm::translate(model2, glm::vec3(0.0f, 0.0f, tractorAnimatie));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model2));
    tractor.Draw(shader);

}



void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render the scene

	// render the scene and tractor
	renderScene(myBasicShader);
    renderTractor(myBasicShader);
    
    mySkyBox.Draw(skyboxShader, view, projection);

}


void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

float start_program;
float timp_curent;
float durata = 15.0f;
float animatie = 0.0f;

void animatie_inceput() {
    timp_curent = glfwGetTime();
    if (timp_curent - start_program > durata) {
        flag = false;
    }
    if ((timp_curent - start_program > 4 && timp_curent - start_program < 6) || (timp_curent - start_program > 7.5 && timp_curent - start_program < 9.5) || (timp_curent - start_program > 11 && timp_curent - start_program < 14)) {
        yaw += 0.7f;
        animatie -= 0.0007f;
    }
    else {
        animatie += 0.001f;
    }

    if (timp_curent - start_program > 7 && timp_curent - start_program < 10) {
        pitch += 0.001f;
    }
    if (timp_curent - start_program > 10 && timp_curent - start_program < 15) {
        pitch += 0.001f;
    }

    processRotation();
    myCamera.move(gps::MOVE_FORWARD, animatie);

}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();
    initSkybox();
    start_program = glfwGetTime();
    pitch -= 2.0f;
    processRotation();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        if (flag) {
            animatie_inceput();
        }
        if (!flag) {
            processMovement();
            processRotation();
        }
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
