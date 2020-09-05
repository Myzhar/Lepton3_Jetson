#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <sl/Camera.hpp>

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
    bool openZed();

private:
    Ui::MainWindow *ui;

    sl::Camera mZed;
    uint16_t mZedW=0;
    uint16_t mZedH=0;
};
#endif // MAINWINDOW_H
