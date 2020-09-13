#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QSettings>
#include <QLabel>

#include "grabber.h"



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
    void onPersonDist(qreal dist,qreal temp);
    void onStatusMessage(QString message);

private slots:
    void on_horizontalSlider_h_offset_valueChanged(int value);
    void on_horizontalSlider_v_offset_valueChanged(int value);
    void on_horizontalSlider_scale_factor_valueChanged(int value);

    void on_pushButton_reset_overlay_clicked();

private:
    Ui::MainWindow *ui;

    Grabber mGrabber;

    qint8 mOvHorOffset=0;
    qint8 mOvVerOffset=0;
    qreal mOvScale=1.0;

    QLabel mStatusLabel;

};
#endif // MAINWINDOW_H
