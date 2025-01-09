#include "collisions.h"
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <cstdio>

float tree_radius = 0.27f;       
float outdoor_radius = 0.2f;
float bonus_radius = 0.1f; 

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


/*    
        carro com linha de chegada (cubo x plano)
        carro com pista (cubo x sla oq (deve ser plano, essa vai ser foda) (talvez tu pode tentar com a grama que nao vai ta em contato direto))
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
bool cube_cilinder_intersect_tree(glm::vec3 min, glm::vec3 max){
    for (const auto& pos : tree_positions) {
        if(cube_cilinder_intersect(min, max, pos, tree_radius)){
            return true;
        }
    }
    return false;
}

// carro com outdoor -> outdoor_post1/2 (cubo x cilindro)
bool cube_cilinder_intersect_outdoor(glm::vec3 min, glm::vec3 max){
    for (const auto& pos : outdoor_positions) {
        if(cube_cilinder_intersect(min, max, pos, outdoor_radius)){
            return true;
        }
    }
    return false;
}

// carro com linha de chegada (ponto x cubo)
bool point_cube_intersect(glm::vec3 point, glm::vec3 min, glm::vec3 max) {
    return (point.x >= min.x && point.x <= max.x) &&
           (point.y >= min.y && point.y <= max.y) &&
           (point.z >= min.z && point.z <= max.z);
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

bool cube_sphere_intersect_bonus(glm::vec3 min, glm::vec3 max, glm::vec3 pos){
        if(cube_sphere_intersect(min, max, pos, bonus_radius)) {
            return true;
        }
    return false;
}


// carro com pista (cubo x sla oq (deve ser plano, essa vai ser foda) (talvez tu pode tentar com a grama que nao vai ta em contato direto))

