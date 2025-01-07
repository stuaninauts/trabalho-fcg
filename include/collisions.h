
#ifndef COLLISIONS_H
#define COLLISIONS_H

#include <glm/glm.hpp>

bool cube_cilinder_intersect(glm::vec3 min, glm::vec3 max, glm::vec3 center, float radius);

bool point_cube_intersect(glm::vec3 point, glm::vec3 min, glm::vec3 max); 

bool cube_sphere_intersect(glm::vec3 min, glm::vec3 max, glm::vec3 center, float radius);

bool cube_cilinder_intersect_tree(glm::vec3 min, glm::vec3 max);

bool cube_cilinder_intersect_outdoor(glm::vec3 min, glm::vec3 max);

bool cube_sphere_intersect_bonus(glm::vec3 min, glm::vec3 max, glm::vec3 pos);

#endif