#pragma once
#include "glm_includes.h"

glm::vec2 smoothF(glm::vec2);
float noise2D(glm::vec2);
float fbm(const glm::vec2);

glm::vec2 random2(glm::vec2);
float surflet2D(glm::vec2, glm::vec2);
float PerlinNoise2D(glm::vec2);

glm::vec3 random3(glm::vec3);
float surflet3D(glm::vec3, glm::vec3);
float PerlinNoise3D(glm::vec3);


