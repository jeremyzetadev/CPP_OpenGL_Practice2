#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_video.h>
#include <glad/glad.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include "util.h"
using namespace std;

///// GLM /////
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi
///// GLM /////

#include "camera.hpp"

// #define SCREEN_HEIGHT 480
// #define SCREEN_WIDTH 640

struct App{
    int SCREEN_HEIGHT   = 480;
    int SCREEN_WIDTH    = 640;
    SDL_Window *m_GraphicsAppWindow = nullptr;
    SDL_GLContext *m_OpenGLContext = nullptr;
    bool m_Quit = false;

    // ShaderGraphics
    GLuint m_GraphicsPipelineShaderProgram = 0;

    Camera m_Camera;
};

struct Mesh3D{
    // VAO
    GLuint m_VertexArrayObject = 0;
    // VBO
    GLuint m_VertexBufferObject = 0;  //position + color
    // EBO
    GLuint m_ElementBufferObject = 0;
    // for glsl use uniform
    float m_uOffset = -1.0f;
    float m_uRotate = 0.0f;
    float m_uScale = 0.5f;
};


// Globals
App gApp;
Mesh3D gMesh1;
Mesh3D gMesh2;

#define ERROR_EXIT(...) {fprintf(stderr, __VA_ARGS__); exit(1);}
#define PRINTF(format, ...) \
    do { \
        fprintf(stderr, "[%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)
// #define printf(...) fprintf(stdout, ##__VA_ARGS__)


////// Error Handling Routines //////
static void GLClearAllErrors(){
    while(glGetError() != GL_NO_ERROR){
    }
}

static bool GLCheckErrorStatus(const char*function, int line){
    while(GLenum error = glGetError()){
        cout << "OpenGL Error: " << error 
             << "\tLine: " << line 
             << "\tfunction: " << function << endl;
        return true;
    }
    return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);
// wrap the function example -> GLCheck(gl_DrawElements(GL_TRIANGLES, 6, GL_INT,0);)
////// Error Handling Routines //////


GLuint CompileShader(GLuint type, const string source){
    GLuint shaderObject;
    if(type==GL_VERTEX_SHADER){
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    }else if(type==GL_FRAGMENT_SHADER){
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    }
    const char *src = source.c_str();
    glShaderSource(shaderObject, 1, &src, NULL);
    glCompileShader(shaderObject);
    return shaderObject;
}

GLuint CreateShaderProgram(const char *vertexFile, const char *fragmentFile){
    std::string vertexShaderSource = load_shader_as_string(vertexFile);       //get_file_contents(vertexFile);
    std::string fragmentShaderSource = load_shader_as_string(fragmentFile);   //get_file_contents(fragmentFile);
    GLuint programObject = glCreateProgram();
    GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);

    glValidateProgram(programObject);

    return programObject;
}

void CreateGraphicsPipeline(){
    gApp.m_GraphicsPipelineShaderProgram = CreateShaderProgram("Shader/vert.glsl","Shader/frag.glsl");
}

// Single VBO (position+color)
void VertexSpecification(Mesh3D *mesh){
    // Lives on the CPU
    const vector<GLfloat> vertexData{
        // Winding order CCW(is front face)
        // 0 - Vertex
        -0.5f, -0.5f, 0.0f,  //bottom left vertex
        1.0f, 0.0f, 0.0f,         //color
        // 1 - Vertex
        0.5f, -0.5f, 0.0f,   //bottom right vertex
        0.0f, 1.0f, 0.0f,         //color
        // 2 - Vertex
        -0.5f, 0.5f, 0.0f,   //top left vertex
        0.0f, 0.0f, 1.0f,         //color
        // 3 - Vertex
        0.5f, 0.5f, 0.0f,  //top right vertex
        0.0f, 0.0f, 1.0f,         //color
    };

    // Setting things up on GPU
    glGenVertexArrays(1, &mesh->m_VertexArrayObject);
    glBindVertexArray(mesh->m_VertexArrayObject);

    // Start generating our VBO ->for Position
    glGenBuffers(1, &mesh->m_VertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->m_VertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

    // Start EBO setup
    const std::vector<GLuint> elementBufferData {2,0,1, 3,2,1};  // vertices of triangle
    glGenBuffers(1, &mesh->m_ElementBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_ElementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementBufferData.size()*sizeof(GLuint), elementBufferData.data(),GL_STATIC_DRAW);

    //    vertex
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(GLfloat)*6, (void *)0);
    //    color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(GLfloat)*6, (void *)(sizeof(GLfloat)*3));

    // Unbind
    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void InitializeProgram(App *app){
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        ERROR_EXIT("SDL2 could not initialize video subsystem");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    app->m_GraphicsAppWindow = SDL_CreateWindow("OpenGL Window", 0, 0, app->SCREEN_WIDTH, app->SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    if(!app->m_GraphicsAppWindow)
        ERROR_EXIT("SDL_Window was not able to be created");

    app->m_OpenGLContext = (SDL_GLContext*)SDL_GL_CreateContext(app->m_GraphicsAppWindow);
    if(!app->m_OpenGLContext)
        ERROR_EXIT("OpenGL context not available");

    // for GL_VENDOR, GL_RENDERER, GL_VERSION
    gladLoadGL();
    puts("OPENGL Loaded");
    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version: %s\n", glGetString(GL_VERSION));
}

void Input(Mesh3D *mesh){
    //Lock mouse cursor on center of window
    static int mouseX = gApp.SCREEN_WIDTH/2;
    static int mouseY = gApp.SCREEN_HEIGHT/2;
        
    SDL_Event e;
    while(SDL_PollEvent(&e) != 0){
        if(e.type == SDL_QUIT){
            std::cout << "Goodbye!" << std::endl;
            gApp.m_Quit = true;
        }else if(e.type == SDL_MOUSEMOTION){
            mouseX = e.motion.xrel;
            mouseY = e.motion.yrel;
            gApp.m_Camera.MouseLook(mouseX, mouseY);
        }
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    // input key to move object
    if(state[SDL_SCANCODE_UP]){
        mesh->m_uOffset+=0.01f;
        cout << "g_uOffset: " << mesh->m_uOffset << endl;
    }
    if(state[SDL_SCANCODE_DOWN]){
        mesh->m_uOffset-=0.01f;
        cout << "g_uOffset: " << mesh->m_uOffset << endl;
    }
    if(state[SDL_SCANCODE_LEFT]){
        mesh->m_uRotate+=0.1f;
        cout << "g_uRotate: " << mesh->m_uRotate << endl;
    }
    if(state[SDL_SCANCODE_RIGHT]){
        mesh->m_uRotate=0.1f;
        cout << "g_uRotate: " << mesh->m_uRotate << endl;
    }
    // input key to move camera
    float speed = 0.01f;
    if(state[SDL_SCANCODE_W]){
        gApp.m_Camera.MoveForward(speed);
    }
    if(state[SDL_SCANCODE_S]){
        gApp.m_Camera.MoveBackward(speed);
    }
    if(state[SDL_SCANCODE_D]){
        gApp.m_Camera.MoveLeft(speed);
    }
    if(state[SDL_SCANCODE_A]){
        gApp.m_Camera.MoveRight(speed);
    }

    if(state[SDL_SCANCODE_ESCAPE]){
        gApp.m_Quit = true;
    }
}

void PreDraw(){
     glDisable(GL_DEPTH_TEST);
     glDisable(GL_CULL_FACE);

     glViewport(0, 0, gApp.SCREEN_WIDTH, gApp.SCREEN_HEIGHT);
     glClearColor(1.f, 1.f, 0.f, 1.f);

     glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

     glUseProgram(gApp.m_GraphicsPipelineShaderProgram);
     // for glsl use uniform -> pass to GPU usage (vert and frag variable from CPU)
     // GLint location = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_Offset");
     // if(location>=0){
     //     glUniform1f(location, g_uOffset);
     // } else {
     //     cout << "Could not find u_Offset, maybe misspelling?\n";
     // }

    // object matrix uniform values
    gMesh1.m_uRotate -=0.02f;
    // g_uOffset +=0.01f;
     // model transform -> translating our object into worldspace
     // rotate->translate (rotating at 0,0,0) then walk forward ※if camera is at 0,0 we can see that the object revolves at camera
     // translate->rotate (walkt at 0,0,0 forward) then rotate  ※if camera is at 0,0 we can see that the object spins at itself at a distance
     glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, gMesh1.m_uOffset));
     model           = glm::rotate(model, glm::radians(gMesh1.m_uRotate), glm::vec3(0.0f, 1.0f, 0.0f));
     model           = glm::scale(model, glm::vec3(gMesh1.m_uScale, gMesh1.m_uScale, gMesh1.m_uScale));
     GLint u_ModelMatrixLocation = glGetUniformLocation(gApp.m_GraphicsPipelineShaderProgram, "u_ModelMatrix");
     if(u_ModelMatrixLocation>=0){
         glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &model[0][0]);
     } else {
         cout << "Could not find u_ModelMatrix, maybe misspelling?\n";
     }
    
     glm::mat4 view = gApp.m_Camera.GetViewMatrix();
     GLint u_ViewLocation = glGetUniformLocation(gApp.m_GraphicsPipelineShaderProgram, "u_ViewMatrix");
     if(u_ViewLocation>=0){
         glUniformMatrix4fv(u_ViewLocation, 1, GL_FALSE, &view[0][0]);
     } else {
         cout << "Could not find u_View, maybe misspelling?\n";
     }

     // projection transform -> (this projection moves out object to z)
     glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)gApp.SCREEN_WIDTH/(float)gApp.SCREEN_HEIGHT, 0.1f, 100.0f);
     GLint u_ProjectionLocation = glGetUniformLocation(gApp.m_GraphicsPipelineShaderProgram, "u_Projection");
     if(u_ProjectionLocation>=0){
         glUniformMatrix4fv(u_ProjectionLocation, 1, GL_FALSE, &projection[0][0]);
     } else {
         cout << "Could not find u_Projection, maybe misspelling?\n";
     }


}

void Draw(){
    glBindVertexArray(gMesh1.m_VertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, gMesh1.m_VertexBufferObject);

    // glDrawArrays(GL_TRIANGLES, 0, 6);
    // GLCheck(glDrawElements(GL_TRIANGLES, 6, GL_INT, 0);) try error
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void MainLoop(){
    //Lock mouse cursor on center of window
    SDL_WarpMouseInWindow(gApp.m_GraphicsAppWindow, gApp.SCREEN_WIDTH/2, gApp.SCREEN_HEIGHT/2);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    while(!gApp.m_Quit){
        Input(&gMesh1);

        PreDraw();

        Draw();

        // Update the screen
        SDL_GL_SwapWindow(gApp.m_GraphicsAppWindow);
    }
}

void CleanUp(){
    SDL_DestroyWindow(gApp.m_GraphicsAppWindow);
    gApp.m_GraphicsAppWindow = nullptr;

    glDeleteBuffers(1, &gMesh1.m_VertexArrayObject);
    glDeleteVertexArrays(1, &gMesh2.m_VertexArrayObject);
    glDeleteProgram(gApp.m_GraphicsPipelineShaderProgram);

    SDL_Quit();
}

int main(){
    InitializeProgram(&gApp);

    VertexSpecification(&gMesh1);

    CreateGraphicsPipeline();

    MainLoop();

    CleanUp();

    return 0;
}
