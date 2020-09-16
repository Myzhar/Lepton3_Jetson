#ifndef LEPTONGRABBER_H
#define LEPTONGRABBER_H

#include <QObject>
#include <thread>
#include <mutex>

#include "Lepton3.hpp"

#include <opencv2/opencv.hpp>

class LeptonGrabber : public QObject
{
    Q_OBJECT
public:
    explicit LeptonGrabber(QObject *parent = nullptr);
    virtual ~LeptonGrabber();

    bool openLepton();
    void startCapture();

    cv::Mat& getLastImageGray16();
    cv::Mat& getLastImageRGB();

protected:
    void grabFunc();
    bool set_rgb_mode(bool enable);
    void normalizeFrame( cv::Mat& in_frame16, cv::Mat& out_frame8, double min_temp, double max_temp , double rescale_fact);

signals:
    void leptonImageReady();
    void statusMessage(QString msg);

private:
    std::shared_ptr<Lepton3> mLepton;
    uint16_t mLeptonW=0;
    uint16_t mLeptonH=0;

    cv::Mat mLeptonGray16;
    cv::Mat mLeptonRGB;
    std::thread mGrabThread;
    std::mutex mImgMutex;

    bool mStopRequest = false;
};

#endif // LEPTONGRABBER_H
