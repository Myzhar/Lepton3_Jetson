#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QMessageBox>
#include <QCheckBox>
#include <QDebug>

#include "globals.h"

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
    ui->openGLWidget_thermal->setOnlyFlir();
    ui->openGLWidget_img->showSkeleton(mShowSkeleton);

    ui->checkBox_skeleton->setChecked(mShowSkeleton);

    ui->statusbar->addWidget(&mZedStatusLabel);
    ui->statusbar->addPermanentWidget(&mLeptonStatusLabel);

    ui->label_simul_fever->setText(tr("+0.0°C"));
    ui->horizontalSlider_simul_fever->setValue(0);

    connect( ui->checkBox_enable_calibration, &QCheckBox::clicked, ui->openGLWidget_img, &OglRenderer::onSetCalibrationMode );

    connect( &mZedGrabber, &ZedGrabber::statusMessage, this, &MainWindow::onZedStatusMessage );
    connect( &mZedGrabber, &ZedGrabber::zedImageReady, this, &MainWindow::onNewZedImage );
    connect( &mZedGrabber, &ZedGrabber::zedObjListReady, this, &MainWindow::onNewZedObjList );

    connect( &mLeptonGrabber, &LeptonGrabber::statusMessage, this, &MainWindow::onLeptonStatusMessage );
    connect( &mLeptonGrabber, &LeptonGrabber::leptonImageReady, this, &MainWindow::onNewLeptonImage );

    bool zed_ok=false;

    do
    {
        zed_ok = mZedGrabber.openZed();

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

    // TODO check
    if( !mLeptonGrabber.openLepton() )
    {
        QMessageBox::critical(this, tr("Error"), tr("FLIR Lepton module not responding"),
                              QMessageBox::StandardButton::Abort);

        exit(EXIT_FAILURE);
    }

    connect( ui->openGLWidget_img, &OglRenderer::nearestPerson, this, &MainWindow::onPersonDist );

    mLeptonGrabber.startCapture();
    mZedGrabber.startCapture();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onZedStatusMessage(QString message)
{
    mZedStatusLabel.setText(message);
}

void MainWindow::onLeptonStatusMessage(QString message)
{
    mLeptonStatusLabel.setText(message);
}

void MainWindow::onNewZedImage()
{
    sl::Mat frame = mZedGrabber.getLastImage();
    ui->openGLWidget_img->updateZedImage(frame);
}

void MainWindow::onNewLeptonImage()
{
    cv::Mat frame16 = mLeptonGrabber.getLastImageGray16();
    cv::Mat frameRGB = mLeptonGrabber.getLastImageRGB();

    ui->openGLWidget_img->updateFlirImages(frameRGB,frame16);
    ui->openGLWidget_thermal->updateFlirImages(frameRGB,frame16);
}

void MainWindow::onNewZedObjList()
{
    sl::Objects obj = mZedGrabber.getLastObjDet();
    ui->openGLWidget_img->updateZedObjects(obj);

    //qDebug() << tr("New Object List");
}

void MainWindow::onPersonDist(qreal dist,qreal temp)
{
    if(dist<100.0)
    {
        ui->lineEdit_nearest_person->setText( tr("%1 m").arg(dist,3,'f',1));
        if( temp!=-273.15) {
            ui->lineEdit_nearest_person_temperature->setText( tr("%1 °C").arg(temp,3,'f',1));

            if(temp < TEMP_WARN)
            {
                ui->label_alert->setText( tr("NO\nFEVER") );
                ui->label_alert->setStyleSheet("QLabel { font-family: Ubuntu; font-size: 32pt; color : green; font-weight:bold;}");
            }
            else if( temp < TEMP_FEVER)
            {
                ui->label_alert->setText( tr("FEVER\nWARNING") );
                ui->label_alert->setStyleSheet("QLabel { font-family: Ubuntu; font-size: 32pt; color : orange; font-weight:bold;}");
            }
            else
            {
                ui->label_alert->setText( tr("FEVER\nALERT") );
                ui->label_alert->setStyleSheet("QLabel { font-family: Ubuntu; font-size: 32pt; color : red; font-weight:bold;}");
            }
        }
        else
        {
            ui->lineEdit_nearest_person_temperature->setText( tr("--.- °C"));
            ui->label_alert->setText( tr("Temperature\nN/A") );
            ui->label_alert->setStyleSheet("QLabel { font-family: Ubuntu; font-size: 32pt; color : black; font-weight:bold;}");
        }
    }
    else
    {
         ui->lineEdit_nearest_person_temperature->setText( tr("--.- °C"));
         ui->lineEdit_nearest_person->setText( tr("--.- m"));
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
    settings.setValue( "show_skeleton", mShowSkeleton );
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

    mShowSkeleton = settings.value( "show_skeleton" ).toBool();

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

void MainWindow::on_checkBox_enable_calibration_clicked(bool checked)
{
    ui->horizontalSlider_h_offset->setEnabled(checked);
    ui->horizontalSlider_v_offset->setEnabled(checked);
    ui->horizontalSlider_scale_factor->setEnabled(checked);
    ui->pushButton_reset_overlay->setEnabled(checked);
}

void MainWindow::on_horizontalSlider_simul_fever_valueChanged(int value)
{
    mSimulFever = static_cast<qreal>(value)/10.;
    ui->label_simul_fever->setText(tr("+%1°C").arg(mSimulFever,3,'f',1));

    ui->openGLWidget_img->setSimulFever(mSimulFever);
}

void MainWindow::on_checkBox_skeleton_clicked(bool checked)
{
    mShowSkeleton=checked;
    ui->openGLWidget_img->showSkeleton(mShowSkeleton);
}
