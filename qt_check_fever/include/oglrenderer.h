#ifndef OGLRENDERER_H
#define OGLRENDERER_H

#include <QOpenGLWidget>
#include <QImage>

#include <sl/Camera.hpp>

class OglRenderer : public QOpenGLWidget
{
    Q_OBJECT;

public:
    OglRenderer(QWidget* parent = nullptr);

    void updateZedImage(sl::Mat &image);
    void updateZedObjects(sl::Objects &objects);

    void setOverlayOffsetScale( qint8 hOffset, qint8 vOffset, qreal scale );

signals:
    void nearestPerson(qreal dist, qreal temp);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    QImage mRenderImage;

    sl::Objects mZedObjects;

    qint8 mOvHorOffset=0;
    qint8 mOvVerOffset=0;
    qreal mOvScale=1.0;
};

#endif // OGLRENDERER_H
