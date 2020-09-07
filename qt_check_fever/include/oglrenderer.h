#ifndef OGLRENDERER_H
#define OGLRENDERER_H

#include <QOpenGLWidget>
#include <QImage>

#include <sl/Camera.hpp>

class OglRenderer : public QOpenGLWidget
{
public:
    OglRenderer(QWidget* parent = nullptr);

    void updateZedImage(sl::Mat &image);
    void updateZedObjects(sl::Objects &objects);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    QImage mRenderImage;

    sl::Objects mZedObjects;
};

#endif // OGLRENDERER_H
