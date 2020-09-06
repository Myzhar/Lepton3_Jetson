#include "oglrenderer.h"

#include <QPainter>

OglRenderer::OglRenderer(QWidget *parent)
 : QOpenGLWidget(parent)
{
}

void OglRenderer::updateZedImage(sl::Mat& image)
{
    sl::uchar4* data = image.getPtr<sl::uchar4>();
    mRenderImage = QImage(data->ptr(), image.getWidth(), image.getHeight(),
                              QImage::Format_ARGB32);

    this->update();
}

void OglRenderer::updateZedObjects(sl::Objects &objects)
{
    mNewObjects = true;
}

void OglRenderer::paintEvent(QPaintEvent*)
{
  QPainter painter(this);

  int xOffset = (this->width() - mRenderImage.width())/2;
  int yOffset = (this->height() - mRenderImage.height())/2;

  painter.drawImage(QRect(xOffset,yOffset,mRenderImage.width(),mRenderImage.height()), mRenderImage);
  if(mNewObjects)
  {
      mNewObjects = false;

      //painter.drawRects( mRects );
  }


}
