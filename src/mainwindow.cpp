#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), cHelp()
{
    ui->setupUi(this);

    ui->menuBar->setNativeMenuBar(false);
    connect(ui->menuHeightMap, SIGNAL(triggered(QAction*))
             , SLOT(onLoadHeightMap(QAction*)));
    ui->mygl->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    cHelp.show();
}


void MainWindow::onLoadHeightMap(QAction*)
{
    ui->mygl->heightMap();
}
