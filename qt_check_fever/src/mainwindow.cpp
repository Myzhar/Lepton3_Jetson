#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QMessageBox>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    bool zed_ok=false;

    do
    {
        zed_ok = openZed();

        if(!zed_ok)
        {
            QMessageBox::critical(this, tr("Error"), tr("Please connect a ZED2 Camera") );

            QThread::msleep(1000);
        }
    } while(!zed_ok);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::openZed()
{
    if(mZed.isOpened())
    {
        mZed.close();
    }

    sl::InitParameters initParams;
    initParams.camera_fps = 15;
    initParams.camera_resolution = sl::RESOLUTION::VGA;

    sl::ERROR_CODE res = mZed.open( initParams );

    if( sl::ERROR_CODE::SUCCESS != res)
    {
        return false;
    }

    if( mZed.getCameraInformation().camera_model != sl::MODEL::ZED2)
    {
        return false;
    }

    mZedW = mZed.getCameraInformation().camera_configuration.resolution.width;
    mZedH = mZed.getCameraInformation().camera_configuration.resolution.height;

    return true;
}
