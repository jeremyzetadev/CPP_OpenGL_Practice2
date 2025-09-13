#include <SDL2/SDL.h>
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

#define SCREEN_HEIGHT 480
#define SCREEN_WIDTH 640

// Globals
SDL_Window *gGraphicsAppWindow = nullptr;
SDL_GLContext *gOpenGLContext = nullptr;
bool gQuit = false;

// VAO
GLuint gVertexArrayObject = 0;
// VBO
GLuint gVertexBufferObject = 0;  //position + color
// EBO
GLuint gElementBufferObject = 0;

// ShaderGraphics
GLuint gGraphicsPipelineShaderProgram = 0;

// for glsl use uniform
float g_uOffset = 0.0f;

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
    gGraphicsPipelineShaderProgram = CreateShaderProgram("Shader/vert.glsl","Shader/frag.glsl");
}

// Multiple VBO
// void VertexSpecification_OLD(){
//     // Lives on the CPU
//     const vector<GLfloat> vertexPositions{
//         // x  y   z
//         -0.8f, -0.8f, 0.0f,  //vertex1
//         0.8f, -0.8f, 0.0f,   //vertex2
//         0.0f, 0.8f, 0.0f,     //vertex3
//     };
//     const vector<GLfloat> vertexColors{
//         // x  y   z
//         1.0f, 0.0f, 0.0f,  
//         0.0f, 1.0f, 0.0f,   
//         0.0f, 0.0f, 1.0f,   
//     };
//
//     // Setting things up on GPU
//     glGenVertexArrays(1, &gVertexArrayObject);
//     glBindVertexArray(gVertexArrayObject);
//
//     // Start generating our VBO ->for Position
//     glGenBuffers(1, &gVertexBufferObject);
//     glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
//     glBufferData(GL_ARRAY_BUFFER, vertexPositions.size() * sizeof(GLfloat), vertexPositions.data(), GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
//     // Start generating our VBO ->for Color
//     glGenBuffers(1, &gVertexBufferObject2);
//     glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject2);
//     glBufferData(GL_ARRAY_BUFFER, vertexColors.size() * sizeof(GLfloat), vertexColors.data(), GL_STATIC_DRAW);
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
//
//     // Unbind
//     glBindVertexArray(0);
//     glDisableVertexAttribArray(0);
//     glDisableVertexAttribArray(1);
// }

// Single VBO (position+color)
void VertexSpecification(){
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
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);

    // Start generating our VBO ->for Position
    glGenBuffers(1, &gVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

    // Start EBO setup
    const std::vector<GLuint> elementBufferData {2,0,1, 3,2,1};  // vertices of triangle
    glGenBuffers(1, &gElementBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementBufferData.size()*sizeof(GLuint), elementBufferData.data(),GL_STATIC_DRAW);

    //    vertex
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*6, (void *)0);
    //    color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*6, (void *)(sizeof(GL_FLOAT)*3));

    // Unbind
    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void InitializeProgram(){
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        ERROR_EXIT("SDL2 could not initialize video subsystem");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    gGraphicsAppWindow = SDL_CreateWindow("OpenGL Window", 0, 0,SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    if(!gGraphicsAppWindow)
        ERROR_EXIT("SDL_Window was not able to be created");

    gOpenGLContext = (SDL_GLContext*)SDL_GL_CreateContext(gGraphicsAppWindow);
    if(!gOpenGLContext)
        ERROR_EXIT("OpenGL context not available");

    // for GL_VENDOR, GL_RENDERER, GL_VERSION
    gladLoadGL();
    puts("OPENGL Loaded");
    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version: %s\n", glGetString(GL_VERSION));
}

void Input(){
    SDL_Event e;
    while(SDL_PollEvent(&e) != 0){
        if(e.type == SDL_QUIT){
            std::cout << "Goodbye!" << std::endl;
            gQuit = true;
        }
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if(state[SDL_SCANCODE_UP]){
        g_uOffset+=0.01f;
        cout << "g_uOffset: " << g_uOffset << endl;
    }
    if(state[SDL_SCANCODE_DOWN]){
        g_uOffset-=0.01f;
        cout << "g_uOffset: " << g_uOffset << endl;
    }
}

void PreDraw(){
     glDisable(GL_DEPTH_TEST);
     glDisable(GL_CULL_FACE);

     glViewport(0,0,SCREEN_WIDTH, SCREEN_HEIGHT);
     glClearColor(1.f, 1.f, 0.f, 1.f);

     glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

     glUseProgram(gGraphicsPipelineShaderProgram);
     GLint location = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_Offset");
     // for glsl use uniform -> pass to GPU usage (vert and frag variable from CPU)
     if(location>=0){
         glUniform1f(location, g_uOffset);
     } else {
         cout << "Could not find u_Offset, maybe misspelling?\n";
     }
}

void Draw(){
    glBindVertexArray(gVertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

    // glDrawArrays(GL_TRIANGLES, 0, 6);
    // GLCheck(glDrawElements(GL_TRIANGLES, 6, GL_INT, 0);) try error
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void MainLoop(){
    while(!gQuit){
        Input();

        PreDraw();

        Draw();

        // Update the screen
        SDL_GL_SwapWindow(gGraphicsAppWindow);
    }
}

void CleanUp(){
    SDL_DestroyWindow(gGraphicsAppWindow);
    SDL_Quit();
}

int main(){
    InitializeProgram();

    VertexSpecification();

    CreateGraphicsPipeline();

    MainLoop();

    CleanUp();

    return 0;
}
