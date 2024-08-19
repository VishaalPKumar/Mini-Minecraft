#include "inventory.h"
#include "ui_inventory.h"
#include "utils/utils.h"

#include <QKeyEvent>
#include <iostream>

Inventory::Inventory(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnTopHint),
    ui(new Ui::Inventory)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowOpacity(0.8);

    views = {
        {BlockType::GRASS, ui->grassView},
        {BlockType::DIRT, ui->dirtView},
        {BlockType::STONE, ui->stoneView},
        {BlockType::WATER, ui->waterView},
        {BlockType::SNOW, ui->snowView},
        {BlockType::SAND, ui->sandView},
        {BlockType::LAVA, ui->lavaView}
    };

    labels = {
        {BlockType::GRASS, ui->grassLabel},
        {BlockType::DIRT, ui->dirtLabel},
        {BlockType::STONE, ui->stoneLabel},
        {BlockType::WATER, ui->waterLabel},
        {BlockType::SNOW, ui->snowLabel},
        {BlockType::SAND, ui->sandLabel},
        {BlockType::LAVA, ui->lavaLabel}
    };

    for (auto view: views) {
        view.second->setStyleSheet("background: transparent; border: 3px solid white;");
        view.second->setPixmap(QPixmap(getCurrentPath() + blockTypeToTexture[view.first]));
        view.second->setScaledContents(true);
    }
}


Inventory::~Inventory()
{
    delete ui;
}

void Inventory::slot_setPlayerInventory(const QString &inventory) {
    QStringList inventoryList = inventory.split(",");
    for (auto item: inventoryList) {
        QStringList itemSplit = item.split(":");
        if (itemSplit.length() == 2) {
            BlockType blockType = static_cast<BlockType>(itemSplit[0].toInt());
            int count = itemSplit[1].toInt();
            labels[blockType]->setText(QString::number(count));
        }
    }
}

void Inventory::slot_setSelectedBlock(BlockType blockType) {
    for (auto view: views) {
        view.second->setStyleSheet("background: transparent; border: 3px solid white;");
    }
    views[blockType]->setStyleSheet("background: transparent; border: 3px solid red;");
}



