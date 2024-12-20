#version 330 core

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

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define CAR    3
#define SUN    4
#define CLOUD  5
#define CAR_HOOD 6
#define CAR_GLASS 7
#define CAR_PAINTING 8
#define CAR_METALIC 9
#define CAR_WHEEL 10
#define CAR_NOT_PAINTED_PARTS 11

uniform int object_id;
uniform int uv_mapping_type;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureCarHood;
uniform sampler2D TextureCarMetalic;
uniform sampler2D TextureCarPainting;
uniform sampler2D TextureCarWheel;
uniform sampler2D TextureCarNotPaintedParts;

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

    // Parâmetros que definem as propriedades espectrais da superfície
    vec3 Kd; // Refletância difusa
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;
    
    // =========================================== MAPEAMENTO COORDENADAS UV =====================================================
    if ( uv_mapping_type == 0) {
        float minx = bbox_min.x;
        float maxx = bbox_max.x;

        float miny = bbox_min.y;
        float maxy = bbox_max.y;

        float minz = bbox_min.z;
        float maxz = bbox_max.z;

        U = (position_model.x - minx) / (maxx - minx);
        V = (position_model.z - minz) / (maxz - minz);
    }

    // =========================================== MAPEAMENTO TEXTURAS =====================================================
    if ( object_id == BUNNY)
    {
        Kd = texture(TextureCarHood, vec2(U,V)).rgb;
    }
    else if ( object_id == PLANE)
    {
        Kd = texture(TextureCarHood, vec2(U,V)).rgb;
    }
    else if ( object_id == SUN)
    {
        Kd = texture(TextureCarHood, vec2(U,V)).rgb;
    }
    else if ( object_id == CLOUD)
    {
        Kd = texture(TextureCarHood, vec2(U,V)).rgb;
    }
    else if ( object_id == CAR_HOOD)
    {
        Kd = texture(TextureCarHood, vec2(U,V)).rgb;
    }
    else if ( object_id == CAR_METALIC)
    {
        Kd = texture(TextureCarMetalic, vec2(U,V)).rgb;
    }
    else if ( object_id == CAR_PAINTING)
    {
        Kd = texture(TextureCarPainting, vec2(U,V)).rgb;
    }
    else if ( object_id == CAR_GLASS)
    {
        Kd = texture(TextureCarWheel, vec2(U,V)).rgb;
    }
    else if ( object_id == CAR_WHEEL)
    {
        Kd = texture(TextureCarWheel, vec2(U,V)).rgb;
    }
    else if ( object_id == CAR_NOT_PAINTED_PARTS)
    {
        Kd = texture(TextureCarNotPaintedParts, vec2(U,V)).rgb;
    }
    
    // ========================================================================================================================
    // Espectro da fonte de iluminação
    vec3 I = vec3(1.0, 1.0, 1.0); // PREENCHA AQUI o espectro da fonte de luz

    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2, 0.2, 0.2); // PREENCHA AQUI o espectro da luz ambiente

    // Termo difuso utilizando a lei dos cossenos de Lambert
    vec3 lambert_diffuse_term = Kd * I * max(dot(n,l), 0.0); // PREENCHA AQUI o termo difuso de Lambert

    // Termo ambiente
    vec3 ambient_term = Ka * Ia; // PREENCHA AQUI o termo ambiente

    // Termo especular utilizando o modelo de iluminação de Phong
    vec3 phong_specular_term  = Ks * I * pow(max(0, dot(r, v)), q); // PREENCH AQUI o termo especular de Phong

    // color.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;

    // MapTextures(Kd);

    color.rgb = Kd * (lambert_diffuse_term + 0.2);

    color.a = 1;
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
} 


    // TA COMENTADO PQ TENQ ATUALIZAR
    // if ( object_id == BUNNY )
    // {
    //     // PREENCHA AQUI
    //     // Propriedades espectrais do coelho
    //     Kd = vec3(0.08,0.4,0.8);
    //     Ks = vec3(0.8, 0.8,0.8);
    //     Ka = vec3(0.04,0.2,0.4);
    //     q = 32.0;
    // }
    // else if ( object_id == PLANE )
    // {
    //     // PREENCHA AQUI
    //     // Propriedades espectrais do plano
    //     Kd = vec3(0.2, 0.2, 0.2);
    //     Ks = vec3(0.3, 0.3, 0.3);
    //     Ka = vec3(0.0,0.0,0.0);
    //     q = 20.0;
    // }
    // else if ( object_id == CAR )
    // {
    //     // Propriedades espectrais do carro
    //     Kd = vec3(1.0, 0.0, 0.0); // Diffuse color (red)
    //     Ks = vec3(1.0, 0.0, 0.0); // Specular color (red)
    //     Ka = vec3(0.1, 0.0, 0.0); // Ambient color (dim red)
    //     q = 20.0;
    // }
    // else if ( object_id == SUN )
    // {
    //     // Propriedades espectrais do plano
    //     Kd = vec3(1.0, 1.0, 0.0); // Diffuse color (yellow)
    //     Ks = vec3(1.0, 1.0, 0.0); // Specular color (yellow)
    //     Ka = vec3(0.1, 0.1, 0.0); // Ambient color (dim yellow)
    //     q = 20.0;
    // }
    // else if ( object_id == CLOUD )
    // {
    //     // Propriedades espectrais da nuvem
    //     Kd = vec3(1.0, 1.0, 1.0); // Diffuse color (white)
    //     Ks = vec3(1.0, 1.0, 1.0); // Specular color (white)
    //     Ka = vec3(0.1, 0.1, 0.1); // Ambient color (dim white)
    //     q = 20.0;
    // }
    // else // Objeto desconhecido = preto
    // {
    //     Kd = vec3(0.0,0.0,0.0);
    //     Ks = vec3(0.0,0.0,0.0);
    //     Ka = vec3(0.0,0.0,0.0);
    //     q = 1.0;
    // }
