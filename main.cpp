#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <C:\Users\55549\local\Documentos\UCS\computacao_grafica\race-circuit-open-gl\include\stb_image.h>
#include <C:\glm\glm\glm.hpp>
#include <C:\glm\glm\gtc\matrix_transform.hpp>
#include <C:\glm\glm\gtc\type_ptr.hpp>
#include <C:\Users\55549\local\Documentos\UCS\computacao_grafica\race-circuit-open-gl\include\Shader.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void animateCar(glm::mat4* model);

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
//const char* floorFragmentShader = "#version 330 core\n"
//    "out vec4 FragColor;\n"
//    "in vec2 TexCoord;\n"
//    "uniform sampler2D texture1;\n"
//    "void main()\n"
//    "{\n"
//    "	FragColor = texture(texture1, TexCoord);\n"
//    "}\n"
//;

//const char* floorFragmentShader = R"(
//    #version 330 core
//    out vec4 FragColor;
//
//    in vec3 FragPos;
//    in vec3 Normal;
//    in vec2 TexCoord;
//
//    uniform sampler2D texture1;
//    uniform vec3 lightPos;
//    uniform vec3 lightColor;
//    uniform vec3 viewPos;
//
//    void main()
//    {
//        // Ambiente
//        float ambientStrength = 0.1;
//        vec3 ambient = ambientStrength * lightColor;
//
//        // Difuso
//        vec3 norm = normalize(Normal);
//        vec3 lightDir = normalize(lightPos - FragPos);
//        float diff = max(dot(norm, lightDir), 0.0);
//        vec3 diffuse = diff * lightColor;
//
//        // Especular
//        float specularStrength = 0.5;
//        vec3 viewDir = normalize(viewPos - FragPos);
//        vec3 reflectDir = reflect(-lightDir, norm);
//        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
//        vec3 specular = specularStrength * spec * lightColor;
//
//        vec3 result = (ambient + diffuse + specular) * texture(texture1, TexCoord).rgb;
//        FragColor = vec4(result, 1.0);
//    }
//)";

const char* floorFragmentShader = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoord;

    uniform sampler2D texture1;
    uniform vec3 lightPos;
    uniform vec3 lightColor;
    uniform vec3 viewPos;

    void main()
    {
        // Textura
        vec3 texColor = texture(texture1, TexCoord).rgb;

        // Ambiente
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * lightColor;

        // Difuso
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Especular
        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor;

        vec3 result = (ambient + diffuse + specular) * texColor;
        FragColor = vec4(result, 1.0);
    }
)";
//const char* floorVertexShader = "#version 330 core\n"
//    "layout (location = 0) in vec3 aPos;\n"
//    "layout (location = 1) in vec2 aTexCoord;\n"
//    "out vec2 TexCoord;\n"
//    "uniform mat4 model;\n"
//    "uniform mat4 view;\n"
//    "uniform mat4 projection;\n"
//    "void main()\n"
//    "{\n"
//    "	gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
//    "	TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
//    "}\n";

const char* floorVertexShader = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoord;

    out vec3 FragPos;
    out vec3 Normal;
    out vec2 TexCoord;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        TexCoord = aTexCoord;

        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)";

//const char* carFragmentShader = "#version 330 core\n"
//    "out vec4 FragColor;\n"
//    "void main()\n"
//    "{\n"
//    "	FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
//    "}\n"
//;

//const char* carFragmentShader = R"(
//    #version 330 core
//    out vec4 FragColor;
//
//    in vec3 FragPos;
//    in vec3 Normal;
//
//    uniform vec3 lightPos;
//    uniform vec3 lightColor;
//    uniform vec3 viewPos;
//
//    void main()
//    {
//        // Cor base do carro (vermelho)
//        vec3 objectColor = vec3(1.0, 0.0, 0.0);
//
//        // Ambiente
//        float ambientStrength = 0.1;
//        vec3 ambient = ambientStrength * lightColor;
//
//        // Difuso
//        vec3 norm = normalize(Normal);
//        vec3 lightDir = normalize(lightPos - FragPos);
//        float diff = max(dot(norm, lightDir), 0.0);
//        vec3 diffuse = diff * lightColor;
//
//        // Especular
//        float specularStrength = 0.5;
//        vec3 viewDir = normalize(viewPos - FragPos);
//        vec3 reflectDir = reflect(-lightDir, norm);
//        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
//        vec3 specular = specularStrength * spec * lightColor;
//
//        vec3 result = (ambient + diffuse + specular) * objectColor;
//        FragColor = vec4(result, 1.0);
//    }
//)";

const char* carFragmentShader = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;

    uniform vec3 lightPos;
    uniform vec3 lightColor;
    uniform vec3 viewPos;

    void main()
    {
        // Cor base do carro (vermelho)
        vec3 objectColor = vec3(1.0, 0.0, 0.0);

        // Ambiente
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * lightColor;

        // Difuso
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Especular
        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor;

        vec3 result = (ambient + diffuse + specular) * objectColor;
        FragColor = vec4(result, 1.0);
    }
)";

//const char* carVertexShader = "#version 330 core\n"
//    "layout (location = 0) in vec3 aPos;\n"
//    "uniform mat4 model;\n"
//    "uniform mat4 view;\n"
//    "uniform mat4 projection;\n"
//    "void main()\n"
//    "{\n"
//    "	gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
//    "}\n"
//;

const char* carVertexShader = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;

    out vec3 FragPos;
    out vec3 Normal;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;

        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)";

//const char* lampFragmentShader = "#version 330 core\n"
//    "out vec4 FragColor;\n"
//    "void main()\n"
//    "{\n"
//    "    FragColor = vec4(0.5, 0.5, 0.5, 1.0);\n"
//    "}\n"
//;

const char* lampFragmentShader = R"(
    #version 330 core
    out vec4 FragColor;

    uniform vec3 lightColor;

    void main()
    {
        FragColor = vec4(lightColor, 1.0);
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
        -1.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        -1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    };

   float carVertices[] = {
        // body triangle 1 front
        -1.0f, -1.0f, 1.0f,
        -1.0f, -0.2f, 1.0f,
        1.0f, -1.0f, 1.0f,

        // body triangle 2 front
        -1.0f, -0.2f, 1.0f,
        1.0f, -0.2f, 1.0f,
        1.0f, -1.0f, 1.0f,

        // window triangle 1 front
        -0.7f, -0.2f, 1.0f,
        -0.7f, 0.75f, 1.0f,
        0.5f, -0.2f, 1.0f,

        // window triangle 2 front
        -0.7f, 0.75f, 1.0f,
        0.5f, 0.75f, 1.0f,
        0.5f, -0.2f, 1.0f,

        // Upper part
        // body triangle 1 upper
        -1.0f, -0.2f, 1.0f,
        -1.0f, -0.2f, -1.0f,
        -0.7f, -0.2f, -1.0f,

        // body triangle 2 upper
        -1.0f, -0.2f, 1.0f,
        -0.7f, -0.2f, -1.0f,
        -0.7f, -0.2f, 1.0f,

        // ceiling triangle 1 upper
        -0.7f, 0.75f, 1.0f,
        -0.7f, 0.75f, -1.0f,
        0.5f, 0.75f, -1.0f,

        // ceiling triangle 2 upper
        -0.7f, 0.75f, 1.0f,
        0.5f, 0.75f, -1.0f,
        0.5f, 0.75f, 1.0f,

        // front triangle 1 upper
        0.5f, -0.2f, 1.0f,
        0.5f, -0.2f, -1.0f,
        1.0f, -0.2f, -1.0f,

        // front triangle 2 upper
        0.5f, -0.2f, 1.0f,
        1.0f, -0.2f, -1.0f,
        1.0f, -0.2f, 1.0f,

        // Front
        // bottom triangle 1 front
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -0.2f, -1.0f,

        // bottom triangle 2 front
        -1.0f, -0.2f, -1.0f,
        -1.0f, -0.2f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        // upper triangle 1 front
        -0.7f, -0.2f, -1.0f,
        -0.7f, 0.75f, -1.0f,
        -0.7f, -0.2f, 1.0f,

        // upper triangle 2 front
        -0.7f, 0.75f, -1.0f,
        -0.7f, 0.75f, 1.0f,
        -0.7f, -0.2f, 1.0f,

        // Back triangle 1
        1.0f, -1.0f, -1.0f,
        1.0f, -0.2f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        // Back triangle 2
        1.0f, -0.2f, -1.0f,
        -1.0f, -0.2f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        // Bottom triangle 1
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,

        // Bottom triangle 2
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        // Right side triangle 1
        1.0f, -1.0f, 1.0f,
        1.0f, -0.2f, 1.0f,
        1.0f, -1.0f, -1.0f,

        // Right side triangle 2
        1.0f, -0.2f, 1.0f,
        1.0f, -0.2f, -1.0f,
        1.0f, -1.0f, -1.0f,

        // Left side triangle 1
        -1.0f, -1.0f, 1.0f,
        -1.0f, -0.2f, 1.0f,
        -1.0f, -1.0f, -1.0f,

        // Left side triangle 2
        -1.0f, -0.2f, 1.0f,
        -1.0f, -0.2f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        // Rear window triangle 1
        -0.7f, -0.2f, -1.0f,
        -0.7f, 0.75f, -1.0f,
        0.5f, -0.2f, -1.0f,

        // Rear window triangle 2
        -0.7f, 0.75f, -1.0f,
        0.5f, 0.75f, -1.0f,
        0.5f, -0.2f, -1.0f,

        // upper triangle 1 front
        0.5f, -0.2f, -1.0f,
        0.5f, 0.75f, -1.0f,
        0.5f, -0.2f, 1.0f,

        // upper triangle 2 front
        0.5f, 0.75f, -1.0f,
        0.5f, 0.75f, 1.0f,
        0.5f, -0.2f, 1.0f,
    };

    float lamp_vertices[] = {

//        -2.0f, -2.0f, -2.0f,
//        -2.0f, -1.0f, -2.0f,
//        -1.5f, -2.0f, -2.0f,
//
//        -2.0f, -2.0f, -2.0f,
//        -2.0f, -1.0f, -2.0f,
//        -1.5f, -1.0f, -2.0f,

//        -2.0f, -1.0f, -2.0f,
//        -2.0f, 0.0f, -2.0f,
//        -1.5f, -1.0f, -2.0f,
//
//        -1.5f, -1.0f, -2.0f,
//        -2.0f, 0.0f, -2.0f,
//        -1.5f, 0.0f, -2.0f,
//
//        -2.0f, 0.0f, -2.0f,
//        -1.5f, 1.0f, -2.0f,
//        -1.5f, 0.0f, -2.0f,
//
//        -2.0f, 0.0f, -2.0f,
//        -2.0f, 1.0f, -2.0f,
//        -1.5f, 1.0f, -2.0f,

        //poste
        -1.0f, 0.0f, -1.0f,
        -1.0f, 1.7f, -1.0f,
        -0.7f, 0.0f, -1.0f,

        -0.7f, 0.0f, -1.0f,
        -1.0f, 1.7f, -1.0f,
        -0.7f, 1.7f, -1.0f,

        // curvatura
        -1.0f, 1.7f, -1.0f,
        -0.7f, 2.0f, -1.0f,
        -0.7f, 1.7f, -1.0f,

        -0.7f, 1.7f, -1.0f,
        -0.7f, 2.0f, -1.0f,
        0.0f, 1.7f, -1.0f,

        -0.7f, 2.0f, -1.0f,
        0.0f, 2.0f, -1.0f,
        0.0f, 1.7f, -1.0f,

        0.0f, 1.7f, -1.0f,
        0.0f, 2.0f, -1.0f,
        0.3f, 1.7f, -1.0f,

        //lateral esquerda
        -1.0f, 0.0f, -0.5f,
        -1.0f, 1.7f, -0.5f,
        -0.7f, 0.0f, -0.5f,

        -0.7f, 0.0f, -0.5f,
        -1.0f, 1.7f, -0.5f,
        -0.7f, 1.7f, -0.5f,

        -1.0f, 1.7f, -0.5f,
        -0.7f, 2.0f, -0.5f,
        -0.7f, 1.7f, -0.5f,

        -0.7f, 1.7f, -0.5f,
        -0.7f, 2.0f, -0.5f,
        0.0f, 1.7f, -0.5f,

        -0.7f, 2.0f, -0.5f,
        0.0f, 2.0f, -0.5f,
        0.0f, 1.7f, -0.5f,

        0.0f, 1.7f, -0.5f,
        0.0f, 2.0f, -0.5f,
        0.3f, 1.7f, -0.5f,

        //vista de cima

        -1.0f, 1.7f, -1.0f,
        -0.7f, 2.0f, -1.0f,
        -0.7f, 1.7f, -1.0f,

        -0.7f, 1.7f, -1.0f,
        -0.7f, 2.0f, -1.0f,
        0.0f, 1.7f, -1.0f,

        -0.7f, 2.0f, -1.0f,
        0.0f, 2.0f, -1.0f,
        0.0f, 1.7f, -1.0f,

        0.0f, 1.7f, -1.0f,
        0.0f, 2.0f, -1.0f,
        0.3f, 1.7f, -1.0f,

        // Parte superior do poste
        // Face frontal
        -1.0f, 1.7f, -1.0f,
        -1.0f, 1.7f, -0.5f,
        -0.7f, 2.0f, -1.0f,

        -0.7f, 2.0f, -1.0f,
        -1.0f, 1.7f, -0.5f,
        -0.7f, 2.0f, -0.5f,

        // Face traseira
        -0.7f, 2.0f, -1.0f,
        -0.7f, 2.0f, -0.5f,
        0.0f, 2.0f, -1.0f,

        0.0f, 2.0f, -1.0f,
        -0.7f, 2.0f, -0.5f,
        0.0f, 2.0f, -0.5f,

        // Face direita
        0.0f, 2.0f, -1.0f,
        0.0f, 2.0f, -0.5f,
        0.3f, 1.7f, -1.0f,

        0.3f, 1.7f, -1.0f,
        0.0f, 2.0f, -0.5f,
        0.3f, 1.7f, -0.5f,

        // Parte inferior (fechando a base)
        -1.0f, 0.0f, -1.0f,
        -1.0f, 0.0f, -0.5f,
        -0.7f, 0.0f, -1.0f,

        -0.7f, 0.0f, -1.0f,
        -1.0f, 0.0f, -0.5f,
        -0.7f, 0.0f, -0.5f,

        // Parte traseira do poste (fechando o comprimento)
        // Face traseira principal
        -1.0f, 0.0f, -1.0f,
        -1.0f, 1.7f, -1.0f,
        -1.0f, 0.0f, -0.5f,

        -1.0f, 1.7f, -1.0f,
        -1.0f, 1.7f, -0.5f,
        -1.0f, 0.0f, -0.5f,

        // Face traseira superior (curvatura)
        -1.0f, 1.7f, -1.0f,
        -1.0f, 1.7f, -0.5f,
        -0.7f, 2.0f, -1.0f,

        -0.7f, 2.0f, -1.0f,
        -1.0f, 1.7f, -0.5f,
        -0.7f, 2.0f, -0.5f,

        // Face frontal (fechando a parte da frente)
        0.3f, 1.7f, -1.0f,
        0.3f, 1.7f, -0.5f,
        -0.7f, 1.7f, -1.0f,

        -0.7f, 0.0f, -1.0f,
        -0.7f, 1.7f, -0.5f,
        -0.7f, 0.0f, -0.5f,





    };

    unsigned int VBOs[3], VAOs[3];
    glGenVertexArrays(3, VAOs);
    glGenBuffers(3, VBOs);

    // Floor
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Car
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(carVertices), carVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Lamp
    glBindVertexArray(VAOs[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lamp_vertices), lamp_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


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
        glm::vec3 lightPos(1.6f, 0.5f, 0.0f); // Posição do poste
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // Cor da luz (branca)

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
        glUniform3fv(glGetUniformLocation(floorShaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(floorShaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
        //glUniform3fv(glGetUniformLocation(floorShaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));




        // Car
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(carShaderProgram);
        glUniform3fv(glGetUniformLocation(carShaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(carShaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
        glUniform3fv(glGetUniformLocation(carShaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.2f));
        model = glm::translate(model, glm::vec3(3.5f, -2.5f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        animateCar(&model);
        glUniformMatrix4fv(glGetUniformLocation(carShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(carShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(carShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(VAOs[1]);
        glDrawArrays(GL_TRIANGLES, 0, 81);

        // Lamp

        //glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(lampShaderProgram);

        glUniform3fv(glGetUniformLocation(lampShaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));

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
    glDeleteVertexArrays(3, VAOs);
    glDeleteBuffers(3, VBOs);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

float velocityMultiplier = 0.08;
void animateCar(glm::mat4* model) {
    if (!animate) return;
    static int lap = 0;

    float clock = ((float) glfwGetTime()) - (14.2 * lap);
    if (clock < 2.2) {
        carPosition.x -= 0.03 * velocityMultiplier;
    } else if (clock < 5.5) {
        carPosition.z -= 0.02 * velocityMultiplier;
        float c = (clock - 2.2) * 6;
        float xVelocity = (-0.01*c*c+0.2*c-1)/33.33;
        if (c >= 10) {
            xVelocity *= -1;
        }
        carPosition.x += xVelocity * velocityMultiplier;
        carRotation = -180 * (clock-2.2)/(5.5-2.2);
    } else if (clock < 9.3) {
        carPosition.x += 0.03 * velocityMultiplier;
    } else if (clock < 12.6) {
        carPosition.z += 0.02  * velocityMultiplier;
        float c = (clock - 9.3) * 6;
        float xVelocity = (0.01*c*c-0.2*c+1)/33.33;
        if (c >= 10) {
            xVelocity *= -1;
        }
        carPosition.x += xVelocity * velocityMultiplier;
        carRotation = -180 - (180 * (clock-9.3)/(12.6-9.3));
    } else if (clock < 14.2) {
        carPosition.x -= 0.03 * velocityMultiplier;
    } else {
        lap++;
    }
    *model = glm::translate(*model, carPosition);
    *model = glm::rotate(*model, glm::radians(carRotation), glm::vec3(0.0f, 0.1f, 0.0f));
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    const float cameraSpeed = 0.01f; // adjust accordingly

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

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        viraCamera(0.0f,1.0f);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        viraCamera(0.0f,-1.0f);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        viraCamera(-1.0f,0.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        viraCamera(1.0f,0.0f);

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


