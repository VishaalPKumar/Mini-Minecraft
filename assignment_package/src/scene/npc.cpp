#include "npc.h"
#include <QString>
#include <iostream>

NPC::NPC(glm::vec3 pos, Terrain &terrain, Mesh &mesh)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
    m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
    creeperState(),
    m_mesh(mesh),
    width(2.f), length(2.f), height(3.f),
    mcr_camera(m_camera)
{}

NPC::~NPC()
{}

static void randomFlip(InputBundle &inputs, bool *keepGoing) {
    // Seed the random number generator
    std::srand(std::time(nullptr));

    // Collect references to all variables in a vector
    std::vector<bool*> keys = {&inputs.wPressed, &inputs.aPressed, &inputs.sPressed, &inputs.dPressed,
                                &inputs.spacePressed, &inputs.spacePressed, &inputs.spacePressed, &inputs.spacePressed};

    // Run indefinitely
    while (*keepGoing) {
        // Randomly select a variable to flip
        int index = std::rand() % keys.size();
        *keys[index] = !*keys[index];  // Flip the selected variable

        // Sleep for random number of seconds
        if (keys[index] == &inputs.spacePressed) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        } else {
            int seconds = std::rand() % 4 + 1;
            std::this_thread::sleep_for(std::chrono::seconds(seconds));
        }

        // Reset all variables to false
        for (bool* b : keys) {
            *b = false;
        }

    }
}

void NPC::tick(float dT, InputBundle &input) {
    processInputs(m_inputs);
    computePhysics(dT, mcr_terrain);
}

void NPC::processInputsFlightMode(InputBundle &inputs) {
    if (inputs.wPressed) m_acceleration += ACC_AMT * glm::normalize(m_forward);
    if (inputs.sPressed) m_acceleration -= ACC_AMT * glm::normalize(m_forward);
    if (inputs.dPressed) m_acceleration += ACC_AMT * glm::normalize(m_right);
    if (inputs.aPressed) m_acceleration -= ACC_AMT * glm::normalize(m_right);
    if (inputs.ePressed) m_acceleration += ACC_AMT * glm::normalize(m_up);
    if (inputs.qPressed) m_acceleration -= ACC_AMT * glm::normalize(m_up);
}

void NPC::processInputsNormalMode(InputBundle &inputs) {
    glm::vec3 forwardXZ = glm::normalize(glm::vec3(m_forward.x, 0, m_forward.z));
    glm::vec3 rightXZ = glm::normalize(glm::vec3(m_right.x, 0, m_right.z));

    if (inputs.wPressed) m_acceleration += ACC_AMT * forwardXZ;
    if (inputs.sPressed) m_acceleration -= ACC_AMT * forwardXZ;
    if (inputs.dPressed) m_acceleration += ACC_AMT * rightXZ;
    if (inputs.aPressed) m_acceleration -= ACC_AMT * rightXZ;

    if (inputs.spacePressed) {
        if (creeperState.onGround) {
            m_velocity.y += JUMP_FORCE;
        } else if (creeperState.inLava || creeperState.inWater) {
            m_velocity.y = FLOATING_SPEED;
        }
    }
}

void NPC::moveCamera(float dx, float dy) {
    rotateOnUpGlobal(-dx * MOUSE_SENSITIVITY);
    rotateOnRightLocal(-dy * MOUSE_SENSITIVITY);
}

void NPC::processInputs(InputBundle &inputs) {

    // Default player acceleration to 0
    m_acceleration = glm::vec3(0);

    if (creeperState.flightMode) {
        processInputsFlightMode(inputs);
    } else {
        processInputsNormalMode(inputs);
    }

    if (inputs.fPressed) {
        creeperState.flightMode = !creeperState.flightMode;
        inputs.fPressed = false;
        if (creeperState.flightMode) m_velocity = glm::vec3(0);
    }
}

void NPC::setPos(glm::vec3 position) {
    m_position = position;
}

void NPC::startRandomMotion() {
    creeperState.flightMode = false; // make sure we're not in flight mode
    m_isRandomlyMoving = true;

    std::thread flipThread(randomFlip, std::ref(m_inputs), &m_isRandomlyMoving);
    flipThread.detach();
}

void NPC::stopRandomMotion() {
    m_isRandomlyMoving = false;
}

void NPC::computePhysics(float dT, const Terrain &terrain) {
    groundCheck(terrain);
    waterLavaCheck(terrain);
    // Gravity
    if (!creeperState.flightMode && !creeperState.onGround) {
        m_acceleration -= glm::vec3(0, GRAVITY, 0);
    }
    dT /= 1000;
    m_velocity += m_acceleration * dT;
    m_velocity *= 0.80f;
    glm::vec3 moveDir = m_velocity * dT;

    if (creeperState.flightMode) {
        moveAlongVector(moveDir);
    } else {
        if (creeperState.inLava || creeperState.inWater) {
            m_velocity *= (0.70);
        }
        moveWithCollisions(moveDir, terrain);
    }
}

void NPC::moveWithCollisions(glm::vec3 dir, const Terrain &terrain) {
    float minXDist = glm::abs(dir.x) - COLLISION_OFFSET;
    float minYDist = glm::abs(dir.y) - COLLISION_OFFSET;
    float minZDist = glm::abs(dir.z) - COLLISION_OFFSET;
    float out_dist;
    glm::ivec3 out_blockHit;
    float axis;
    std::vector<glm::vec3> corners = getPlayerCorners(true, true, true);

    bool collided = false;

    for (auto& corner : corners) {
        // x axis
        glm::vec3 dirX = glm::vec3(dir.x, 0, 0);
        collided &= gridMarch(corner, dirX, terrain, &out_dist, &out_blockHit, &axis, isSolidBlock);
        minXDist = std::max(std::min(out_dist - COLLISION_OFFSET, minXDist), 0.f);
        // y axis
        glm::vec3 dirY = glm::vec3(0, dir.y, 0);
        collided &= gridMarch(corner, dirY, terrain, &out_dist, &out_blockHit, &axis, isSolidBlock);
        minYDist = std::max(std::min(out_dist - COLLISION_OFFSET, minYDist), 0.f);
        // z axis
        glm::vec3 dirZ = glm::vec3(0, 0, dir.z);
        collided &= gridMarch(corner, dirZ, terrain, &out_dist, &out_blockHit, &axis, isSolidBlock);
        minZDist = std::max(std::min(out_dist - COLLISION_OFFSET, minZDist), 0.f);
    }

    moveAlongVector(glm::vec3(minXDist * glm::sign(dir.x),
                              minYDist * glm::sign(dir.y),
                              minZDist * glm::sign(dir.z)));

    if (collided) {
        m_inputs.spacePressed = true;
    } else {
        m_inputs.spacePressed = false;
    }
}

void NPC::groundCheck(const Terrain &terrain) {
    if (m_velocity.y > 0) {
        creeperState.onGround = false;
        return;
    }

    std::vector<glm::vec3> bottomCorners = getPlayerCorners(true, false, false);
    for (auto& corner : bottomCorners) {
        glm::vec3 rayDirection = glm::vec3(0, -0.00001f, 0);
        float out_dist;
        glm::ivec3 out_blockHit;
        float axis;
        if (gridMarch(corner, rayDirection, terrain, &out_dist, &out_blockHit, &axis, isSolidBlock)) {
            creeperState.onGround = true;
            return;
        }
    }
    creeperState.onGround = false;
}

void NPC::waterLavaCheck(const Terrain &terrain) {

    float out_dist;
    glm::ivec3 out_blockHit;
    float axis;
    if (gridMarch(m_camera.mcr_position, glm::vec3(0.0001f, 0, 0), terrain, &out_dist, &out_blockHit, &axis, isWaterLavaBlock) ||
        gridMarch(m_camera.mcr_position, glm::vec3(-0.0001f, 0, 0), terrain, &out_dist, &out_blockHit, &axis, isWaterLavaBlock) ||
        gridMarch(m_camera.mcr_position, glm::vec3(0, 0, 0.0001f), terrain, &out_dist, &out_blockHit, &axis, isWaterLavaBlock) ||
        gridMarch(m_camera.mcr_position, glm::vec3(0, 0, -0.0001f), terrain, &out_dist, &out_blockHit, &axis, isWaterLavaBlock)) {
        BlockType type = mcr_terrain.getGlobalBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z);
        creeperState.inLava = type == LAVA;
        creeperState.inWater = type == WATER;
        return;
    }

    creeperState.inLava = false;
    creeperState.inWater = false;
}


bool NPC::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection,
                       const Terrain &terrain, float *out_dist,
                       glm::ivec3 *out_blockHit, float *axis,
                       HitConditionFunction2 hitCondition) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;
    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t;
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain.getGlobalBlockAt(currCell.x, currCell.y, currCell.z);
        if(hitCondition(cellType)) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            *axis = interfaceAxis;
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}

std::vector<glm::vec3> NPC::getPlayerCorners(bool includeBottom, bool includeTop, bool includeMiddle) {
    std::vector<glm::vec3> corners;

    if (includeBottom) {
        for (float x = -width / 2; x <= width / 2; x += width) {
            for (float z = -length / 2; z <= length / 2; z += length) {
                corners.push_back(m_position + glm::vec3(x, 0, z));
            }
        }
    }

    if (includeTop) {
        for (float x = -width / 2; x <= width / 2; x += width) {
            for (float z = -length / 2; z <= length / 2; z += length) {
                corners.push_back(m_position + glm::vec3(x, height, z));
            }
        }
    }

    if (includeMiddle) {
        for (float x = -width / 2; x <= width / 2; x += width) {
            for (float z = -length / 2; z <= length / 2; z += length) {
                corners.push_back(m_position + glm::vec3(x, height / 2, z));
            }
        }
    }
    return corners;
}

glm::vec3 NPC::getPos() {
    return m_position;
}


void NPC::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

CreeperState NPC::getPlayerState() {
    return creeperState;
}

