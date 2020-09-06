#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    bool zed_ok=false;

    do
    {
        zed_ok = mGrabber.openZed();

        if(!zed_ok)
        {
            if( QMessageBox::StandardButton::Abort == QMessageBox::critical(this, tr("Error"), tr("Please connect a ZED2 Camera"),
                                  QMessageBox::StandardButton::Abort|QMessageBox::StandardButton::Ok) )
            {
                exit(EXIT_FAILURE);
            }

            QThread::msleep(1000);
        }
    } while(!zed_ok);

    connect( &mGrabber, &Grabber::zedImageReady, this, &MainWindow::onNewZedImage );
    connect( &mGrabber, &Grabber::zedObjListReady, this, &MainWindow::onNewZedObjList );

    mGrabber.startCapture();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewZedImage()
{
    sl::Mat frame = mGrabber.getLastImage();

    ui->openGLWidget_img->updateZedImage(frame);

    //qDebug() << tr("New Image");
}

void MainWindow::onNewZedObjList()
{
    sl::Objects obj = mGrabber.getLastObjDet();

    //qDebug() << tr("New Object List");
}
