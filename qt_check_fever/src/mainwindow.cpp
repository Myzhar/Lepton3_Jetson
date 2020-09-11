#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    restoreSettings();

    ui->actionOverlay_Calibration->setChecked(!ui->dockWidget->isHidden());
    ui->horizontalSlider_h_offset->setValue(mOvHorOffset);
    ui->horizontalSlider_v_offset->setValue(mOvVerOffset);
    ui->horizontalSlider_scale_factor->setValue(mOvScale*100);
    ui->openGLWidget_img->setOverlayOffsetScale(mOvHorOffset,mOvVerOffset,mOvScale);

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
    connect( ui->openGLWidget_img, &OglRenderer::nearestPerson, this, &MainWindow::onPersonDist );

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
    ui->openGLWidget_img->updateZedObjects(obj);

    //qDebug() << tr("New Object List");
}

void MainWindow::onPersonDist(qreal dist,qreal temp)
{
    if(dist<100.0 && temp!=-273.15)
    {
        ui->lineEdit_nearest_person->setText( tr("%1 m").arg(dist,3,'f',1));
        ui->lineEdit_nearest_person_temperature->setText( tr("%1°C").arg(temp,3,'f',1));
    }
}

void MainWindow::on_horizontalSlider_h_offset_valueChanged(int value)
{
    mOvHorOffset = value;
    ui->openGLWidget_img->setOverlayOffsetScale(mOvHorOffset,mOvVerOffset,mOvScale);
}

void MainWindow::on_horizontalSlider_v_offset_valueChanged(int value)
{
    mOvVerOffset = value;
    ui->openGLWidget_img->setOverlayOffsetScale(mOvHorOffset,mOvVerOffset,mOvScale);
}

void MainWindow::on_horizontalSlider_scale_factor_valueChanged(int value)
{
    mOvScale = static_cast<qreal>(value)/100.;
    ui->openGLWidget_img->setOverlayOffsetScale(mOvHorOffset,mOvVerOffset,mOvScale);

}

void MainWindow::saveSettings()
{
    QSettings settings("Myzhar","JetsonFeverControl");
    settings.setValue( "overlay_hor_offset", mOvHorOffset );
    settings.setValue( "overlay_ver_offset", mOvVerOffset );
    settings.setValue( "overlay_scale", mOvScale );
    settings.setValue( "geometry", saveGeometry() );
    settings.setValue( "windowState", saveState() );
    settings.sync();
}

void MainWindow::restoreSettings()
{
    QSettings settings("Myzhar","JetsonFeverControl");
    bool ok;
    mOvHorOffset = settings.value( "overlay_hor_offset" ).toInt(&ok);
    if(!ok)
    {
        mOvHorOffset = 0;
    }
    mOvVerOffset = settings.value( "overlay_ver_offset" ).toInt(&ok);
    if(!ok)
    {
        mOvVerOffset = 0;
    }
    mOvScale = settings.value( "overlay_scale" ).toDouble(&ok);
    if(!ok)
    {
        mOvScale = 1.0;
    }

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::on_pushButton_reset_overlay_clicked()
{
    mOvHorOffset = 0;
    mOvVerOffset = 0;
    mOvScale = 1.0;
    ui->openGLWidget_img->setOverlayOffsetScale(mOvHorOffset,mOvVerOffset,mOvScale);
    ui->horizontalSlider_h_offset->setValue(mOvHorOffset);
    ui->horizontalSlider_v_offset->setValue(mOvVerOffset);
    ui->horizontalSlider_scale_factor->setValue(mOvScale*100);
}
