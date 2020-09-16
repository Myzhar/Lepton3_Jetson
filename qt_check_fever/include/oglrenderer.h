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
    void updateFlirImage(cv::Mat &image);

    void setOverlayOffsetScale( qint8 hOffset, qint8 vOffset, qreal scale );

signals:
    void nearestPerson(qreal dist, qreal temp);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    QImage mRenderImage;
    QImage mFlirImg;
    QImage mAlphaCh;
    bool mAlphaChValid=false;
    const quint8 mAlphaVal=150;

    bool mOnlyFlir=false;

    sl::Objects mZedObjects;

    qint8 mOvHorOffset=0;
    qint8 mOvVerOffset=0;
    qreal mOvScale=1.0;


};

#endif // OGLRENDERER_H
