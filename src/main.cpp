//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   LABORATÓRIO 4
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>

// Headers locais, definidos na pasta "include/"
#include "utils.h"
#include "matrices.h"
#include "collisions.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define SKYBOX 0
#define PLANE  1
#define CAR    2
#define CAR_HOOD 3
#define CAR_GLASS 4
#define CAR_PAINTING 5
#define CAR_METALIC 6
#define CAR_WHEEL 7
#define CAR_NOT_PAINTED_PARTS 8
#define TREE_BODY 9
#define TREE_LEAVES 10
#define TRACK 11
#define BONUS 12
#define OUTDOOR_FACE 13
#define OUTDOOR_POST 14
#define FINISH_LINE 15

#define PI 3.141592f

// Camera look-at: valores maximos e minimos da camera em relação a z/y
#define MIN_DISTANCE_LOOK_AT_Z 3.0f
#define MAX_DISTANCE_LOOK_AT_Z 8.0f
#define MIN_DISTANCE_LOOK_AT_Y 0.8f
#define MAX_DISTANCE_LOOK_AT_Y 3.0f

// List of selected object names
const std::vector<std::string> selected_objects = {
    "body",
    "front_bumper",
    "Bottom_panel",
    "side_skirts",
    "Bottom_panel.001",
};

void InitializeBonusObjects();

void UpdateBonusObjects(float deltaTime); 

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .

struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        // Se basepath == NULL, então setamos basepath como o dirname do
        // filename, para que os arquivos MTL sejam corretamente carregados caso
        // estejam no mesmo diretório dos arquivos OBJ.
        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i+1);
                basepath = dirname.c_str();
            }
        }

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            if (shapes[shape].name.empty())
            {
                fprintf(stderr,
                        "*********************************************\n"
                        "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                        "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                        "*********************************************\n",
                    filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }

        printf("OK.\n");
    }
};

struct Car
{
    glm::vec3 carPosition; 
    glm::vec3 carVelocity;
    glm::vec3 carAcceleration;
    glm::vec3 carDirection;
    
    float speed; // Velocidade atual do carro
    float acceleration; // Aceleração (ou RPM) do carro
    float acceleration_rate; // Taxa de aceleração
    float deceleration_rate; // Taxa de desaceleração
    
    float max_speed; // Velocidade máxima do carro
    float max_acceleration; // Aceleracao Máxima do carro
    
    float wheel_rotation_angle; // Angulo atual de rotacao das rodas (foi feita simplificacao de todas rodas terem a mesma rotacao)
    float rotation_angle; // Angulo de rotacao do carro

    float front_wheel_angle; // Angulo atual de rotação das rodas dianteiras
    float max_front_wheel_angle; // Ângulo máximo de rotação das rodas dianteiras
    float negative_camber_angle; // Ângulo de cambagem das rodas

    glm::vec3 frontLeftWheelPosition; // Posição da roda dianteira esquerda
    glm::vec3 frontRightWheelPosition; // Posição da roda dianteira direita
    glm::vec3 rearLeftWheelPosition; // Posição da roda traseira esquerda
    glm::vec3 rearRightWheelPosition; // Posição da roda traseira direita

    glm::mat4 frontLeftWheelTransform; 
    glm::mat4 frontRightWheelTransform; 
    glm::mat4 rearLeftWheelTransform; 
    glm::mat4 rearRightWheelTransform; 

    int pontuation;
    float pontuation_multiplier;

    // Construtor
    Car() 
        : carPosition(0.0f, -0.95f, 0.0f),
          carVelocity(0.0f, 0.0f, 0.0f), 
          carAcceleration(0.0f, 0.0f, 0.0f), 
          carDirection(0.0f, 0.0f, -1.0f),
          speed(0.0f), 
          acceleration(0.0f), 
          acceleration_rate(10.0f), 
          deceleration_rate(5.0f),
          max_speed(20.0f), 
          max_acceleration(20.0f),
          wheel_rotation_angle(0.0f),
          rotation_angle(0.0f),
          front_wheel_angle(0.0f),
          max_front_wheel_angle(glm::radians(40.0f)),
          negative_camber_angle(glm::radians(10.0f)),
          frontLeftWheelPosition(-1.11f, -0.5503f, 0.1809f),
          frontRightWheelPosition(-1.11f, 0.5393f, 0.1858f),          
          rearLeftWheelPosition(0.5940f, -0.5501f, 0.19642f ),
          rearRightWheelPosition(0.59399f, 0.54683f, 0.20197f),
          frontLeftWheelTransform(1.0f),
          frontRightWheelTransform(1.0f),
          rearLeftWheelTransform(1.0f),
          rearRightWheelTransform(1.0f),
          pontuation(0),
          pontuation_multiplier(1.0f)
    {}
};

Car car;

// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename);
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(ObjModel*); // Função para debugging

void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
void DrawBoundingBox(const std::string& object_name);

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowVelocity(GLFWwindow* window);
void TextRendering_ShowPontuation(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

void DrawCar();
void DrawOutdoors();
void DrawBonus();
void DrawTrees();
void DrawWheelsWithTransform(const char* object_name, glm::mat4 transform);

std::pair<glm::vec3, glm::vec3> ComputeCarAABB(const Car& car);
void resetCar();

// Atualizacoes no carro
void UpdateCarSpeedAndPosition(Car &car, bool key_W_pressed, bool key_S_pressed, bool key_A_pressed, bool key_D_pressed, float deltaTime);
void UpdateFrontWheelsAngle(Car &car, bool key_A_pressed, bool key_D_pressed, float deltaTime);
void UpdateWheelsTransforms(Car &car, float deltaTime);
void UpdatePontuation(Car &car, float deltaTime);

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;
};

// estrutura auxiliar para desenhar os objetos do carro
struct ObjectConfig {
    int object_id;
    std::string object_name;
    int uv_mapping_type;
};

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 3.5f; // Distância da câmera para a origem

// Variáveis que controlam rotação do antebraço
float g_ForearmAngleZ = 0.0f;
float g_ForearmAngleX = 0.0f;

// Variáveis que controlam translação do torso
float g_TorsoPositionX = 0.0f;
float g_TorsoPositionY = 0.0f;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = false;
// Variável que controla se as bounding boxes serao desenhadas na tela
bool g_Show_BBOX = false;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;
GLint g_model_uniform;
GLint g_view_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform;
GLint g_bbox_min_uniform;
GLint g_bbox_max_uniform;
GLuint g_uv_mapping_type_uniform;

// Número de texturas carregadas pela função LoadTextureImage()
GLuint g_NumLoadedTextures = 0;

// Movimentacao do carro
bool key_W_pressed = false;
bool key_S_pressed = false;
bool key_A_pressed = false;
bool key_D_pressed = false;


    struct BezierCurve {
        glm::vec3 p0; // Starting point
        glm::vec3 p1; // Control point 1
        glm::vec3 p2; // Control point 2
        glm::vec3 p3; // Ending point

        // Evaluate the Bezier curve at parameter t ∈ [0, 1]
        glm::vec3 evaluate(float t) const {
            float u = 1.0f - t;
            float tt = t * t;
            float uu = u * u;
            float uuu = uu * u;
            float ttt = tt * t;

            glm::vec3 point = uuu * p0;
            point += 3.0f * uu * t * p1;
            point += 3.0f * u * tt * p2;
            point += ttt * p3;

            return point;
     }
    };  

    struct BonusObject {
        glm::vec3 initialPosition;
        BezierCurve pathCurve;
        float t; // Parameter to track the object's position along the curve
        float speed; // Speed at which 't' progresses
        bool active; // Whether the bonus is active (for optional reuse)
        glm::vec3 currentPostion;

        BonusObject(glm::vec3 initPos, BezierCurve curve, float spd)
            : initialPosition(initPos), pathCurve(curve), t(0.0f), speed(spd), active(true), currentPostion(initPos) {}
    };// Vector to hold all bonus objects
std::vector<BonusObject> bonusObjects;


// Camera look-at: define fator de progressão ao usar scroll para zoom
float delta_look_at_y = MAX_DISTANCE_LOOK_AT_Y - MIN_DISTANCE_LOOK_AT_Y; 
float delta_look_at_z = MAX_DISTANCE_LOOK_AT_Z - MIN_DISTANCE_LOOK_AT_Z;
float relation_delta_yz = delta_look_at_y / delta_look_at_z;

// Camera look-at
bool negative_acceleration = false;

// Movimentacao da camera livre
bool key_UP_pressed = false;
bool key_DOWN_pressed = false;
bool key_LEFT_pressed = false;
bool key_RIGHT_pressed = false;

#define TIMEOUT_FINISH_LINE 5.0f

double last_bonus = glfwGetTime();
bool can_receive_finish_line = false;

// Toggle do tipo de camera
// true para look at, false para livre
bool type_camera_look_at = true;
float camera_speed = 3.0f;
// Camera look at: valor inicial de offset em relação ao carro
glm::vec3 camera_offset(0.0f, MAX_DISTANCE_LOOK_AT_Y, MAX_DISTANCE_LOOK_AT_Z);

glm::vec3 finish_line_min(-5.24f, -0.95f, 2.68f);
glm::vec3 finish_line_max(5.24f, -0.95f, 3.32f);

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    window = glfwCreateWindow(1000, 800, "Necessidade por Velocidade (UFRGS Edition)", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 1000, 800); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

    LoadTextureImage("../../data/background/kloofendal_48d_partly_cloudy_puresky_4k.hdr"); // TextureSkybox

    // Texturas do carro
    LoadTextureImage("../../data/car/car-textures/Naval_Ensign_of_Japan.png"); // TextureCarHood
    LoadTextureImage("../../data/car/car-textures/1K-Silver_Base Color.jpg"); // TextureCarMetalic
    LoadTextureImage("../../data/car/car-textures/SolidBlack.png"); // TextureCarGlass
    LoadTextureImage("../../data/car/car-textures/Metal049A_1K-JPG_Color.jpg"); // TextureCarPainting
    LoadTextureImage("../../data/car/car-textures/Rubber004_1K-JPG_Color.jpg"); // TextureCarWheel
    LoadTextureImage("../../data/car/car-textures/Metal027_1K-JPG_Color.jpg"); // TextureCarNotPaintedParts

    // Outras texturas
    LoadTextureImage("../../data/plane/Grass004_1K-JPG_Color.jpg"); // TextureGrass
    LoadTextureImage("../../data/track/Asphalt026C_1K-JPG_Color.jpg"); // TextureTrack
    LoadTextureImage("../../data/tree/Bark012_1K-JPG_Color.jpg"); // TextureTree
    LoadTextureImage("../../data/bonus/Metal048A_1K-JPG_Color.jpg"); // TextureBonus
    LoadTextureImage("../../data/outdoor/jdm-japan-flag.png"); // TextureOutdoorFace
    LoadTextureImage("../../data/line/white-texture.jpg"); // TextureFinishLine


    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel skyboxmodel("../../data/background/skysemisphere.obj");
    ComputeNormals(&skyboxmodel);
    BuildTrianglesAndAddToVirtualScene(&skyboxmodel);

    ObjModel trackmodel("../../data/track/track.obj");
    ComputeNormals(&trackmodel);
    BuildTrianglesAndAddToVirtualScene(&trackmodel);

    ObjModel planemodel("../../data/plane/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel carmodel("../../data/car/supratunadov5.obj");
    ComputeNormals(&carmodel);
    BuildTrianglesAndAddToVirtualScene(&carmodel);

    ObjModel treemodel("../../data/tree/tree.obj");
    ComputeNormals(&treemodel);
    BuildTrianglesAndAddToVirtualScene(&treemodel);

    ObjModel bonusmodel("../../data/bonus/bonus-compressed.obj");
    ComputeNormals(&bonusmodel);
    BuildTrianglesAndAddToVirtualScene(&bonusmodel);

    ObjModel outdoormodel("../../data/outdoor/outdoor.obj");
    ComputeNormals(&outdoormodel);
    BuildTrianglesAndAddToVirtualScene(&outdoormodel);

    ObjModel finishlinemodel("../../data/line/finish-line.obj");
    ComputeNormals(&finishlinemodel);
    BuildTrianglesAndAddToVirtualScene(&finishlinemodel);

    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    InitializeBonusObjects();

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        static double previousTime = glfwGetTime();
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - previousTime);
        previousTime = currentTime;

        // Funcoes de atualizacao das propriedades do carro
        UpdateCarSpeedAndPosition(car, key_W_pressed, key_S_pressed, key_A_pressed, key_D_pressed, deltaTime);        
        UpdateFrontWheelsAngle(car, key_A_pressed, key_D_pressed, deltaTime);
        UpdateWheelsTransforms(car, deltaTime);
        UpdatePontuation(car, deltaTime);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(g_GpuProgramID);

        // Computamos a posição da câmera utilizando coordenadas esféricas.  As
        // variáveis g_CameraDistance, g_CameraPhi, e g_CameraTheta são
        // controladas pelo mouse do usuário. Veja as funções CursorPosCallback()
        // e ScrollCallback().
        float r = g_CameraDistance;
        float y = r*sin(g_CameraPhi);
        float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

        glm::vec4 camera_position_c;
        glm::vec4 camera_lookat_l;
        glm::vec4 camera_view_vector;
        glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); // Vetor "up" fixado para apontar para o "céu" (eito Y global)
        if (type_camera_look_at) {
            camera_lookat_l  = glm::vec4(car.carPosition.x, car.carPosition.y, car.carPosition.z, 1.0f); // Camera look-at
            glm::vec3 car_direction = glm::normalize(car.carDirection);
            glm::vec3 camera_offset_rotated = glm::vec3(
                -(car_direction.x * camera_offset.z + car_direction.z * camera_offset.x),
                camera_offset.y,
                -(car_direction.z * camera_offset.z - car_direction.x * camera_offset.x)
            );
            camera_position_c  = glm::vec4(car.carPosition.x + camera_offset_rotated.x, 
                     car.carPosition.y + camera_offset_rotated.y, 
                     car.carPosition.z + camera_offset_rotated.z, 
                     1.0f); // Ponto "c", centro da câmera
            camera_view_vector = camera_lookat_l - camera_position_c; 
        }
        else {
            camera_view_vector = -glm::vec4(x, y, z, 0.0f);
            glm::vec4 vetor_w  = -camera_view_vector;
            glm::vec4 vetor_u  = crossproduct(camera_up_vector, vetor_w);
            if (key_UP_pressed)
                camera_position_c += -vetor_w * camera_speed * deltaTime;
            if (key_DOWN_pressed)
                camera_position_c += vetor_w * camera_speed * deltaTime;    
            if (key_RIGHT_pressed)
                camera_position_c += vetor_u * camera_speed * deltaTime;
            if (key_LEFT_pressed)
                camera_position_c += -vetor_u * camera_speed * deltaTime;
        }

        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

        glm::mat4 projection;

        float nearplane = -0.1f;  // Posição do "near plane"
        float farplane  = -1000.0f; // Posição do "far plane"

        float field_of_view = PI / 3.0f;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);

        glm::mat4 model = Matrix_Identity(); 

        glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        UpdateBonusObjects(deltaTime); // Update positions based on Bezier curves

        DrawCar();
     
        DrawTrees();

        DrawBonus();

        DrawOutdoors();

        // skybox
        model = Matrix_Rotate_Y(-PI/4) *
                Matrix_Scale(350.0f, 350.0f, 350.0f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, SKYBOX);
        glUniform1i(g_uv_mapping_type_uniform, 99);
        DrawVirtualObject("the_skysemisphere");

        // pista
        model = Matrix_Translate(0.0f, -0.98f, 0.0f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, TRACK);
        glUniform1i(g_uv_mapping_type_uniform, 1);
        DrawVirtualObject("the_track");

        // plano da grama
        model = Matrix_Translate(0.0f, -1.0f, 0.0f)
              * Matrix_Scale(1.0f, 1.0f, 1.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, PLANE);
        glUniform1i(g_uv_mapping_type_uniform, 1);
        DrawVirtualObject("the_plane");

        model = Matrix_Translate(0.0f, -0.95f, 3.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, FINISH_LINE);
        glUniform1i(g_uv_mapping_type_uniform, 1);
        DrawVirtualObject("finish_line");

        TextRendering_ShowVelocity(window);
        TextRendering_ShowPontuation(window);
        TextRendering_ShowFramesPerSecond(window);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}

void LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Parâmetros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;
}
// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Setamos as variáveis "bbox_min" e "bbox_max" do fragment shader
    // com os parâmetros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    if (g_Show_BBOX == true) 
        DrawBoundingBox(object_name);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

void DrawBoundingBox(const std::string& object_name)
{

    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;

    GLfloat vertices[] = {
        bbox_min.x, bbox_min.y, bbox_min.z, // 0
        bbox_max.x, bbox_min.y, bbox_min.z, // 1
        bbox_max.x, bbox_max.y, bbox_min.z, // 2
        bbox_min.x, bbox_max.y, bbox_min.z, // 3
        bbox_min.x, bbox_min.y, bbox_max.z, // 4
        bbox_max.x, bbox_min.y, bbox_max.z, // 5
        bbox_max.x, bbox_max.y, bbox_max.z, // 6
        bbox_min.x, bbox_max.y, bbox_max.z  // 7
    };

    GLuint indices[] = {
        0, 1, 1, 2, 2, 3, 3, 0, // bottom face
        4, 5, 5, 6, 6, 7, 7, 4, // top face
        0, 4, 1, 5, 2, 6, 3, 7  // vertical lines
    };

    GLuint vao = 0, vbo = 0, ebo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Draw the bounding box
    glBindVertexArray(vao);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Cleanup
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}


// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shaders/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shaders/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    g_model_uniform      = glGetUniformLocation(g_GpuProgramID, "model"); // Variável da matriz "model"
    g_view_uniform       = glGetUniformLocation(g_GpuProgramID, "view"); // Variável da matriz "view" em shader_vertex.glsl
    g_projection_uniform = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    g_object_id_uniform  = glGetUniformLocation(g_GpuProgramID, "object_id"); // Variável "object_id" em shader_fragment.glsl
    g_bbox_min_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_min");
    g_bbox_max_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_max");

    // Nova variável para o tipo de mapeamento UV
    g_uv_mapping_type_uniform = glGetUniformLocation(g_GpuProgramID, "uv_mapping_type");

    // Variáveis em "shader_fragment.glsl" para acesso das imagens de textura
    glUseProgram(g_GpuProgramID);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureSkybox"), 0);

    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureCarHood"), 1);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureCarMetalic"), 2);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureCarGlass"), 3);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureCarPainting"), 4);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureCarWheel"), 5);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureCarNotPaintedParts"), 6);
    
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureGrass"), 7);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureTrack"), 8);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureTree"), 9);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureBonus"), 10);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureOutdoorFace"), 11);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureFinishLine"), 12);

}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice.

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4  vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
            }

            const glm::vec4  a = vertices[0];
            const glm::vec4  b = vertices[1];
            const glm::vec4  c = vertices[2];

            // PREENCHA AQUI o cálculo da normal de um triângulo cujos vértices
            // estão nos pontos "a", "b", e "c", definidos no sentido anti-horário.
            glm::vec3 edge1 = glm::vec3(b - a);
            glm::vec3 edge2 = glm::vec3(c - a);
            glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
            const glm::vec4  n = glm::vec4(normal, 0.0f);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize( 3*num_vertices );

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3*i + 0] = n.x;
        model->attrib.normals[3*i + 1] = n.y;
        model->attrib.normals[3*i + 2] = n.z;
    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para deleção após serem linkados 
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.

    if (g_LeftMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da câmera com os deslocamentos
        g_CameraTheta -= 0.01f*dx;
        g_CameraPhi   += 0.01f*dy;
    
        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = PI/2;
        float phimin = -phimax;
    
        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;
    
        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_RightMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_ForearmAngleZ -= 0.01f*dx;
        g_ForearmAngleX += 0.01f*dy;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_MiddleMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_TorsoPositionX += 0.01f*dx;
        g_TorsoPositionY -= 0.01f*dy;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    //g_CameraDistance -= 0.1f*yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    //const float verysmallnumber = std::numeric_limits<float>::epsilon();
    
    float scrollSensitivity = 1.0f; // Camera look at: usar scroll para aumentar/diminuir distancia
    camera_offset.z -= yoffset * scrollSensitivity;
    camera_offset.y -= yoffset * scrollSensitivity * relation_delta_yz;
    //if (g_CameraDistance < verysmallnumber)
    //    g_CameraDistance = verysmallnumber;
    if (camera_offset.z < MIN_DISTANCE_LOOK_AT_Z)
    {
        camera_offset.z = MIN_DISTANCE_LOOK_AT_Z;
    }
    if (camera_offset.z > MAX_DISTANCE_LOOK_AT_Z)
    {
        camera_offset.z = MAX_DISTANCE_LOOK_AT_Z;
    }
    if (camera_offset.y < MIN_DISTANCE_LOOK_AT_Y)
    {
        camera_offset.y = MIN_DISTANCE_LOOK_AT_Y;
    }
    if (camera_offset.y > MAX_DISTANCE_LOOK_AT_Y)
    {
        camera_offset.y = MAX_DISTANCE_LOOK_AT_Y;
    }

}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // =====================
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    // =====================

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Movimentacao do Carro
    if (key == GLFW_KEY_W)
    {
        if (action == GLFW_PRESS)
            key_W_pressed = true;

        else if (action == GLFW_RELEASE)
            key_W_pressed = false;

        else if (action == GLFW_REPEAT)
            ;
    }

    if (key == GLFW_KEY_S)
    {
        if (action == GLFW_PRESS)
            key_S_pressed = true;

        else if (action == GLFW_RELEASE)
            key_S_pressed = false;

        else if (action == GLFW_REPEAT)
            ;
    }

    if (key == GLFW_KEY_A)
    {
        if (action == GLFW_PRESS)
            key_A_pressed = true;

        else if (action == GLFW_RELEASE)
            key_A_pressed = false;

        else if (action == GLFW_REPEAT)
            ;
    }
        if (key == GLFW_KEY_D)
    {
        if (action == GLFW_PRESS)
            key_D_pressed = true;

        else if (action == GLFW_RELEASE)
            key_D_pressed = false;

        else if (action == GLFW_REPEAT)
            ;
    }

    // Movimentacao da camera livre
    if (key == GLFW_KEY_UP)
    {
        if (action == GLFW_PRESS)
            key_UP_pressed = true;

        else if (action == GLFW_RELEASE)
            key_UP_pressed = false;

        else if (action == GLFW_REPEAT)
            ;
    }
    if (key == GLFW_KEY_DOWN)
    {
        if (action == GLFW_PRESS)
            key_DOWN_pressed = true;

        else if (action == GLFW_RELEASE)
            key_DOWN_pressed = false;

        else if (action == GLFW_REPEAT)
            ;
    }
    if (key == GLFW_KEY_LEFT)
    {
        if (action == GLFW_PRESS)
            key_LEFT_pressed = true;

        else if (action == GLFW_RELEASE)
            key_LEFT_pressed = false;

        else if (action == GLFW_REPEAT)
            ;
    }
    if (key == GLFW_KEY_RIGHT)
    {
        if (action == GLFW_PRESS)
            key_RIGHT_pressed = true;

        else if (action == GLFW_RELEASE)
            key_RIGHT_pressed = false;

        else if (action == GLFW_REPEAT)
            ;
    }

      // Se o usuário apertar a tecla espaço, reseta a a posicao do carro com sua velocidade e etc
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        car.carPosition = glm::vec3(0.0f, -0.95f, 0.0f);
        car.carVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
        car.carAcceleration = glm::vec3(0.0f, 0.0f, 0.0f);
        car.speed = 0.0f;
        car.acceleration = 0.0f;
        car.wheel_rotation_angle = 0.0f;
        car.rotation_angle = 0.0f;
        car.front_wheel_angle = 0.0f;
        car.pontuation = 0;
    }
    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usuário apertar a tecla B, fazemos um "toggle" dos bbox dos objetos.
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        g_Show_BBOX = !g_Show_BBOX;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        type_camera_look_at = !type_camera_look_at;
    }
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Mostra velocidade atual do carro
void TextRendering_ShowVelocity(GLFWwindow* window)
{

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    char buffer[50];
    snprintf(buffer, 50, "Speed: %.2f", glm::length(car.carVelocity));
    TextRendering_PrintString(window, buffer, -1.0f + charwidth, 1.0f - 1 * lineheight, 1.0f);

    snprintf(buffer, 50, "Acceleration: %.2f", car.acceleration);
    TextRendering_PrintString(window, buffer, -1.0f + charwidth, 1.0f - 2 * lineheight, 1.0f);

    if (!g_ShowInfoText)
        return;

    snprintf(buffer, 50, "Vec Velocity: (%.2f, %.2f, %.2f)", car.carVelocity.x, car.carVelocity.y, car.carVelocity.z);
    TextRendering_PrintString(window, buffer, -1.0f + charwidth, 1.0f - 5 * lineheight, 1.0f);

    snprintf(buffer, 50, "Vec Acceleration: (%.2f, %.2f, %.2f)", car.carAcceleration.x, car.carAcceleration.y, car.carAcceleration.z);
    TextRendering_PrintString(window, buffer, -1.0f + charwidth, 1.0f - 6 * lineheight, 1.0f);

    snprintf(buffer, 50, "Front Wheel Angle: %.2f", car.front_wheel_angle);
    TextRendering_PrintString(window, buffer, -1.0f + charwidth, 1.0f - 8 * lineheight, 1.0f);

    snprintf(buffer, 50, "Car Rotation Angle: %.2f", car.rotation_angle);
    TextRendering_PrintString(window, buffer, -1.0f + charwidth, 1.0f - 9 * lineheight, 1.0f);

    snprintf(buffer, 50, "Vec Direction: (%.2f, %.2f, %.2f)", car.carDirection.x, car.carDirection.y, car.carDirection.z);
    TextRendering_PrintString(window, buffer, -1.0f + charwidth, 1.0f - 10 * lineheight, 1.0f);

}

void TextRendering_ShowPontuation(GLFWwindow* window)
{
    float pad = TextRendering_LineHeight(window);
    char buffer[50];
    snprintf(buffer, 50, "Multiplier: %.1f x", car.pontuation_multiplier);
    TextRendering_PrintString(window, buffer, -1.0f + pad / 10, -1.0f + 12 * pad / 10, 1.0f);
    snprintf(buffer, 50, "Pontuation: %d", car.pontuation);
    TextRendering_PrintString(window, buffer, -1.0f + pad / 10, -1.0f + 2 * pad / 10, 1.0f);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    // if ( !g_ShowInfoText )
    //     return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);
    
        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// Função para debugging: imprime no terminal todas informações de um modelo
// geométrico carregado de um arquivo ".obj".
// Veja: https://github.com/syoyo/tinyobjloader/blob/22883def8db9ef1f3ffb9b404318e7dd25fdbb51/loader_example.cc#L98
void PrintObjModelInfo(ObjModel* model)
{
  const tinyobj::attrib_t                & attrib    = model->attrib;
  const std::vector<tinyobj::shape_t>    & shapes    = model->shapes;
  const std::vector<tinyobj::material_t> & materials = model->materials;

  printf("# of vertices  : %d\n", (int)(attrib.vertices.size() / 3));
  printf("# of normals   : %d\n", (int)(attrib.normals.size() / 3));
  printf("# of texcoords : %d\n", (int)(attrib.texcoords.size() / 2));
  printf("# of shapes    : %d\n", (int)shapes.size());
  printf("# of materials : %d\n", (int)materials.size());

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", materials[i].roughness);
    printf("  material.Pm     = %f\n", materials[i].metallic);
    printf("  material.Ps     = %f\n", materials[i].sheen);
    printf("  material.Pc     = %f\n", materials[i].clearcoat_thickness);
    printf("  material.Pcr    = %f\n", materials[i].clearcoat_thickness);
    printf("  material.aniso  = %f\n", materials[i].anisotropy);
    printf("  material.anisor = %f\n", materials[i].anisotropy_rotation);
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}

void DrawAxes(const glm::mat4& model)
{
    GLfloat vertices[] = {
        // X axis (red)
        0.0f, 0.0f, 0.0f, // Origin
        1.5f, 0.0f, 0.0f, // X axis

        // Y axis (green)
        0.0f, 0.0f, 0.0f, // Origin
        0.0f, 1.5f, 0.0f, // Y axis

        // Z axis (blue)
        0.0f, 0.0f, 0.0f, // Origin
        0.0f, 0.0f, 1.5f  // Z axis
    };

    GLfloat colors[] = {
        // Colors for X axis (red)
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        // Colors for Y axis (green)
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        // Colors for Z axis (blue)
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f
    };

    GLuint vao, vbo[2];
    glGenVertexArrays(1, &vao);
    glGenBuffers(2, vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Draw the axes
    glBindVertexArray(vao);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glLineWidth(3.0f); // Increase the line width
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);

    // Cleanup
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(2, vbo);
}

void DrawCar()
{
    glm::mat4 model = Matrix_Translate(car.carPosition.x, car.carPosition.y, car.carPosition.z)
                    * Matrix_Rotate_Y(car.rotation_angle)
                    * Matrix_Rotate_Y(-PI/2)
                    * Matrix_Rotate_X(-PI/2);

    std::vector<ObjectConfig> car_objects = {
        {CAR_HOOD, "hood", 0}, // X
        
        {CAR_METALIC, "back_toyota_logo", 99},
        {CAR_METALIC, "front_toyota_logo", 99},
        {CAR_METALIC, "exhaust", 5},
        {CAR_METALIC, "license_plate", 2},
        
        {CAR_GLASS, "front_window", 0},
        {CAR_GLASS, "side_smaller_window", 1},
        {CAR_GLASS, "side_window", 1},
        {CAR_GLASS, "tail_window", 0},
        {CAR_GLASS, "front_light_glass", 2},
        {CAR_GLASS, "tail_lights_glass", 2},

        {CAR_PAINTING, "body", 3},
        {CAR_PAINTING, "door", 1},
        {CAR_PAINTING, "front_bumper", 0},
        {CAR_PAINTING, "front_fender", 0},
        {CAR_PAINTING, "side_panel", 0},
        {CAR_PAINTING, "trunk", 0},
        {CAR_PAINTING, "mirrors", 0},
        {CAR_PAINTING, "Wing", 0},

        {CAR_PAINTING, "front_window_frame", 0},
        {CAR_PAINTING, "side_smaller_window_frame", 0},
        {CAR_PAINTING, "side_window_frame", 0},
        {CAR_PAINTING, "tail_window_frame", 0},

        {CAR_NOT_PAINTED_PARTS, "Bottom_panel", 0},
        {CAR_NOT_PAINTED_PARTS, "front_skirts", 0},
        {CAR_NOT_PAINTED_PARTS, "tank", 3},
        {CAR_NOT_PAINTED_PARTS, "Bottom_panel", 0},
        {CAR_NOT_PAINTED_PARTS, "Bottom_panel.001", 0},
        {CAR_NOT_PAINTED_PARTS, "side_skirts", 0},
        {CAR_NOT_PAINTED_PARTS, "cooler_holes", 0},
        
        {CAR_WHEEL, "wheel_front_left", 3},
        {CAR_WHEEL, "wheel_back_left", 3},
        {CAR_WHEEL, "wheel_front_right", 3},
        {CAR_WHEEL, "wheel_back_right", 3}
    };

    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    for (const auto& obj : car_objects) {
        glUniform1i(g_object_id_uniform, obj.object_id);
        glUniform1i(g_uv_mapping_type_uniform, obj.uv_mapping_type);

        if (obj.object_name == "wheel_front_left") {
            glm::mat4 frontLeftWheelModel = model * car.frontLeftWheelTransform;
            DrawWheelsWithTransform(obj.object_name.c_str(), frontLeftWheelModel);
        }
        else if (obj.object_name == "wheel_front_right") {
            glm::mat4 frontRightWheelModel = model * car.frontRightWheelTransform;
            DrawWheelsWithTransform(obj.object_name.c_str(), frontRightWheelModel);
        }
        else if (obj.object_name == "wheel_back_left") {
            glm::mat4 rearLeftWheelModel = model * car.rearLeftWheelTransform;
            DrawWheelsWithTransform(obj.object_name.c_str(), rearLeftWheelModel);
        }
        else if (obj.object_name == "wheel_back_right") {
            glm::mat4 rearRightWheelModel = model * car.rearRightWheelTransform;
            DrawWheelsWithTransform(obj.object_name.c_str(), rearRightWheelModel);
        }
        else
            DrawVirtualObject(obj.object_name.c_str());

        if (g_Show_BBOX)
            DrawBoundingBox(obj.object_name.c_str());
    }
}
void DrawOutdoors() 
{
    // outdoor main
    glm::mat4 model = Matrix_Translate(0.0f, 5.0f, -30.0f)
        * Matrix_Scale(2.5f, 2.5f, 2.5f);
    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    glUniform1i(g_object_id_uniform, OUTDOOR_FACE);
    glUniform1i(g_uv_mapping_type_uniform, 0);
    DrawVirtualObject("outdoor_face");
    glUniform1i(g_object_id_uniform, OUTDOOR_POST);
    glUniform1i(g_uv_mapping_type_uniform, 3);
    DrawVirtualObject("outdoor_post1");
    DrawVirtualObject("outdoor_post2");
    glUniform1i(g_uv_mapping_type_uniform, 0);
    DrawVirtualObject("outdoor_back");

    // outros outdoors
    model = Matrix_Translate(30.0f, 0.7f, -100.0f) * Matrix_Scale(0.7f, 0.7f, 0.7f);
    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    glUniform1i(g_object_id_uniform, OUTDOOR_FACE);
    glUniform1i(g_uv_mapping_type_uniform, 0);
    DrawVirtualObject("outdoor_face");
    glUniform1i(g_object_id_uniform, OUTDOOR_POST);
    glUniform1i(g_uv_mapping_type_uniform, 3);
    DrawVirtualObject("outdoor_post1");
    DrawVirtualObject("outdoor_post2");
    glUniform1i(g_uv_mapping_type_uniform, 0);
    DrawVirtualObject("outdoor_back");

    model = Matrix_Translate(22.0f, 0.7f, -44.0f) * Matrix_Scale(0.7f, 0.7f, 0.7f) * Matrix_Rotate_Y(PI/2);
    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    glUniform1i(g_object_id_uniform, OUTDOOR_FACE);
    glUniform1i(g_uv_mapping_type_uniform, 0);
    DrawVirtualObject("outdoor_face");
    glUniform1i(g_object_id_uniform, OUTDOOR_POST);
    glUniform1i(g_uv_mapping_type_uniform, 3);
    DrawVirtualObject("outdoor_post1");
    DrawVirtualObject("outdoor_post2");
    glUniform1i(g_uv_mapping_type_uniform, 0);
    DrawVirtualObject("outdoor_back");

    model = Matrix_Translate(-0.0f, 0.7f, 55.0f) * Matrix_Scale(0.7f, 0.7f, 0.7f) * Matrix_Rotate_Y(3*PI/4);
    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
    glUniform1i(g_object_id_uniform, OUTDOOR_FACE);
    glUniform1i(g_uv_mapping_type_uniform, 0);
    DrawVirtualObject("outdoor_face");
    glUniform1i(g_object_id_uniform, OUTDOOR_POST);
    glUniform1i(g_uv_mapping_type_uniform, 3);
    DrawVirtualObject("outdoor_post1");
    DrawVirtualObject("outdoor_post2");
    glUniform1i(g_uv_mapping_type_uniform, 0);
    DrawVirtualObject("outdoor_back");
}

void DrawBonus()
{
    glm::mat4 model = Matrix_Identity();

    for (const auto& bonus : bonusObjects) {
        if (bonus.active) {
            model = Matrix_Translate(bonus.currentPostion.x, bonus.currentPostion.y, bonus.currentPostion.z)
                    * Matrix_Scale(0.6f, 0.6f, 0.6f);
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, BONUS);
            glUniform1i(g_uv_mapping_type_uniform, 0);
            DrawVirtualObject("the_bonus");
            
            if (g_Show_BBOX)
                DrawBoundingBox("the_bonus");
        }
    }
}

void DrawTrees()
{
    glm::mat4 model = Matrix_Identity();

    std::vector<glm::vec3> tree_positions = {
        glm::vec3(6.0f,-1.0f,-8.0f), 
        glm::vec3(-6.0f,-1.0f,-8.0f),
        glm::vec3(78.0f, -1.0f, -74.0f),
        glm::vec3(96.0f, -1.0f, -74.0f),
        glm::vec3(17.0f, -1.0f, -70.0f),
        glm::vec3(41.0f, -1.0f, -42.0f),
        glm::vec3(47.0f, -1.0f, -4.0f),
        glm::vec3(39.0f, -1.0f, 31.0f),
        glm::vec3(10.0f, -1.0f, 35.0f),
    };
    for (const auto& pos : tree_positions) {
        model = Matrix_Translate(pos.x, pos.y, pos.z);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, TREE_BODY);
        glUniform1i(g_uv_mapping_type_uniform, 5);
        DrawVirtualObject("tree_body");
        glUniform1i(g_object_id_uniform, TREE_LEAVES);
        glUniform1i(g_uv_mapping_type_uniform, 5);
        DrawVirtualObject("tree_leaves");

        if (g_Show_BBOX) {
            DrawBoundingBox("tree_body");
        }
    }
}

void DrawWheelsWithTransform(const char* object_name, glm::mat4 transform)
{
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);
    // if (g_Show_BBOX == true) 
    //     DrawBoundingBox(bbox_min, bbox_max);
        
    SceneObject& obj = g_VirtualScene[object_name];
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform1i(g_object_id_uniform, CAR_WHEEL);
    glBindVertexArray(obj.vertex_array_object_id);
    glUniform1i(g_uv_mapping_type_uniform, 0);

    glDrawElements(obj.rendering_mode, obj.num_indices, GL_UNSIGNED_INT, (void*)(obj.first_index * sizeof(GLuint)));
    glBindVertexArray(0);
}

// Lógica para atualização da velocidade e posição do carro
void UpdateCarSpeedAndPosition(Car &car, bool key_W_pressed, bool key_S_pressed, bool key_A_pressed, bool key_D_pressed, float deltaTime)
{
    const float epsilon = 0.1f;
    const float drift_factor = 0.99f; // Intensidade do drift (quanto maior, mais "ensaboado")
    const float stability = 0.5f; // Fator de estabilização do drift

    // Direção atual baseada na orientação do carro
    glm::vec3 forward_direction = glm::normalize(glm::vec3(
        -sin(car.rotation_angle), 
        0.0f, 
        -cos(car.rotation_angle)
    ));

    glm::vec3 right_direction = glm::normalize(crossproduct3(forward_direction, glm::vec3(0.0f, 1.0f, 0.0f)));

    car.carDirection = forward_direction;

    // Controle de aceleração
    if (key_W_pressed)
    {
        if (car.acceleration < 0.0f) {
            car.carAcceleration = glm::vec3(0.0f);
        }
        float speed_factor = 1.0f - (glm::length(car.carVelocity) / car.max_speed);
        car.carAcceleration += forward_direction * car.acceleration_rate * speed_factor * deltaTime;
        if (glm::length(car.carAcceleration) > car.max_acceleration)
            car.carAcceleration = glm::normalize(car.carAcceleration) * car.max_acceleration;
    }
    else if (key_S_pressed)
    {
        if (car.acceleration > 0.0f) {
            car.carAcceleration = glm::vec3(0.0f);
        }
        car.carAcceleration -= forward_direction * car.acceleration_rate * deltaTime;
        if (glm::length(car.carAcceleration) > car.max_acceleration)
            car.carAcceleration = -glm::normalize(car.carAcceleration) * car.max_acceleration;
    }
    else
    {
        if (glm::length(car.carVelocity) > epsilon)
        {
            car.carAcceleration = -glm::normalize(car.carVelocity) * car.deceleration_rate;
        }
        else
        {
            car.carAcceleration = glm::vec3(0.0f);
            car.carVelocity = glm::vec3(0.0f);
        }
    }

    // Atualiza a velocidade do carro com base na aceleração
    car.carVelocity += car.carAcceleration * deltaTime;

    // Limita a velocidade máxima
    if (glm::length(car.carVelocity) > car.max_speed)
        car.carVelocity = glm::normalize(car.carVelocity) * car.max_speed;

    // Simulação de drift (deslizamento lateral)
    glm::vec3 lateral_velocity = dotproduct3(car.carVelocity, right_direction) * right_direction;
    glm::vec3 forward_velocity = dotproduct3(car.carVelocity, forward_direction) * forward_direction;

    // Aplica o fator de drift e estabilidade
    lateral_velocity *= drift_factor;
    car.carVelocity = forward_velocity + lateral_velocity;

    // Gradualmente reduz o deslizamento lateral para estabilizar o carro
    car.carVelocity -= lateral_velocity * stability * deltaTime;

    // Atualiza a rotação do carro (direção geral)
    if (glm::length(car.carVelocity) > epsilon)
    {
        float rotation_angle = glm::tan(car.front_wheel_angle);
        car.rotation_angle += rotation_angle * deltaTime;
    }

    // Atualiza a posição do carro
    car.carPosition += car.carVelocity * deltaTime;

    std::pair<glm::vec3, glm::vec3> bbox = ComputeCarAABB(car);
    glm::vec3 bbox_min = bbox.first;
    glm::vec3 bbox_max = bbox.second;
    
    // Verifica colisão com arvores
    if(cube_cilinder_intersect_tree(bbox_min, bbox_max)){
        car.carPosition -= car.carVelocity * deltaTime;
        car.carVelocity = glm::vec3(0.0f);
        resetCar(); 
    }
    
    // Verifica colisão com outdoor
    if(cube_cilinder_intersect_outdoor(bbox_min, bbox_max)){
        car.carPosition -= car.carVelocity * deltaTime;
        car.carVelocity = glm::vec3(0.0f); 
        resetCar();
    }

    // Verifica colisão com bonus
    for(int i=0; i<5; i++){
        if(cube_sphere_intersect_bonus(bbox_min, bbox_max, bonusObjects[i].currentPostion) && bonusObjects[i].active){   
            car.pontuation_multiplier += 0.1;
            bonusObjects[i].active = false;
        }
    }
    
    if((glfwGetTime() - last_bonus) > TIMEOUT_FINISH_LINE){
        can_receive_finish_line = true;
    }

    // Verifica colisão com linha de chegada
    if(point_cube_intersect(car.carPosition, finish_line_min, finish_line_max) && can_receive_finish_line){
        car.pontuation += 1000;
        can_receive_finish_line = false;
        last_bonus = glfwGetTime();
        for (auto& bonus : bonusObjects) {
            bonus.active = true;
        }    
    }

    // Atualiza valores escalares
    car.speed = glm::length(car.carVelocity);
    car.acceleration = glm::length(car.carAcceleration);
    car.acceleration = (dotproduct3(car.carAcceleration, forward_direction) < 0) ? -car.acceleration : car.acceleration;
}

void UpdateFrontWheelsAngle(Car &car, bool key_A_pressed, bool key_D_pressed, float deltaTime) 
{
    float turn_speed = glm::radians(200.0f); // Velocidade de ajuste das rodas
    float return_speed = glm::radians(100.0f); // Velocidade de retorno ao neutro

    if (key_A_pressed)
    {
        car.front_wheel_angle += turn_speed * deltaTime;
        if (car.front_wheel_angle > car.max_front_wheel_angle)
            car.front_wheel_angle = car.max_front_wheel_angle;
    }
    else if (key_D_pressed)
    {
        car.front_wheel_angle -= turn_speed * deltaTime;
        if (car.front_wheel_angle < -car.max_front_wheel_angle)
            car.front_wheel_angle = -car.max_front_wheel_angle;
    }
    else
    {
        if (car.front_wheel_angle > 0.0f)
        {
            car.front_wheel_angle -= return_speed * deltaTime;
            if (car.front_wheel_angle < 0.0f)
                car.front_wheel_angle = 0.0f;
        }
        else if (car.front_wheel_angle < 0.0f)
        {
            car.front_wheel_angle += return_speed * deltaTime;
            if (car.front_wheel_angle > 0.0f)
                car.front_wheel_angle = 0.0f;
        }
    }
}

void UpdateWheelsTransforms(Car &car, float deltaTime) 
{
    // Calcula a rotação das rodas com base na distância percorrida
    float rotation_direction = (dotproduct3(car.carVelocity, glm::vec3(0.0f, 0.0f, -1.0f)) < 0) ? 1.0f : -1.0f;
    float distance = rotation_direction * car.speed * deltaTime;
    float wheel_radius = 0.4f;
    car.wheel_rotation_angle += distance / wheel_radius;

    // Atualiza as matrizes de transformação das 4 rodas
    car.frontLeftWheelTransform = Matrix_Translate(
                                    car.frontLeftWheelPosition.x, 
                                    car.frontLeftWheelPosition.y, 
                                    car.frontLeftWheelPosition.z)
                                * Matrix_Rotate_X(-car.negative_camber_angle) // correcao da cambagem
                                * Matrix_Rotate_Z(car.front_wheel_angle) // angulacao das rodas (necessario apenas para as dianteiras)
                                * Matrix_Rotate_Y(car.wheel_rotation_angle) // rotacao das rodas
                                * Matrix_Rotate_X(car.negative_camber_angle) // correcao da cambagem
                                * Matrix_Translate(                                    
                                    -car.frontLeftWheelPosition.x, 
                                    -car.frontLeftWheelPosition.y, 
                                    -car.frontLeftWheelPosition.z);

    car.frontRightWheelTransform = Matrix_Translate(
                                car.frontRightWheelPosition.x, 
                                car.frontRightWheelPosition.y, 
                                car.frontRightWheelPosition.z)
                            * Matrix_Rotate_X(car.negative_camber_angle) // correcao da cambagem (lado direito tem valor inverso)
                            * Matrix_Rotate_Z(car.front_wheel_angle)
                            * Matrix_Rotate_Y(car.wheel_rotation_angle)
                            * Matrix_Rotate_X(-car.negative_camber_angle)
                            * Matrix_Translate(                                    
                                -car.frontRightWheelPosition.x, 
                                -car.frontRightWheelPosition.y, 
                                -car.frontRightWheelPosition.z);

    car.rearRightWheelTransform = Matrix_Translate(
                            car.rearRightWheelPosition.x, 
                            car.rearRightWheelPosition.y, 
                            car.rearRightWheelPosition.z)
                        * Matrix_Rotate_X(car.negative_camber_angle)
                        * Matrix_Rotate_Y(car.wheel_rotation_angle)
                        * Matrix_Rotate_X(-car.negative_camber_angle)
                        * Matrix_Translate(                                    
                            -car.rearRightWheelPosition.x, 
                            -car.rearRightWheelPosition.y, 
                            -car.rearRightWheelPosition.z);

    car.rearLeftWheelTransform = Matrix_Translate(car.rearLeftWheelPosition.x,
                            car.rearLeftWheelPosition.y,
                            car.rearLeftWheelPosition.z)
                        * Matrix_Rotate_X(-car.negative_camber_angle)
                        * Matrix_Rotate_Y(car.wheel_rotation_angle)
                        * Matrix_Rotate_X(car.negative_camber_angle)
                        * Matrix_Translate(-car.rearLeftWheelPosition.x,
                            -car.rearLeftWheelPosition.y,
                            -car.rearLeftWheelPosition.z);
    
}

void UpdatePontuation(Car &car, float deltaTime)
{
    // Atualiza a pontuação do carro baseada na velocidade e só aumenta quando o carro fizer curvas
    float abs_wheel_angle = glm::abs(car.front_wheel_angle);
    if (abs_wheel_angle > 0.3f)
    {
        car.pontuation += abs_wheel_angle * 50 * car.speed * deltaTime * car.pontuation_multiplier;
    }
}

std::pair<glm::vec3, glm::vec3> ComputeCarAABB(const Car& car) {
    // 1. Define the local (unrotated) bounding box corners relative to the car's center
    float halfWidth  = 0.64f;

    // Local space corners
    std::vector<glm::vec3> localCorners = {
        // Bottom face
        glm::vec3(-halfWidth, 0.0f, -1.8f),
        glm::vec3( halfWidth, 0.0f, -1.8f),
        glm::vec3( halfWidth, 0.0f,  1.3f),
        glm::vec3(-halfWidth, 0.0f,  1.3f),
        // Top face
        glm::vec3(-halfWidth,  0.8f, -1.8f),
        glm::vec3( halfWidth,  0.8f, -1.8f),
        glm::vec3( halfWidth,  0.8f,  1.3f),
        glm::vec3(-halfWidth,  0.8f,  1.3f)
    };

    // 2. Create the rotation matrix (assuming rotation around the Y-axis)
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), car.rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));

    // 3. Initialize min and max points with extreme values
    glm::vec3 aabb_min(std::numeric_limits<float>::max());
    glm::vec3 aabb_max(std::numeric_limits<float>::lowest());

    // 4. Transform each corner and update the AABB
    for (const auto& corner : localCorners) {
        // Apply rotation
        glm::vec4 rotatedCorner = rotationMatrix * glm::vec4(corner, 1.0f);

        // Apply translation to world space
        glm::vec3 worldCorner = glm::vec3(rotatedCorner) + car.carPosition;

        // Update AABB min
        aabb_min.x = std::min(aabb_min.x, worldCorner.x);
        aabb_min.y = std::min(aabb_min.y, worldCorner.y);
        aabb_min.z = std::min(aabb_min.z, worldCorner.z);

        // Update AABB max
        aabb_max.x = std::max(aabb_max.x, worldCorner.x);
        aabb_max.y = std::max(aabb_max.y, worldCorner.y);
        aabb_max.z = std::max(aabb_max.z, worldCorner.z);
    }

    return { aabb_min, aabb_max };
}

void resetCar(){
    car.carPosition = glm::vec3(0.0f, -0.95f, 0.0f);
    car.carVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
    car.carAcceleration = glm::vec3(0.0f, 0.0f, 0.0f);
    car.speed = 0.0f;
    car.acceleration = 0.0f;
    car.wheel_rotation_angle = 0.0f;
    car.rotation_angle = 0.0f;
    car.front_wheel_angle = 0.0f;
    car.pontuation = 0;
    car.pontuation_multiplier = 1;
    for (auto& bonus : bonusObjects) {
        bonus.active = true;
    }
}

void InitializeBonusObjects() {

    std::vector<glm::vec3> bonus_positions = {
        glm::vec3(16.0f, -0.9f, -89.0f), // curva 1
        glm::vec3(90.0f, -0.9f, -74.0f), // curva 2
        glm::vec3(27.0f, -0.9f, -44.0f), // curva 3
        glm::vec3(63.0f, -0.9f, -4.0f), // curva 4
        glm::vec3(10.0f, -0.9f, 53.0f), // curva 5
        // glm::vec3(0.0f, -0.9f, -2.0f) // posicao padrao
    };

    for(const auto& pos : bonus_positions) {
        BezierCurve curve = {
            pos, // p0: Start position
            pos + glm::vec3(-1.0f, 0.0f, 0.3f),  // p1: Control point 1
            pos + glm::vec3(1.0f, 0.0f, 0.3f),  // p2: Control point 2'1
            pos   // p3: End position
        };

        // Create a BonusObject and add it to the vector
        bonusObjects.emplace_back(pos, curve, 1.0f); // Adjust speed as needed
    }
}

void UpdateBonusObjects(float deltaTime) {
    for (auto& bonus : bonusObjects) {
        if (bonus.active) {
            bonus.t += bonus.speed * deltaTime; // Progress along the curve

            if (bonus.t > 1.0f) {
                bonus.t = 0.0f;
            }

            bonus.currentPostion = bonus.pathCurve.evaluate(bonus.t);

            // Update the model matrix or position of the bonus object
            // Assuming you have a function to update the object's position
            // For example:
            // UpdateBonusModelMatrix(bonusModel, newPosition);
        }
    }
}