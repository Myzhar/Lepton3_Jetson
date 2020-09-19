#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QSettings>
#include <QLabel>

#include "zedgrabber.h"
#include "leptongrabber.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void saveSettings();
    void restoreSettings();
    virtual void closeEvent(QCloseEvent *event);

protected slots:
    void onNewZedImage();
    void onNewZedObjList();
    void onNewLeptonImage();
    void onPersonDist(qreal dist,qreal temp);
    void onZedStatusMessage(QString message);
    void onLeptonStatusMessage(QString message);

private slots:
    void on_horizontalSlider_h_offset_valueChanged(int value);
    void on_horizontalSlider_v_offset_valueChanged(int value);
    void on_horizontalSlider_scale_factor_valueChanged(int value);

    void on_pushButton_reset_overlay_clicked();

    void on_checkBox_enable_calibration_clicked(bool checked);

    void on_horizontalSlider_simul_fever_valueChanged(int value);

    void on_checkBox_skeleton_clicked(bool checked);

private:
    Ui::MainWindow *ui;

    ZedGrabber mZedGrabber;
    LeptonGrabber mLeptonGrabber;

    qint8 mOvHorOffset=0;
    qint8 mOvVerOffset=0;
    qreal mOvScale=1.0;

    qreal mSimulFever=0.0;

    bool mShowSkeleton=false;

    QLabel mZedStatusLabel;
    QLabel mLeptonStatusLabel;

};
#endif // MAINWINDOW_H
