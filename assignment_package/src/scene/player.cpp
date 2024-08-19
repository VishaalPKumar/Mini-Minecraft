#include "player.h"
#include <QString>
#include <iostream>

Player::Player(glm::vec3 pos, Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
    m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
    playerState(), width(1.f), length(1.f), height(2.f),
    mcr_camera(m_camera), playerInventory()
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain);
}

void Player::processInputsFlightMode(InputBundle &inputs) {
    if (inputs.wPressed) m_acceleration += ACC_AMT * glm::normalize(m_forward);
    if (inputs.sPressed) m_acceleration -= ACC_AMT * glm::normalize(m_forward);
    if (inputs.dPressed) m_acceleration += ACC_AMT * glm::normalize(m_right);
    if (inputs.aPressed) m_acceleration -= ACC_AMT * glm::normalize(m_right);
    if (inputs.ePressed) m_acceleration += ACC_AMT * glm::normalize(m_up);
    if (inputs.qPressed) m_acceleration -= ACC_AMT * glm::normalize(m_up);
}

void Player::processInputsNormalMode(InputBundle &inputs) {
    glm::vec3 forwardXZ = glm::normalize(glm::vec3(m_forward.x, 0, m_forward.z));
    glm::vec3 rightXZ = glm::normalize(glm::vec3(m_right.x, 0, m_right.z));

    if (inputs.wPressed) m_acceleration += ACC_AMT * forwardXZ;
    if (inputs.sPressed) m_acceleration -= ACC_AMT * forwardXZ;
    if (inputs.dPressed) m_acceleration += ACC_AMT * rightXZ;
    if (inputs.aPressed) m_acceleration -= ACC_AMT * rightXZ;

    if (inputs.spacePressed) {
        if (playerState.onGround) {
            m_velocity.y += JUMP_FORCE;
        } else if (playerState.inLava || playerState.inWater) {
            m_velocity.y = FLOATING_SPEED;
        }
    }
}

void Player::moveCamera(float dx, float dy) {
    rotateOnUpGlobal(-dx * MOUSE_SENSITIVITY);
    rotateOnRightLocal(-dy * MOUSE_SENSITIVITY);
}

void Player::processInputs(InputBundle &inputs) {

    // Default player acceleration to 0
    m_acceleration = glm::vec3(0);

    if (playerState.flightMode) {
        processInputsFlightMode(inputs);
    } else {
        processInputsNormalMode(inputs);
    }

    if (inputs.fPressed) {
        playerState.flightMode = !playerState.flightMode;
        inputs.fPressed = false;
        if (playerState.flightMode) m_velocity = glm::vec3(0);
    }
}

void Player::computePhysics(float dT, const Terrain &terrain) {
    groundCheck(terrain);
    waterLavaCheck(terrain);
    // Gravity
    if (!playerState.flightMode && !playerState.onGround) {
        m_acceleration -= glm::vec3(0, GRAVITY, 0);
    }
    dT /= 1000;
    m_velocity += m_acceleration * dT;
    m_velocity *= 0.80f;
    glm::vec3 moveDir = m_velocity * dT;

    if (playerState.flightMode) {
        moveAlongVector(moveDir);
    } else {
        if (playerState.inLava || playerState.inWater) {
            m_velocity *= (0.70);
        }
        moveWithCollisions(moveDir, terrain);
    }
}

void Player::moveWithCollisions(glm::vec3 dir, const Terrain &terrain) {
    float minXDist = glm::abs(dir.x) - COLLISION_OFFSET;
    float minYDist = glm::abs(dir.y) - COLLISION_OFFSET;
    float minZDist = glm::abs(dir.z) - COLLISION_OFFSET;
    float out_dist;
    glm::ivec3 out_blockHit;
    float axis;
    std::vector<glm::vec3> corners = getPlayerCorners(true, true, true);

    for (auto& corner : corners) {
        // x axis
        glm::vec3 dirX = glm::vec3(dir.x, 0, 0);
        gridMarch(corner, dirX, terrain, &out_dist, &out_blockHit, &axis, isSolidBlock);
        minXDist = std::max(std::min(out_dist - COLLISION_OFFSET, minXDist), 0.f);
        // y axis
        glm::vec3 dirY = glm::vec3(0, dir.y, 0);
        gridMarch(corner, dirY, terrain, &out_dist, &out_blockHit, &axis, isSolidBlock);
        minYDist = std::max(std::min(out_dist - COLLISION_OFFSET, minYDist), 0.f);
        // z axis
        glm::vec3 dirZ = glm::vec3(0, 0, dir.z);
        gridMarch(corner, dirZ, terrain, &out_dist, &out_blockHit, &axis, isSolidBlock);
        minZDist = std::max(std::min(out_dist - COLLISION_OFFSET, minZDist), 0.f);
    }

    moveAlongVector(glm::vec3(minXDist * glm::sign(dir.x),
                              minYDist * glm::sign(dir.y),
                              minZDist * glm::sign(dir.z)));
}

void Player::groundCheck(const Terrain &terrain) {
    if (m_velocity.y > 0) {
        playerState.onGround = false;
        return;
    }

    std::vector<glm::vec3> bottomCorners = getPlayerCorners(true, false, false);
    for (auto& corner : bottomCorners) {
        glm::vec3 rayDirection = glm::vec3(0, -0.00001f, 0);
        float out_dist;
        glm::ivec3 out_blockHit;
        float axis;
        if (gridMarch(corner, rayDirection, terrain, &out_dist, &out_blockHit, &axis, isSolidBlock)) {
            playerState.onGround = true;
            return;
        }
    }
    playerState.onGround = false;
}

void Player::waterLavaCheck(const Terrain &terrain) {

        float out_dist;
        glm::ivec3 out_blockHit;
        float axis;
        if (gridMarch(m_camera.mcr_position, glm::vec3(0.0001f, 0, 0), terrain, &out_dist, &out_blockHit, &axis, isWaterLavaBlock) ||
            gridMarch(m_camera.mcr_position, glm::vec3(-0.0001f, 0, 0), terrain, &out_dist, &out_blockHit, &axis, isWaterLavaBlock) ||
            gridMarch(m_camera.mcr_position, glm::vec3(0, 0, 0.0001f), terrain, &out_dist, &out_blockHit, &axis, isWaterLavaBlock) ||
            gridMarch(m_camera.mcr_position, glm::vec3(0, 0, -0.0001f), terrain, &out_dist, &out_blockHit, &axis, isWaterLavaBlock)) {
            BlockType type = mcr_terrain.getGlobalBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z);
            playerState.inLava = type == LAVA;
            playerState.inWater = type == WATER;
            return;
        }

    playerState.inLava = false;
    playerState.inWater = false;
}


bool Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection,
                       const Terrain &terrain, float *out_dist,
                       glm::ivec3 *out_blockHit, float *axis,
                       HitConditionFunction hitCondition) {
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

std::vector<glm::vec3> Player::getPlayerCorners(bool includeBottom, bool includeTop, bool includeMiddle) {
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

glm::vec3 Player::getPos() {
    return m_position;
}

bool Player::blockWithinRange(float *out_dist, glm::ivec3 *out_blockHit, float *axis) {
    glm::vec3 rayOrigin = m_camera.mcr_position;
    glm::vec3 rayDirection = BLOCK_ACTION_RANGE * glm::normalize(m_forward);

    return gridMarch(rayOrigin, rayDirection, mcr_terrain, out_dist, out_blockHit, axis,
                     [](BlockType cellType) {
                         return cellType != BEDROCK && cellType != EMPTY && cellType != WATER && cellType != LAVA;
                     });
}

void Player::destroyBlock() {
    float out_dist;
    glm::ivec3 out_blockHit;
    float axis;

    if (blockWithinRange(&out_dist, &out_blockHit, &axis)) {
        if (mcr_terrain.getGlobalBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z) == BlockType::BEDROCK) return;
        // Add block to inventory
        playerInventory.addBlockToInventory(mcr_terrain.getGlobalBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z));

        mcr_terrain.setGlobalBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z, BlockType::EMPTY);
        uPtr<Chunk> &c = mcr_terrain.getChunkAt(out_blockHit.x, out_blockHit.z);
        c->destroyVBOdata();
        c->createAndBufferVBOData();

    }
}

void Player::createBlock() {
    BlockType blockType = playerInventory.selectedBlock;
    if (playerInventory.getBlockCount(blockType) == 0) return;

    float out_dist;
    glm::ivec3 out_blockHit;
    float axis;

    if (blockWithinRange(&out_dist, &out_blockHit, &axis)) {
        switch ((int) axis) {
        case 0:
            out_blockHit.x -= glm::sign(m_forward.x);
            break;
        case 1:
            out_blockHit.y -= glm::sign(m_forward.y);
            break;
        case 2:
            out_blockHit.z -= glm::sign(m_forward.z);
            break;
        default:
            break;
        }
        playerInventory.removeBlockFromInventory();
        mcr_terrain.setGlobalBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z, blockType);
        uPtr<Chunk> &c = mcr_terrain.getChunkAt(out_blockHit.x, out_blockHit.z);
        c->destroyVBOdata();
        c->createAndBufferVBOData();
    }
}


void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
QString Player::stateAsQString() const {
    std::vector<std::string> stateStrs;
    if (playerState.flightMode) stateStrs.push_back("Flight Mode");
    if (playerState.onGround) stateStrs.push_back("On Ground");
    if (playerState.inLava) stateStrs.push_back("In Lava");
    if (playerState.inWater) stateStrs.push_back("In Water");
    if (stateStrs.empty()) stateStrs.push_back("Normal Mode");
    std::string str;
    for (size_t i = 0; i < stateStrs.size(); ++i) {
        str += stateStrs[i];
        if (i != stateStrs.size() - 1) str += ", ";
    }
    return QString::fromStdString(str);
}

PlayerState Player::getPlayerState() {
    return playerState;
}

PlayerInventory& Player::getPlayerInventory() {
    return playerInventory;
}

QString Player::inventoryAsQString() const {
    return playerInventory.asQString();
}
