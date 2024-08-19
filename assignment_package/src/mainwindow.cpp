#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), cHelp(),
    inventory_flag(false),
    playerInfoWindow(this)
{
    ui->setupUi(this);
    ui->mygl->setFocus();
    this->playerInfoWindow.show();
    playerInfoWindow.move(QGuiApplication::primaryScreen()->availableGeometry().center() - this->rect().center() + QPoint(this->width() * 0.75, 0));
    connect(ui->mygl, SIGNAL(sig_sendPlayerPos(QString)), &playerInfoWindow, SLOT(slot_setPosText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerVel(QString)), &playerInfoWindow, SLOT(slot_setVelText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerAcc(QString)), &playerInfoWindow, SLOT(slot_setAccText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerLook(QString)), &playerInfoWindow, SLOT(slot_setLookText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerChunk(QString)), &playerInfoWindow, SLOT(slot_setChunkText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerTerrainZone(QString)), &playerInfoWindow, SLOT(slot_setZoneText(QString)));

    connect(ui->mygl, SIGNAL(sig_showInventory()), this, SLOT(slot_showInventory()));
    connect(ui->mygl, SIGNAL(sig_sendPlayerState(QString)), &playerInfoWindow, SLOT(slot_setPlayerStateText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerInventoryUpdate(QString)), &inventoryWindow, SLOT(slot_setPlayerInventory(QString)));
    connect(ui->mygl, SIGNAL(sig_sendSelectedBlock(BlockType)), &inventoryWindow, SLOT(slot_setSelectedBlock(BlockType)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::moveEvent(QMoveEvent *event) {
    QMainWindow::moveEvent(event);
    if(inventory_flag) {
        QPoint newPos = this->pos() + QPoint(this->width() / 2 - inventoryWindow.width() / 2,
                                             this->height() * 0.9 - inventoryWindow.height() / 2);
        inventoryWindow.move(newPos);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    if (inventory_flag) {
        QPoint newPos = this->pos() + QPoint(this->width() / 2 - inventoryWindow.width()/2,
                                             this->height() * 0.9 - inventoryWindow.height()/2);
        inventoryWindow.move(newPos);
    }
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    cHelp.show();
}

void MainWindow::slot_showInventory() {
    inventory_flag = !inventory_flag;
    if (inventory_flag) {
        inventoryWindow.setAttribute(Qt::WA_ShowWithoutActivating); // Important: show without taking focus
        inventoryWindow.setWindowFlags(inventoryWindow.windowFlags() | Qt::WindowStaysOnTopHint);
        QPoint newPos = this->pos() + QPoint(this->width() / 2 - inventoryWindow.width()/2,
                                             this->height() * 0.9 - inventoryWindow.height()/2);
        inventoryWindow.move(newPos);
        inventoryWindow.show();
    } else {
        this->inventoryWindow.close();
    }
    this->activateWindow();
}
