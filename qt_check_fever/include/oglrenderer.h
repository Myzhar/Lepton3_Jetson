#ifndef OGLRENDERER_H
#define OGLRENDERER_H

#include <QOpenGLWidget>
#include <QImage>

#include <sl/Camera.hpp>
#include <opencv2/opencv.hpp>

#include "globals.h"

class OglRenderer : public QOpenGLWidget
{
    Q_OBJECT

public:
    OglRenderer(QWidget* parent = nullptr);

    void updateZedImage(sl::Mat &image);
    void updateZedObjects(sl::Objects &objects);
    void updateFlirImages(cv::Mat &image, cv::Mat& raw16);

    void setOverlayOffsetScale( qint8 hOffset, qint8 vOffset, qreal scale );
    inline void setSimulFever(qreal value){mSimulFever=value;}
    inline void setOnlyFlir(bool onlyFlir=true){mOnlyFlir=onlyFlir;}
    inline void showSkeleton(bool show){mShowSkeleton=show;}

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
    bool mShowSkeleton=false;

    sl::Objects mZedObjects;

    qint8 mOvHorOffset=0;
    qint8 mOvVerOffset=0;
    qreal mOvScale=1.0;

    bool mCalibrating=false;

    cv::Mat mOcvRaw16;
    cv::Mat mOcvRGB;

    qreal mSimulFever=0.0;
};

#endif // OGLRENDERER_H
