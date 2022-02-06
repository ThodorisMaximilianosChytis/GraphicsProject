#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <shader_m.h>
#include <filesystem.h>
#include <model.h>

#include <Sphere.h>
#include <camera.h>

#include <iostream>
#include <math.h>

#define SHPERE_ORBITAL_RADIUS glm::vec3(0.0f, 0.0f,  25.0f)
#define POINT_ZERO glm::vec3(0.0f, 0.0f, 0.0f)
#define PI 3.14159265359f

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(POINT_ZERO);

//shpere
float speed = 1;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    Model faceModel(FileSystem::getPath("resources/woman/woman1.obj"));
    Sphere litSphere(40, 40);

    Shader faceShader("shaders/face_shader.vs", "shaders/face_shader.fs");
    Shader litSphereShader("shaders/lit_sphere_shader.vs", "shaders/lit_sphere_shader.fs");

    // shader configuration
    // --------------------
    faceShader.use();
    faceShader.setInt("material.diffuse", 0);
    faceShader.setInt("material.specular", 1);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {

        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);



        //background color
        glClearColor(0.2f, 0.2f, 0.2f, 1.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        //setup face
        faceShader.use();
        faceShader.setVec3("light.position", lightPos);
        faceShader.setVec3("viewPos", camera.Position);

        // light properties
        faceShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        faceShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        faceShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // material properties
        faceShader.setFloat("material.shininess", 55.0f);

 
        faceShader.setMat4("projection", projection);
        faceShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, POINT_ZERO);
        // down scale
        model = glm::scale(model, glm::vec3(0.1f));
        faceShader.setMat4("model", model);

        // draw the face
        faceModel.Draw(faceShader);



        //LightPosition around the face
        float glfwGetTime_temp = glfwGetTime() * speed;
        lightPos.x = SHPERE_ORBITAL_RADIUS.z * sin(PI * 2.0f * (float )glfwGetTime_temp/360.0);
        lightPos.z = SHPERE_ORBITAL_RADIUS.z * cos(PI * 2.0f * (float )glfwGetTime_temp/360.0);
        
        
        // SETUP THE WIREFRAME LIT SPHERE
        litSphereShader.use();
        litSphereShader.setMat4("projection", projection);
        litSphereShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, POINT_ZERO);

        //Scale size of SHPERE
        model = glm::scale(model, glm::vec3(0.7f)); // a smaller cube
    
        //translate to lightPoistion which is orbiting around the the face
        model = glm::translate(model, lightPos);
        litSphereShader.setMat4("model", model);

        //draw shpere
        litSphere.Draw();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
        cout << "Exit" << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){
        speed += 0.05;             //faster speed
        cout << "speed is " << speed << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS){
        speed -= 0.05  ;          //slower speed   
        if (speed<1)
            speed=1;
        cout << "speed is " << speed << endl;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
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
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}






