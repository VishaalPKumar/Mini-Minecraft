#pragma once
#include "entity.h"
#include "camera.h"
#include "terrain.h"
#include "mesh.h"

using HitConditionFunction2 = std::function<bool(BlockType)>;

struct CreeperState {
    bool flightMode, onGround, inLava, inWater;
    CreeperState()
        : flightMode(true), onGround(false), inLava(false), inWater(false)
    {}
};

class NPC : public Entity {
private:
    glm::vec3 m_velocity, m_acceleration;
    Camera m_camera;
    Terrain &mcr_terrain;
    CreeperState creeperState;
    float width, length, height;
    bool m_isRandomlyMoving;
    InputBundle m_inputs;

    void processInputs(InputBundle &inputs);
    void computePhysics(float dT, const Terrain &terrain);

public:
    // Readonly public reference to our camera
    // for easy access from MyGL
    const Camera& mcr_camera;
    Mesh &m_mesh;

    NPC(glm::vec3 pos, Terrain &terrain, Mesh &mesh);
    virtual ~NPC() override;

    void setCameraWidthHeight(unsigned int w, unsigned int h);

    void tick(float dT, InputBundle &input) override;

    void startRandomMotion();
    void stopRandomMotion();

    // Movement Operations
    void processInputsNormalMode(InputBundle &inputs);
    void processInputsFlightMode(InputBundle &inputs);

    // Camera
    void moveCamera(float dx, float dy);

    //Grid March
    bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection,
                   const Terrain &terrain, float *out_dist,
                   glm::ivec3 *out_blockHit, float *axis,
                   HitConditionFunction2);

    // Collision Checking
    void moveWithCollisions(glm::vec3 dir, const Terrain &terrain);
    void groundCheck(const Terrain &terrain);

    // Water | Lava Checking
    void waterLavaCheck(const Terrain &terrain);

    CreeperState getPlayerState();

    // Helper function to get corner points
    std::vector<glm::vec3> getPlayerCorners(bool includeBottom, bool includeTop, bool includeMiddle);

    glm::vec3 getPos();

    void setPos(glm::vec3 position);

};
