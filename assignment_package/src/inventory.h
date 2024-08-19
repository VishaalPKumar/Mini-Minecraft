#pragma once

#include <QWidget>
#include "QtWidgets/qlabel.h"
#include "scene/chunk.h"


static std::unordered_map<BlockType, QString, EnumHash> blockTypeToTexture = {
    {BlockType::GRASS, "/textures/grassTexture.png"},
    {BlockType::DIRT, "/textures/dirtTexture.png"},
    {BlockType::STONE, "/textures/stoneTexture.png"},
    {BlockType::WATER, "/textures/waterTexture.png"},
    {BlockType::SNOW, "/textures/snowTexture.png"},
    {BlockType::SAND, "/textures/sandTexture.png"},
    {BlockType::LAVA, "/textures/lavaTexture.png"}
};

namespace Ui {
class Inventory;
}

class Inventory : public QWidget {
    Q_OBJECT
public:
    explicit Inventory(QWidget *parent = nullptr);
    ~Inventory();
public slots:
    void slot_setPlayerInventory(const QString &inventory);
    void slot_setSelectedBlock(BlockType blockType);

private:
    Ui::Inventory *ui;
    std::unordered_map<BlockType, QLabel*, EnumHash> views;
    std::unordered_map<BlockType, QLabel*, EnumHash> labels;
};
