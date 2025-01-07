#version 330 core

#define PI 3.141592f

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

in float gouraud_lambert;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
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


uniform int object_id;
uniform int uv_mapping_type;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureSkybox;

uniform sampler2D TextureCarHood;
uniform sampler2D TextureCarMetalic;
uniform sampler2D TextureCarGlass;
uniform sampler2D TextureCarPainting;
uniform sampler2D TextureCarWheel;
uniform sampler2D TextureCarNotPaintedParts;

uniform sampler2D TextureGrass;
uniform sampler2D TextureTrack;
uniform sampler2D TextureTree;
uniform sampler2D TextureBonus;
uniform sampler2D TextureOutdoorFace;
uniform sampler2D TextureFinishLine;


// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.5,0.0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Vetor que define o sentido da reflexão especular ideal.
    vec4 r = -l + 2 * n * dot(n, l); // PREENCHA AQUI o vetor de reflexão especular ideal

    vec4 h = normalize(l + v);

    // Parâmetros que definem as propriedades espectrais da superfície
    vec3 Kd; // Refletância difusa
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;
    
    // =========================================== MAPEAMENTO COORDENADAS UV =====================================================
    // 0 plano XY
    // 1 plano XZ
    // 2 plano YZ
    // 3 esfera
    // 4 cubico
    // 5 cilindrico XY
    // 6 cilindrico XZ
    // 7 cilindrico YZ
    if ( uv_mapping_type == 0) {
        float minx = bbox_min.x;
        float maxx = bbox_max.x;

        float miny = bbox_min.y;
        float maxy = bbox_max.y;

        float minz = bbox_min.z;
        float maxz = bbox_max.z;

        U = (position_model.x - minx) / (maxx - minx);
        V = (position_model.y - miny) / (maxy - miny);
    } else if ( uv_mapping_type == 1) {
        float minx = bbox_min.x;
        float maxx = bbox_max.x;

        float miny = bbox_min.y;
        float maxy = bbox_max.y;

        float minz = bbox_min.z;
        float maxz = bbox_max.z;

        U = (position_model.x - minx) / (maxx - minx);
        V = (position_model.z - minz) / (maxz - minz);
    } else if ( uv_mapping_type == 2) {
        float minx = bbox_min.x;
        float maxx = bbox_max.x;

        float miny = bbox_min.y;
        float maxy = bbox_max.y;

        float minz = bbox_min.z;
        float maxz = bbox_max.z;

        U = (position_model.y - miny) / (maxy - miny);
        V = (position_model.z - minz) / (maxz - minz);
    } else if ( uv_mapping_type == 3) {
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;

        vec4 position_relative = position_model - bbox_center;
        float theta = atan(position_relative.z, position_relative.x);
        float phi = asin(position_relative.y / length(position_relative));

        U = (theta + PI) / (2.0 * PI);
        V = (phi + (PI / 2)) / PI;
    } else if ( uv_mapping_type == 4) {
        vec3 abs_position = abs(position_model.xyz);
        if (abs_position.x >= abs_position.y && abs_position.x >= abs_position.z) {
            U = (position_model.z / abs_position.x + 1.0) * 0.5;
            V = (position_model.y / abs_position.x + 1.0) * 0.5;
        } else if (abs_position.y >= abs_position.x && abs_position.y >= abs_position.z) {
            U = (position_model.x / abs_position.y + 1.0) * 0.5;
            V = (position_model.z / abs_position.y + 1.0) * 0.5;
        } else {
            U = (position_model.x / abs_position.z + 1.0) * 0.5;
            V = (position_model.y / abs_position.z + 1.0) * 0.5;
        }
    } else if ( uv_mapping_type == 5) {
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;

        vec4 position_relative = position_model - bbox_center;
        float theta = atan(position_relative.y, position_relative.x);
        U = (theta + PI) / (2.0 * PI);
        V = position_relative.z / length(position_relative);
    } else if ( uv_mapping_type == 6) {
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;

        vec4 position_relative = position_model - bbox_center;
        float theta = atan(position_relative.z, position_relative.x);
        U = (theta + PI) / (2.0 * PI);
        V = position_relative.y / length(position_relative);
    } else if ( uv_mapping_type == 7) {
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;

        vec4 position_relative = position_model - bbox_center;
        float theta = atan(position_relative.y, position_relative.z);
        U = (theta + PI) / (2.0 * PI);
        V = position_relative.x / length(position_relative);
    }
    else 
    {
        U = texcoords.x;
        V = texcoords.y;
    }

    // =========================================== MAPEAMENTO TEXTURAS =====================================================
    if ( object_id == SKYBOX)
    {
        color.rgb = texture(TextureSkybox, vec2(U,V)).rgb;
        color.a = 1.0;
        return;
    }
    else if ( object_id == TRACK )
    {
        float repeat_factor = 50.0; 
        vec2 uv_repeated = vec2(U, V) * repeat_factor;
        Kd = texture(TextureTrack, uv_repeated).rgb;
        Ks = vec3(0.1, 0.1, 0.1); // Low specular reflectance for asphalt
        Ka = vec3(0.05, 0.05, 0.05); // Ambient reflectance
        q = 10.0; // Specular exponent for rough surface
    }
    else if (object_id == PLANE)
    {
        float repeat_factor = 100.0; 
        vec2 uv_repeated = vec2(U, V) * repeat_factor;
        Kd = texture(TextureGrass, uv_repeated).rgb;
        Ks = vec3(0.0, 0.0, 0.0);
        Ka = vec3(0.0, 0.0, 0.0);
        q = 1.0;
    }
    // CARRO
    else if ( object_id == CAR_HOOD)
    {
        Kd = texture(TextureCarHood, vec2(U,V)).rgb;
        Ks = vec3(0.0, 0.0, 0.0);
        Ka = vec3(0.0, 0.0, 0.0);
        q = 1.0;
    }
    else if ( object_id == CAR_METALIC)
    {
        Kd = texture(TextureCarMetalic, vec2(U,V)).rgb;
        // Kd = vec3(0.0, 0.0, 0.0);
        Ks = vec3(0.9, 0.9, 0.9); // High specular reflectance for metallic look
        Ka = vec3(0.1, 0.1, 0.1); // Ambient reflectance
        q = 128.0; // Specular exponent for shiny surface
    }
    else if ( object_id == CAR_PAINTING)
    {
        // float repeat_factor = 100.0; 
        // vec2 uv_repeated = vec2(U, V) * repeat_factor;
        // Kd = texture(TextureCarPainting, uv_repeated).rgb;
        Kd = vec3(0.8, 0.8, 0.8);
        Ks = vec3(0.8, 0.8, 0.8); // High specular reflectance for shiny car paint
        Ka = vec3(0.1, 0.1, 0.1); // Ambient reflectance
        q = 64.0; // Specular exponent for shiny surface
    }
    else if ( object_id == CAR_GLASS)
    {
        Kd = texture(TextureCarGlass, vec2(U,V)).rgb;
        Ks = vec3(0.9, 0.9, 0.9); // High specular reflectance for shiny glass
        Ka = vec3(0.1, 0.1, 0.1); // Ambient reflectance
        q = 128.0; // High specular exponent for shiny surface
    }
    else if ( object_id == CAR_WHEEL)
    {
        float repeat_factor = 20.0; 
        vec2 uv_repeated = vec2(U, V) * repeat_factor;
        Kd = texture(TextureCarWheel, uv_repeated).rgb;
        Ks = vec3(0.1, 0.1, 0.1); // Low specular reflectance for rubber
        Ka = vec3(0.05, 0.05, 0.05); // Ambient reflectance
        q = 10.0; // Specular exponent for rough surface
    }
    else if ( object_id == CAR_NOT_PAINTED_PARTS)
    {
        Kd = texture(TextureCarNotPaintedParts, vec2(U,V)).rgb;
        Kd = vec3(0.0588, 0.0588, 0.0588);
        Ks = vec3(0.1, 0.1, 0.1); // Low specular reflectance for rubber
        Ka = vec3(0.05, 0.05, 0.05); // Ambient reflectance
        q = 10.0; // Specular exponent for rough surface
    }
    else if ( object_id == TREE_BODY)
    {
        float repeat_factor = 30.0; 
        vec2 uv_repeated = vec2(U, V) * repeat_factor;
        Kd = texture(TextureTree, uv_repeated).rgb;
        Ks = vec3(0.2, 0.2, 0.2); // Specular color for tree body
        Ka = vec3(0.1, 0.05, 0.02); // Ambient color for tree body
        q = 10.0; // Specular exponent for rough surface
    }
    else if ( object_id == TREE_LEAVES)
    {
        Kd = vec3(0.9451, 0.549, 0.6353); // Diffuse color for cherry blossom (light pink)
        Ks = vec3(0.4, 0.4, 0.4); // Specular color for cherry blossom
        Ka = vec3(0.25, 0.25, 0.25); // Ambient color for cherry blossom
        q = 10.0; // Specular exponent for rough surface
    }
    else if ( object_id == BONUS)
    {
        Kd = texture(TextureBonus, vec2(U,V)).rgb;
        Ks = Kd; // Specular color (gold)
        Ka = vec3(0.25, 0.22, 0.06); // Ambient color (gold)
        q = 128.0; // High specular exponent for shiny surface
    }
    else if ( object_id == OUTDOOR_FACE)
    {
        Kd = texture(TextureOutdoorFace, vec2(U,V)).rgb;
        Ks = vec3(0.0, 0.0, 0.0);
        Ka = vec3(0.0, 0.0, 0.0);
        q = 1.0;
    }
    else if (object_id == OUTDOOR_POST) 
    {
        Kd = texture(TextureCarMetalic, vec2(U,V)).rgb;
        Ks = vec3(0.8, 0.8, 0.8); // High specular reflectance for metallic look
        Ka = vec3(0.1, 0.1, 0.1); // Ambient reflectance
        q = 64.0; // Specular exponent for shiny surface
    }
    else if (object_id == FINISH_LINE) 
    {
        Kd = texture(TextureFinishLine, vec2(U,V)).rgb;
        Ks = vec3(0.8, 0.8, 0.8); // High specular reflectance for metallic look
        Ka = vec3(0.1, 0.1, 0.1); // Ambient reflectance
        q = 64.0; // Specular exponent for shiny surface
    }

    // =========================================== MODELO DE ILUMINACAO =====================================================
    vec3 I = vec3(1.0, 1.0, 1.0); // espectro da fonte de luz

    vec3 Ia = vec3(0.102, 0.102, 0.098); // espectro da luz ambiente

    vec3 lambert_diffuse_term = Kd * I * max(dot(n,l), 0.0); // termo difuso de Lambert

    vec3 ambient_term = Ka * Ia; // termo ambiente

    vec3 phong_specular_term  = Ks * I * pow(max(0, dot(n, h)), q); // termo especular de blinn-Phong

    int[] lambert_objects = int[](OUTDOOR_FACE, OUTDOOR_POST, TREE_BODY, TREE_LEAVES);

    bool is_lambert_object = false;
    for (int i = 0; i < lambert_objects.length(); i++) {
        if (object_id == lambert_objects[i]) {
            is_lambert_object = true;
            break;
        }
    }
    if (is_lambert_object) 
    {
        // apenas iluminacao difusa de Lambert
        color.rgb = lambert_diffuse_term + ambient_term;
    }
    else 
    {
        // iluminacao completa de Blinn-Phong
        color.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;
    }

    // =========================================== INTERPOLACAO =====================================================
    // gouraud para o objeto BONUS
    if (object_id == BONUS || object_id == CAR_GLASS)
    {
        color.rgb = Kd * I * gouraud_lambert + ambient_term + phong_specular_term;
    }
    color.a = 1;
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
} 