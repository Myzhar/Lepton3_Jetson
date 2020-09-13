#ifndef GRABBER_H
#define GRABBER_H

#include <QObject>
#include <thread>
#include <mutex>

#include <sl/Camera.hpp>

class Grabber : public QObject
{
    Q_OBJECT
public:
    explicit Grabber(QObject *parent = nullptr);
    virtual ~Grabber();

    bool openZed();
    void startCapture();

    sl::Mat& getLastImage();
    sl::Objects& getLastObjDet();

protected:
    void grabFunc();

signals:
    void zedImageReady();
    void zedObjListReady();
    void statusMessage(QString msg);

private:
    sl::Camera mZed;
    sl::RESOLUTION mZedResol = sl::RESOLUTION::VGA;
    int mZedFramerate = 30;
    uint16_t mZedW=0;
    uint16_t mZedH=0;

    sl::Mat mZedRgb;
    sl::Mat mZedDepth;
    sl::Objects mZedObj;

    sl::ObjectDetectionRuntimeParameters mObjRtParams;

    std::thread mGrabThread;
    std::mutex mImgMutex;
    std::mutex mObjMutex;

    bool mStopRequest = false;
};

#endif // GRABBER_H
