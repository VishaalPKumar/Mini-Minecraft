#pragma once
#include "entity.h"
#include "camera.h"
#include "terrain.h"

using HitConditionFunction = std::function<bool(BlockType)>;

struct PlayerState {
    bool flightMode, onGround, inLava, inWater;
    PlayerState()
        : flightMode(true), onGround(false), inLava(false), inWater(false)
    {}
};

struct PlayerInventory {
    std::unordered_map<BlockType, int, EnumHash> inventory;
    BlockType selectedBlock;
    PlayerInventory()
        : selectedBlock(BlockType::GRASS)
    {
        inventory[BlockType::GRASS] = 10;
        inventory[BlockType::DIRT] = 10;
        inventory[BlockType::STONE] = 10;
        inventory[BlockType::WATER] = 10;
        inventory[BlockType::SNOW] = 10;
        inventory[BlockType::SAND] = 10;
        inventory[BlockType::LAVA] = 10;
    }

    void addBlockToInventory(BlockType blockType) {
        if (inventory.find(blockType) != inventory.end()) {
            inventory[blockType]++;
        }
    }

    void removeBlockFromInventory() {
        if (inventory.find(selectedBlock) != inventory.end()) {
            inventory[selectedBlock]--;
        }
    }

    int getBlockCount(BlockType blockType) {
        if (inventory.find(blockType) != inventory.end()) {
            return inventory[blockType];
        }
        return 0;
    }

    void setSelectedBlock(BlockType blockType) {
        selectedBlock = blockType;
    }


    QString asQString() const {
        QString str;
        for (auto &pair : inventory) {
            str += QString::number(static_cast<int>(pair.first)) + ":" + QString::number(pair.second) + ",";
        }
        return str;
    }
};

class Player : public Entity {
private:
    glm::vec3 m_velocity, m_acceleration;
    Camera m_camera;
    Terrain &mcr_terrain;
    PlayerState playerState;
    PlayerInventory playerInventory;
    float width, length, height;

    void processInputs(InputBundle &inputs);
    void computePhysics(float dT, const Terrain &terrain);


public:
    // Readonly public reference to our camera
    // for easy access from MyGL
    const Camera& mcr_camera;

    Player(glm::vec3 pos, Terrain &terrain);
    virtual ~Player() override;

    void setCameraWidthHeight(unsigned int w, unsigned int h);

    void tick(float dT, InputBundle &input) override;

    // Movement Operations
    void processInputsNormalMode(InputBundle &inputs);
    void processInputsFlightMode(InputBundle &inputs);

    // Camera
    void moveCamera(float dx, float dy);

    //Grid March
    bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection,
                   const Terrain &terrain, float *out_dist,
                   glm::ivec3 *out_blockHit, float *axis,
                   HitConditionFunction);

    // Collision Checking
    void moveWithCollisions(glm::vec3 dir, const Terrain &terrain);
    void groundCheck(const Terrain &terrain);

    // Water | Lava Checking
    void waterLavaCheck(const Terrain &terrain);

    PlayerState getPlayerState();

    // Inventory
    PlayerInventory& getPlayerInventory();

    // Helper function to get corner points
    std::vector<glm::vec3> getPlayerCorners(bool includeBottom, bool includeTop, bool includeMiddle);

    glm::vec3 getPos();

    // Function for removing and destroying blocks
    bool blockWithinRange(float *out_dist, glm::ivec3 *out_blockHit, float *axis);
    void destroyBlock();
    void createBlock();

    // Player overrides all of Entity's movement
    // functions so that it transforms its camera
    // by the same amount as it transforms itself.
    void moveAlongVector(glm::vec3 dir) override;
    void moveForwardLocal(float amount) override;
    void moveRightLocal(float amount) override;
    void moveUpLocal(float amount) override;
    void moveForwardGlobal(float amount) override;
    void moveRightGlobal(float amount) override;
    void moveUpGlobal(float amount) override;
    void rotateOnForwardLocal(float degrees) override;
    void rotateOnRightLocal(float degrees) override;
    void rotateOnUpLocal(float degrees) override;
    void rotateOnForwardGlobal(float degrees) override;
    void rotateOnRightGlobal(float degrees) override;
    void rotateOnUpGlobal(float degrees) override;

    // For sending the Player's data to the GUI
    // for display
    QString posAsQString() const;
    QString velAsQString() const;
    QString accAsQString() const;
    QString lookAsQString() const;
    QString stateAsQString() const;

    QString inventoryAsQString() const;
};
