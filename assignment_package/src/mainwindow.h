#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "cameracontrolshelp.h"
#include "playerinfo.h"
#include "inventory.h"


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void on_actionQuit_triggered();

    void on_actionCamera_Controls_triggered();

    void slot_showInventory();

private:
    Ui::MainWindow *ui;
    CameraControlsHelp cHelp;
    bool inventory_flag;
    PlayerInfo playerInfoWindow;
    Inventory inventoryWindow;
};


#endif // MAINWINDOW_H
