#include "collisions.h"
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <cstdio>

std::vector<glm::vec3> tree_positions = {
    glm::vec3(6.0f,-1.0f,-8.0f), 
    glm::vec3(-6.0f,-1.0f,-8.0f),
    glm::vec3(78.0f, -1.0f, -74.0f),
    glm::vec3(96.0f, -1.0f, -74.0f),
    glm::vec3(17.0f, -1.0f, -70.0f),
    glm::vec3(41.0f, -1.0f, -42.0f),
    glm::vec3(47.0f, -1.0f, -4.0f),
    glm::vec3(39.0f, -1.0f, 31.0f),
    glm::vec3(10.0f, -1.0f, 35.0f)
};

std::vector<glm::vec3> outdoor_positions = {
    glm::vec3(6.0f, 0.0f, -30.0f),
    glm::vec3(-6.0f, 0.0f, -30.0f),
    glm::vec3(31.68f, 0.0f, -100.0f),
    glm::vec3(28.32f, 0.0f, -100.0f),
    glm::vec3(22.0f, 0.0f, -42.32f),
    glm::vec3(22.0f, 0.0f, -45.68f),
    glm::vec3(-1.43f, 0.0f, 53.57f),
    glm::vec3(1.43f, 0.0f, 56.43f)
};

std::vector<glm::vec3> bonus_positions = {
    glm::vec3(16.0f, -0.9f, -89.0f), // curva 1
    glm::vec3(90.0f, -0.9f, -74.0f), // curva 2
    glm::vec3(27.0f, -0.9f, -44.0f), // curva 3
    glm::vec3(63.0f, -0.9f, -4.0f), // curva 4
    glm::vec3(10.0f, -0.9f, 53.0f), // curva 5
    // glm::vec3(0.0f, -0.9f, -2.0f) // posicao padrao
};


/*    
    implementar colisoes
        carro com arvore -> tree_body (cubo x cilindro)
        carro com outdoor -> outdoor_post1/2 (cubo x cilindro)
        carro com linha de chegada (cubo x plano)
        carro com pista (cubo x sla oq (deve ser plano, essa vai ser foda) (talvez tu pode tentar com a grama que nao vai ta em contato direto))
        carro com objeto bonus (cubo x esfera) 
*/

bool cube_cilinder_intersect(glm::vec3 min, glm::vec3 max, glm::vec3 center, float radius){
    float closest_x = std::max(min.x, std::min(center.x, max.x));
    float closest_y = std::max(min.y, std::min(center.y, max.y));
    float closest_z = std::max(min.z, std::min(center.z, max.z));

    glm::vec3 closest_point = glm::vec3(closest_x, closest_y, closest_z);

    float distance = glm::distance(glm::vec3(center.x, center.y, center.z), closest_point);

    if(distance <= radius){
        return true;
    }

    return false;
}

// carro com arvore -> tree_body (cubo x cilindro)
bool cube_cilinder_intersect_tree(glm::vec3 min, glm::vec3 max, float radius){
    for (const auto& pos : tree_positions) {
        if(cube_cilinder_intersect(min, max, pos, radius)){
            return true;
        }
    }
    return false;
}

// carro com outdoor -> outdoor_post1/2 (cubo x cilindro)
bool cube_cilinder_intersect_outdoor(glm::vec3 min, glm::vec3 max, float radius){
    for (const auto& pos : outdoor_positions) {
        if(cube_cilinder_intersect(min, max, pos, radius)){
            return true;
        }
    }
    return false;
}

// carro com linha de chegada (cubo x plano)
bool cube_plane_intersect(glm::vec3 normal, glm::vec3 min, glm::vec3 max){
    glm::vec3 vertices[8] = {
        glm::vec3(min.x, min.y, min.z),
        glm::vec3(min.x, min.y, max.z),
        glm::vec3(min.x, max.y, min.z),
        glm::vec3(min.x, max.y, max.z),
        glm::vec3(max.x, min.y, min.z),
        glm::vec3(max.x, min.y, max.z),
        glm::vec3(max.x, max.y, min.z),
        glm::vec3(max.x, max.y, max.z)
    };

    float min_distance = std::numeric_limits<float>::max();
    float max_distance = std::numeric_limits<float>::lowest();

    for (int i = 0; i < 8; i++){
        float distance = glm::dot(normal, vertices[i]);
        min_distance = std::min(min_distance, distance);
        max_distance = std::max(max_distance, distance);
    }

    return (min_distance <= 0.0f) && (max_distance >= 0.0f);
}


// carro com objeto bonus (cubo x esfera) 
bool cube_sphere_intersect(glm::vec3 min, glm::vec3 max, glm::vec3 center, float radius){
    float closest_x = std::max(min.x, std::min(center.x, max.x));
    float closest_y = std::max(min.y, std::min(center.y, max.y));
    float closest_z = std::max(min.z, std::min(center.z, max.z));

    glm::vec3 closest_point = glm::vec3(closest_x, closest_y, closest_z);

    float distance = glm::distance(closest_point, center);

    return (distance <= radius);
}


// carro com pista (cubo x sla oq (deve ser plano, essa vai ser foda) (talvez tu pode tentar com a grama que nao vai ta em contato direto))

