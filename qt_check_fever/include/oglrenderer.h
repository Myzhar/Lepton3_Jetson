#ifndef OGLRENDERER_H
#define OGLRENDERER_H

#include <QOpenGLWidget>
#include <QImage>

#include <sl/Camera.hpp>
#include <opencv2/opencv.hpp>

class OglRenderer : public QOpenGLWidget
{
    Q_OBJECT

public:
    OglRenderer(QWidget* parent = nullptr);

    void setOnlyFlir(bool onlyFlir=true){mOnlyFlir=onlyFlir;}

    void updateZedImage(sl::Mat &image);
    void updateZedObjects(sl::Objects &objects);
    void updateFlirImages(cv::Mat &image, cv::Mat& raw16);

    void setOverlayOffsetScale( qint8 hOffset, qint8 vOffset, qreal scale );

signals:
    void nearestPerson(qreal dist, qreal temp);

public slots:
    void onSetCalibrationMode(bool calibActive){mCalibrating=calibActive;}

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    QImage mRenderImage;
    QImage mFlirImg;
    QImage mAlphaCh;
    bool mAlphaChValid=false;

    bool mOnlyFlir=false;

    sl::Objects mZedObjects;

    qint8 mOvHorOffset=0;
    qint8 mOvVerOffset=0;
    qreal mOvScale=1.0;

    bool mCalibrating=false;

    cv::Mat mOcvRaw16;
    cv::Mat mOcvRGB;

    const quint8 mAlphaVal=175;


    const double temp_min_human = 34.5f;
    const double temp_warn = 37.0f;
    const double temp_fever = 37.5f;
    const double temp_max_human = 42.0f;

    const cv::Scalar COL_NORM_TEMP=cv::Scalar(15,200,15);
    const cv::Scalar COL_WARN_TEMP=cv::Scalar(10,170,200);
    const cv::Scalar COL_FEVER_TEMP=cv::Scalar(40,40,220);

    // Hypothesis: sensor is linear in 14bit dynamic
    // If the range of the sensor is [0,150] °C in High Gain mode
   const double temp_scale_factor = 0.0092; // 150/(2^14-1))
};

#endif // OGLRENDERER_H
