#include "noise.h"

glm::vec2 smoothF(glm::vec2 uv)
{
    return uv*uv*(3.f-2.f*uv);
}


float noise(glm::vec2 uv)
{
    const float k = 257.;
    glm::vec4 l  = glm::vec4(glm::floor(uv),glm::fract(uv));
    float u = l.x + l.y * k;
    glm::vec4 v = glm::vec4(u, u+1.,u+k, u+k+1.);
    v = glm::fract(glm::fract(1.23456789f * v) * v / .987654321f);
    glm::vec2 smoothed = smoothF(glm::vec2(l.z, l.w));
    l.z = smoothed[0];
    l.w = smoothed[1];
    l.x = glm::mix(v.x, v.y, l.z);
    l.y = glm::mix(v.z, v.w, l.z);
    return glm::mix(l.x, l.y, l.w);
}

float fbm(const glm::vec2 uv)
{
    float a = 0.5;
    float f = 5.0;
    float n = 0.;
    int it = 8;
    for(int i = 0; i < 32; i++)
    {
        if(i<it)
        {
            n += noise(uv*f)*a;
            a *= .5;
            f *= 2.;
        }
    }
    return n;
}

glm::vec2 random2(glm::vec2 p) {
    return glm::normalize(
        2.f *
            glm::fract(
                glm::sin(
                    glm::vec2(glm::dot(p,glm::vec2(127.1,311.7)),
                              glm::dot(p,glm::vec2(269.5,183.3))))
                *43758.5453f)
        - 1.f
        );
}

float surflet2D(glm::vec2 P, glm::vec2 gridPoint)
{
    // Compute falloff function by converting linear distance to a polynomial (quintic smootherstep function)
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.0) + 15 * pow(distX, 4.0) - 10 * pow(distX, 3.0);
    float tY = 1 - 6 * pow(distY, 5.0) + 15 * pow(distY, 4.0) - 10 * pow(distY, 3.0);

    // Get the random vector for the grid point
    glm::vec2 gradient = random2(gridPoint);
    // Get the vector from the grid point to P
    glm::vec2 diff = P - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = glm::dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;
}

float PerlinNoise2D(glm::vec2 uv)
{
    // Tile the space
    glm::vec2 uvXLYL = glm::floor(uv);
    glm::vec2 uvXHYL = uvXLYL + glm::vec2(1,0);
    glm::vec2 uvXHYH = uvXLYL + glm::vec2(1,1);
    glm::vec2 uvXLYH = uvXLYL + glm::vec2(0,1);

    return surflet2D(uv, uvXLYL) + surflet2D(uv, uvXHYL) + surflet2D(uv, uvXHYH) + surflet2D(uv, uvXLYH);
}

glm::vec3 random3(glm::vec3 p) {
    return glm::normalize(
        2.f *
            glm::fract(
                glm::sin(
                    glm::vec3(glm::dot(p,glm::vec3(127.1,311.7, 419.2)),
                              glm::dot(p,glm::vec3(269.5,183.3, 371.9)),
                              glm::dot(p,glm::vec3(314.9, 624.7, 582.3))))
                *43758.5453f)
        - 1.f
        );
}

float surflet3D(glm::vec3 p, glm::vec3 gridPoint) {
    glm::vec3 t2 = glm::abs(p - gridPoint);

    glm::vec3 t = (
        glm::vec3(1.f) -
        6.f * glm::vec3(glm::pow(t2.x, 5.f), glm::pow(t2.y, 5.f), glm::pow(t2.z, 5.f)) +
        15.f * glm::vec3(glm::pow(t2.x, 4.f), glm::pow(t2.y, 4.f), glm::pow(t2.z, 4.f)) -
        10.f * glm::vec3(glm::pow(t2.x, 3.f), glm::pow(t2.y, 3.f), glm::pow(t2.z, 3.f))
    );

    glm::vec3 gradient = random3(gridPoint) * 2.f - glm::vec3(1., 1., 1.);
    glm::vec3 diff = p - gridPoint;
    float height = glm::dot(diff, gradient);
    return height * t.x * t.y * t.z;
}

float PerlinNoise3D(glm::vec3 p) {
    float surfletSum = 0.f;
    for(int dx = 0; dx <= 1; ++dx) {
        for(int dy = 0; dy <= 1; ++dy) {
            for(int dz = 0; dz <= 1; ++dz) {
                surfletSum += surflet3D(p, glm::floor(p) + glm::vec3(dx, dy, dz));
            }
        }
    }
    return surfletSum;

}
