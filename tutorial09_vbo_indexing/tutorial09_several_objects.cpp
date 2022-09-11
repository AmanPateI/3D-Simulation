/*
Author: Aman Patel
Class: ECE4122
Last Date Modified: 12/7/2021

Description:Main file for Buzzy Bowl. 
            Uses multithreading and OpenGL to develop 
            a 3D simlation of a UAV show.
*/
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow* window;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include "ECE_UAV.cpp"

enum ImageType { eBITMAP, OTHER };

// load objects
bool objectInit(ImageType ImageType, const std::string& ImageFilename, const std::string& ObjectFilename,
    GLuint& Texture, GLuint& Vertexbuffer, GLuint& UVbuffer, GLuint& Normalbuffer, GLuint& Elementbuffer,
    uint& nCount)
{
    //texture accessing
    if (ImageType == ImageType::eBITMAP)
    {
        Texture = loadBMP_custom(ImageFilename.c_str());
    }
    else
    {
        Texture = loadDDS(ImageFilename.c_str());
    }
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool resolution = loadOBJ(ObjectFilename.c_str(), vertices, uvs, normals);

    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
    nCount = indices.size();
    // VBO load
    glGenBuffers(1, &Vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
    glGenBuffers(1, &UVbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, UVbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);
    // normal buffer
    glGenBuffers(1, &Normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
    // indicies buffer
    glGenBuffers(1, &Elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
    return true;
}

void generateObject(GLuint& Texture, GLuint& TextureID, GLuint& Vertexbuffer, GLuint& UVbuffer, GLuint& Normalbuffer,
    GLuint& Elementbuffer, uint& nCount)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    glUniform1i(TextureID, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, Vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized
        0,                  // stride
        (void*)0            // array buffer offset
    );
    // UAV parameters
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, UVbuffer);
    glVertexAttribPointer(
        1,                                // attribute
        2,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );
    //normal parameters
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, Normalbuffer);
    glVertexAttribPointer(
        2,                                // attribute
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );
    // element array buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Elementbuffer);
    glDrawElements(
        GL_TRIANGLES,      
        nCount,    
        GL_UNSIGNED_SHORT,   
        (void*)0           
    );


}
// All UAV objects
ECE_UAV UAVObjects[15];
// main function
int main(void)
{
    //initialize UAV positions
    double xStart{ -130.0 }, xOffset{ 70.0 };
    double yStart{ 65 }, yOffset{ -65 };
    double uav_pos[3] = { 0.0,0.0,5.0 };
    double xScale{ 0 };
    double yScale{ 0 };
    for (int n = 0; n < 15; n++)
    {
        xScale = n % 5;
        yScale = n / 5;
        uav_pos[0] = xStart + xScale * xOffset;
        uav_pos[1] = yStart + yScale * yOffset;
        UAVObjects[n].position(uav_pos);
    }
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // window resolutions
    window = glfwCreateWindow(1080, 720, "Tutorial 09 - Rendering several models", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // Initialize GLEW
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    // used to exit the program
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // disable cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);
    // background color is blue
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    // texuture handle
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
    // Football field variables
    GLuint footballTexture;
    GLuint footballVertexBuffer;
    GLuint footballUVBuffer;
    GLuint footballNormalBuffer;
    GLuint footballElementBuffer;
    uint nCountFootballField = 0;
    const std::string ImageFilename = "ff.bmp";
    const std::string ObjectFilename = "footballfield.obj";
    //initialize up football field object
    objectInit(ImageType::eBITMAP, ImageFilename, ObjectFilename, footballTexture, footballVertexBuffer,
        footballUVBuffer, footballNormalBuffer, footballElementBuffer, nCountFootballField);
    // UAV variable declaration
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);
    GLuint Texture = loadDDS("uvmap.DDS");
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
    // Load into VBO
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    float rotAngle = glm::half_pi<float>();
    //The start function starts the threads for the 15 UAVs
    for (int ii = 0; ii < 15; ii++)
    {
        UAVObjects[ii].start();
    }
    // Starts here
    do {
        // Measure speed
        double currentTime = glfwGetTime();
        nbFrames++;
        printf("Time elapsed is %f\n", (currentTime - lastTime));
        if (currentTime - lastTime >= 90.0)
        {
            for (int ii = 0; ii < 15; ii++)
            {
                UAVObjects[ii].stop();
            }
            break;
        }
        //clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        computeMatricesFromInputs();
        glm::mat4 ProjectionMatrix = glm::perspective(40.0f, 1.33f, 0.1f, 1000.0f); 
        glm::mat4 ViewMatrix = glm::lookAt(glm::vec3(0.0f, 50.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        glm::mat4 ModelMatrix1 = glm::mat4(1.0);
        glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);
        // generate football field
        generateObject(footballTexture, TextureID, footballVertexBuffer, footballUVBuffer, footballNormalBuffer, footballElementBuffer, nCountFootballField);
        glUseProgram(programID);
        // set light position
        glm::vec3 lightPos = glm::vec3(0, 200, 100);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
        // repeated
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glUniform1i(TextureID, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        // draw triangles
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);
        // collision check
        double positions[3];
        for (int i = 0; i < 15; i++)
        {
            UAVObjects[i].getPosition(positions);
            ModelMatrix1 = glm::mat4(1.0);
            ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(positions[0], positions[1], positions[2]));
            ModelMatrix1 = glm::rotate(ModelMatrix1, rotAngle, glm::vec3(0.0f, 1.0f, 1.0f));
            ModelMatrix1 = glm::rotate(ModelMatrix1, rotAngle, glm::vec3(-0.7f, 1.0f, 1.0f));
            ModelMatrix1 = glm::scale(ModelMatrix1, glm::vec3(3, 3, 3));
            MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);
            glDrawElements(
                GL_TRIANGLES, 
                indices.size(), 
                GL_UNSIGNED_SHORT, 
                (void*)0 
            );
            for (int j = 0; j < 15; j++) {
                if (i != j) {
                    if (UAVObjects[j].checkCollision(positions)) {
                        double otherVelocity[3], currVelocity[3];
                        UAVObjects[j].getVelocity(otherVelocity);
                        UAVObjects[i].getVelocity(currVelocity);
                        UAVObjects[j].velocity(currVelocity);
                        UAVObjects[i].velocity(otherVelocity);
                    }
                }
            }
        }
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        // swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));

    } // use escape key to close program
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);
    for (int ii = 0; ii < 15; ii++)
    {
        UAVObjects[ii].stop();
    }
    glDeleteBuffers(1, &footballVertexBuffer);
    glDeleteBuffers(1, &footballUVBuffer);
    glDeleteBuffers(1, &footballNormalBuffer);
    glDeleteBuffers(1, &footballElementBuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &footballTexture);

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}