#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Shader.h>
#include <iostream>
#include <fstream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void animateCar(glm::mat4* model);
glm::vec3 computeNormal(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);
void processVerticesWithNormals(const float* vertices, int numVertices, float* verticesWithNormals);
glm::vec3 carControlledPosition = glm::vec3(0.0f);

glm::vec3 cameraPos   = glm::vec3(5.0f, 2.0f,  5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const bool animate = true;

glm::vec3 carPosition = glm::vec3(1.0f);
float carRotation = 0;

/* Camera */
float sensitivity = 1.0f;
float yaw = -90.0f;
float pitch = 0.0f;
float zoom = 45.0f;
void reset() {
    cameraPos   = glm::vec3(4.0f, 0.0f,  34.0f);
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

    sensitivity = 1.0f;
    yaw = -90.0f;
    pitch = 0.0f;
    zoom = 45.0f;
}

void zoomControl(float z) {
    zoom += z;
    if (zoom <1.0f)
        zoom = 1.0f;
    if (zoom> 100.0f)
        zoom = 100.0f;
}

void viraCamera(float x, float y) {
        yaw   += x * sensitivity;
        pitch += y * sensitivity;

        if(pitch > 89.0f)
            pitch = 89.0f;
        if(pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);
}

// Funções para compilar shaders
GLuint compileShader(const char* vertexSource, const char* fragmentSource);
void checkShaderCompileStatus(GLuint shader);
void checkProgramLinkStatus(GLuint program);
// Processar eventos de teclado
void processInput(GLFWwindow *window, float *x, float *y, float *z);

// Shaders (cada objeto terá um shader próprio)
const char* floorFragmentShader = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "	FragColor = texture(texture1, TexCoord);\n"
    "}\n"
;

const char* floorVertexShader = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "	gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
    "	TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
    "}\n";

// Car
const char* carFragmentShader = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "	FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n"
;

const char* carVertexShader = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 normal;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "	gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
    "}\n"
;

// Lamp
const char* lampFragmentShader = R"(
    #version 330 core
    out vec4 FragColor;

    uniform vec3 lampColor;

    void main()
    {
        FragColor = vec4(lampColor, 1.0);
    }
)";

const char* lampVertexShader = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "	gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
    "}\n"
;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "TDE2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glew: load all OpenGL function pointers
    // ---------------------------------------
    if(glewInit()!=GLEW_OK) {
        std::cout << "Ocorreu um erro iniciando GLEW!" << std::endl;
    } else {
        std::cout << "GLEW OK!" << std::endl;
        std::cout << glGetString(GL_VERSION) << std::endl;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    GLuint floorShaderProgram = compileShader(floorVertexShader, floorFragmentShader);
    GLuint carShaderProgram = compileShader(carVertexShader, carFragmentShader);
    GLuint lampShaderProgram = compileShader(lampVertexShader, lampFragmentShader);



    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // Triangle 1
        -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,

        // Triangle 2
        -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f
    };

    float carVertices[] = {
        // body triangle 1 front
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        -1.0f, -0.2f, 1.0f, 0.0f, 0.0f, -1.0f,
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f, -1.0f,

        // body triangle 2 front
        -1.0f, -0.2f, 1.0f, 0.0f, 0.0f, -1.0f,
        1.0f, -0.2f, 1.0f, 0.0f, 0.0f, -1.0f,
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f, -1.0f,

        // window triangle 1 front
        -0.7f, -0.2f, 1.0f, 0.0f, 0.0f, -1.0f,
        -0.7f, 0.75f, 1.0f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.2f, 1.0f, 0.0f, 0.0f, -1.0f,

        // window triangle 2 front
        -0.7f, 0.75f, 1.0f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.75f, 1.0f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.2f, 1.0f, 0.0f, 0.0f, -1.0f,

        // Upper part
        // body triangle 1 upper
        -1.0f, -0.2f, 1.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -0.2f, -1.0f, 0.0f, -1.0f, 0.0f,
        -0.7f, -0.2f, -1.0f, 0.0f, -1.0f, 0.0f,

        // body triangle 2 upper
        -1.0f, -0.2f, 1.0f, 0.0f, -1.0f, 0.0f,
        -0.7f, -0.2f, -1.0f, 0.0f, -1.0f, 0.0f,
        -0.7f, -0.2f, 1.0f, 0.0f, -1.0f, 0.0f,

        // ceiling triangle 1 upper
        -0.7f, 0.75f, 1.0f, 0.0f, -1.0f, 0.0f,
        -0.7f, 0.75f, -1.0f, 0.0f, -1.0f, 0.0f,
        0.5f, 0.75f, -1.0f, 0.0f, -1.0f, 0.0f,

        // ceiling triangle 2 upper
        -0.7f, 0.75f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.5f, 0.75f, -1.0f, 0.0f, -1.0f, 0.0f,
        0.5f, 0.75f, 1.0f, 0.0f, -1.0f, 0.0f,

        // front triangle 1 upper
        0.5f, -0.2f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.2f, -1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, -0.2f, -1.0f, 0.0f, -1.0f, 0.0f,

        // front triangle 2 upper
        0.5f, -0.2f, 1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, -0.2f, -1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, -0.2f, 1.0f, 0.0f, -1.0f, 0.0f,

        // Front
        // bottom triangle 1 front
        -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, -0.2f, -1.0f, 1.0f, 0.0f, 0.0f,

        // bottom triangle 2 front
        -1.0f, -0.2f, -1.0f, 1.0f, 0.0f, -0.0f,
        -1.0f, -0.2f, 1.0f, 1.0f, 0.0f, -0.0f,
        -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, -0.0f,

        // upper triangle 1 front
        -0.7f, -0.2f, -1.0f, 1.0f, 0.0f, 0.0f,
        -0.7f, 0.75f, -1.0f, 1.0f, 0.0f, 0.0f,
        -0.7f, -0.2f, 1.0f, 1.0f, 0.0f, 0.0f,

        // upper triangle 2 front
        -0.7f, 0.75f, -1.0f, 1.0f, 0.0f, -0.0f,
        -0.7f, 0.75f, 1.0f, 1.0f, 0.0f, -0.0f,
        -0.7f, -0.2f, 1.0f, 1.0f, 0.0f, -0.0f,

        // Back triangle 1
        1.0f, -1.0f, -1.0f, 0.0f, -0.0f, 1.0f,
        1.0f, -0.2f, -1.0f, 0.0f, -0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, -0.0f, 1.0f,

        // Back triangle 2
        1.0f, -0.2f, -1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -0.2f, -1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        // Bottom triangle 1
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f,

        // Bottom triangle 2
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f,

        // Right side triangle 1
        1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -0.2f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,

        // Right side triangle 2
        1.0f, -0.2f, 1.0f, -1.0f, 0.0f, -0.0f,
        1.0f, -0.2f, -1.0f, -1.0f, 0.0f, -0.0f,
        1.0f, -1.0f, -1.0f, -1.0f, 0.0f, -0.0f,

        // Left side triangle 1
        -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, -0.2f, 1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,

        // Left side triangle 2
        -1.0f, -0.2f, 1.0f, -1.0f, 0.0f, -0.0f,
        -1.0f, -0.2f, -1.0f, -1.0f, 0.0f, -0.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, -0.0f,

        // Rear window triangle 1
        -0.7f, -0.2f, -1.0f, 0.0f, 0.0f, -1.0f,
        -0.7f, 0.75f, -1.0f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.2f, -1.0f, 0.0f, 0.0f, -1.0f,

        // Rear window triangle 2
        -0.7f, 0.75f, -1.0f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.75f, -1.0f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.2f, -1.0f, 0.0f, 0.0f, -1.0f,

        // upper triangle 1 front
        0.5f, -0.2f, -1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.75f, -1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.2f, 1.0f, 1.0f, 0.0f, 0.0f,

        // upper triangle 2 front
        0.5f, 0.75f, -1.0f, 1.0f, 0.0f, -0.0f,
        0.5f, 0.75f, 1.0f, 1.0f, 0.0f, -0.0f,
        0.5f, -0.2f, 1.0f, 1.0f, 0.0f, -0.0f,
    };

    float lampVertices[] = {
        //poste
        -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.7f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        -0.7f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.7f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,

        // curvatura
        -1.0f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.7f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.7f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,

        -0.7f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.7f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,

        -0.7f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.3f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,

        //lateral esquerda
        -1.0f, 0.0f, -0.5f, 0.0f, 0.0f, 1.0f,
        -1.0f, 1.7f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.7f, 0.0f, -0.5f, 0.0f, 0.0f, 1.0f,

        -0.7f, 0.0f, -0.5f, 0.0f, 0.0f, 1.0f,
        -1.0f, 1.7f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.7f, 1.7f, -0.5f, 0.0f, 0.0f, 1.0f,

        -1.0f, 1.7f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.7f, 2.0f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.7f, 1.7f, -0.5f, 0.0f, 0.0f, 1.0f,

        -0.7f, 1.7f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.7f, 2.0f, -0.5f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.7f, -0.5f, 0.0f, 0.0f, 1.0f,

        -0.7f, 2.0f, -0.5f, 0.0f, 0.0f, 1.0f,
        0.0f, 2.0f, -0.5f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.7f, -0.5f, 0.0f, 0.0f, 1.0f,

        0.0f, 1.7f, -0.5f, 0.0f, 0.0f, 1.0f,
        0.0f, 2.0f, -0.5f, 0.0f, 0.0f, 1.0f,
        0.3f, 1.7f, -0.5f, 0.0f, 0.0f, 1.0f,

        //vista de cima
        -1.0f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.7f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.7f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,

        -0.7f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.7f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,

        -0.7f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.3f, 1.7f, 1.0f, 0.0f, 0.0f, 1.0f,

        // Parte superior do poste
        // Face frontal
        -1.0f, 1.7f, 1.0f, -0.707107f, 0.707107f, 0.0f,
        -1.0f, 1.7f, -0.5f, -0.707107f, 0.707107f, 0.0f,
        -0.7f, 2.0f, 1.0f, -0.707107f, 0.707107f, 0.0f,

        -0.7f, 2.0f, 1.0f, -0.707107f, 0.707107f, 0.0f,
        -1.0f, 1.7f, -0.5f, -0.707107f, 0.707107f, 0.0f,
        -0.7f, 2.0f, -0.5f, -0.707107f, 0.707107f, 0.0f,

        // Face traseira
        -0.7f, 2.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.7f, 2.0f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.0f, 2.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 2.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.7f, 2.0f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.0f, 2.0f, -0.5f, 0.0f, 1.0f, 0.0f,

        // Face direita
        0.0f, 2.0f, 1.0f, 0.707107f, 0.707107f, 0.0f,
        0.0f, 2.0f, -0.5f, 0.707107f, 0.707107f, 0.0f,
        0.3f, 1.7f, 1.0f, 0.707107f, 0.707107f, 0.0f,

        0.3f, 1.7f, 1.0f, 0.707107f, 0.707107f, 0.0f,
        0.0f, 2.0f, -0.5f, 0.707107f, 0.707107f, 0.0f,
        0.3f, 1.7f, -0.5f, 0.707107f, 0.707107f, 0.0f,

        // Parte inferior (fechando a base)
        -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.7f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        -0.7f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.7f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f,

        // Parte traseira do poste (fechando o comprimento)
        // Face traseira principal
        -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, 1.7f, 1.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, -0.5f, 1.0f, 0.0f, 0.0f,

        -1.0f, 1.7f, 1.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, 1.7f, -0.5f, 1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, -0.5f, 1.0f, 0.0f, 0.0f,

        // Face traseira superior (curvatura)
        -1.0f, 1.7f, 1.0f, -0.707107f, 0.707107f, 0.0f,
        -1.0f, 1.7f, -0.5f, -0.707107f, 0.707107f, 0.0f,
        -0.7f, 2.0f, 1.0f, -0.707107f, 0.707107f, 0.0f,

        -0.7f, 2.0f, 1.0f, -0.707107f, 0.707107f, 0.0f,
        -1.0f, 1.7f, -0.5f, -0.707107f, 0.707107f, 0.0f,
        -0.7f, 2.0f, -0.5f, -0.707107f, 0.707107f, 0.0f,

        // Face frontal (fechando a parte da frente)
        0.3f, 1.7f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.3f, 1.7f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.7f, 1.7f, 1.0f, 0.0f, 1.0f, 0.0f,

        -0.7f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.7f, 1.7f, -0.5f, 1.0f, 0.0f, 0.0f,
        -0.7f, 0.0f, -0.5f, 1.0f, 0.0f, 0.0f,
    };

    unsigned int VBOs[3], VAOs[3];
    glGenVertexArrays(3, VAOs);
    glGenBuffers(3, VBOs);

    // Floor
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Car
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(carVertices), carVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Lamp
    glBindVertexArray(VAOs[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), lampVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load and create a texture
    // -------------------------
    unsigned int texture1;
    // texture 1
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load("res/images/pista.jpeg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    glUseProgram(floorShaderProgram);
    glUniform1i(glGetUniformLocation(floorShaderProgram, "texture1"), 0); // ourShader.setInt("texture1", 0);


    viraCamera(-50.0f, -20.0f);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        // activate shader
        glUseProgram(floorShaderProgram);

        // create transformations
        glm::mat4 model         = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);

        // model = glm::rotate(model, 0.2f, glm::vec3(0.0f, 1.0f, 0.0f));
        // model = glm::rotate(model, 0.75f, glm::vec3(0.0f, 1.0f, 0.0f));
        // model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, -0.5f, 0.0f)); // Com movimentação (ajuda pra debug)
        model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 4.0f));

        view  = glm::translate(view, glm::vec3(0.05f, -0.25f, -3.0f));
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // retrieve the matrix uniform locations
        unsigned int modelLoc = glGetUniformLocation(floorShaderProgram, "model");
        unsigned int viewLoc  = glGetUniformLocation(floorShaderProgram, "view");
        unsigned int projectionLoc  = glGetUniformLocation(floorShaderProgram, "projection");
        // pass them to the shaders (3 different ways)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


        // Car
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(carShaderProgram);

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.2f));
        model = glm::translate(model, glm::vec3(3.5f, -2.5f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, carControlledPosition);
        glUniformMatrix4fv(glGetUniformLocation(carShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(carShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(carShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(VAOs[1]);
        glDrawArrays(GL_TRIANGLES, 0, 81);

        // Lamp
        glm::vec3 lightPos(1.6f, 0.5f, 0.0f); // Lamp position
        glm::vec3 lampColor(0.4196f, 0.1255f, 0.0667f); // Light color (Brown-ish)
        glUseProgram(lampShaderProgram);

        glUniform3fv(glGetUniformLocation(lampShaderProgram, "lampColor"), 1, glm::value_ptr(lampColor));

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.5f));
        model = glm::translate(model, glm::vec3(1.6f, -1.0f, 0.0f));
        //model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lampShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lampShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lampShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(VAOs[2]);
        glDrawArrays(GL_TRIANGLES, 0, 90);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

float velocityMultiplier = 1;
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    const float cameraSpeed = 0.05f; // adjust accordingly

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPos += glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        cameraPos += glm::vec3(0.0f, -1.0f, 0.0f) * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        zoomControl(1.0f);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        zoomControl(-1.0f);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        reset();
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// Funções auxiliares
GLuint compileShader(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompileStatus(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompileStatus(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkProgramLinkStatus(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void checkShaderCompileStatus(GLuint shader) {
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Erro de compilação de shader: %s\n", infoLog);
    }
}

void checkProgramLinkStatus(GLuint program) {
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "Erro ao linkar programa: %s\n", infoLog);
    }
}

glm::vec3 computeNormal(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 normal = glm::cross(edge1, edge2);
    return glm::normalize(normal);
}

void processVerticesWithNormals(const float* vertices, int numVertices, float* verticesWithNormals) {
    const int numTriangles = numVertices / 3;

    for (int i = 0; i < numTriangles; ++i) {
        int idx0 = i * 9;
        int idx1 = idx0 + 3;
        int idx2 = idx0 + 6;

        glm::vec3 v0(vertices[idx0], vertices[idx0 + 1], vertices[idx0 + 2]);
        glm::vec3 v1(vertices[idx1], vertices[idx1 + 1], vertices[idx1 + 2]);
        glm::vec3 v2(vertices[idx2], vertices[idx2 + 1], vertices[idx2 + 2]);

        glm::vec3 normal = computeNormal(v0, v1, v2);

        auto addVertexWithNormal = [&](int vertexIndex, int outputIndex) {
            verticesWithNormals[outputIndex]     = vertices[vertexIndex];
            verticesWithNormals[outputIndex + 1] = vertices[vertexIndex + 1];
            verticesWithNormals[outputIndex + 2] = vertices[vertexIndex + 2];
            verticesWithNormals[outputIndex + 3] = normal.x;
            verticesWithNormals[outputIndex + 4] = normal.y;
            verticesWithNormals[outputIndex + 5] = normal.z;
        };

        addVertexWithNormal(idx0, i * 18);
        addVertexWithNormal(idx1, i * 18 + 6);
        addVertexWithNormal(idx2, i * 18 + 12);
    }
}
