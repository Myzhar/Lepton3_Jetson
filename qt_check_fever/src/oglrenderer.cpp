#include "oglrenderer.h"

#include <QPainter>
#include <QSurfaceFormat>

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
    mZedObjects = objects;
}

void OglRenderer::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int xOffset = (this->width() - mRenderImage.width())/2;
    int yOffset = (this->height() - mRenderImage.height())/2;

    painter.drawImage(QRect(xOffset,yOffset,mRenderImage.width(),mRenderImage.height()), mRenderImage);

    if(mZedObjects.object_list.size() > 0)
    {
        int penSize = 3;
        foreach (sl::ObjectData obj, mZedObjects.object_list)
        {
            painter.setBrush( Qt::NoBrush);
            QPen pen;
            quint8 r = (obj.id+90)*45235235;
            quint8 g = (obj.id+30)*34653524;
            quint8 b = (obj.id+60)*23451366;
            pen.setColor( QColor(r,g,b,150) );
            pen.setWidth(penSize);
            pen.setCapStyle(Qt::RoundCap);
            painter.setPen(pen);
            QRect rect;
            rect.setTopLeft( QPoint(xOffset+obj.head_bounding_box_2d[0][0],yOffset+obj.head_bounding_box_2d[0][1]));
            rect.setBottomRight( QPoint(xOffset+obj.head_bounding_box_2d[2][0],yOffset+obj.head_bounding_box_2d[2][1]));
            painter.drawRoundedRect(rect,3*penSize,3*penSize);

            pen.setWidth(2*penSize);
            painter.setPen(pen);
            // ----> Ears and Eyes
            for( int i=static_cast<int>(sl::BODY_PARTS::RIGHT_EYE); i<=static_cast<int>(sl::BODY_PARTS::LEFT_EAR); i++)
            {
                float x = obj.keypoint_2d[i][0];
                float y = obj.keypoint_2d[i][1];
                if( x>0 && y>0 )
                {
                    painter.drawPoint(xOffset+x,yOffset+y);
                }
            }
            // <---- Ears and Eyes

            // ----> Nose
            float x = obj.keypoint_2d[static_cast<int>(sl::BODY_PARTS::NOSE)][0];
            float y = obj.keypoint_2d[static_cast<int>(sl::BODY_PARTS::NOSE)][1];
            if( x>0 && y>0 )
            {
                painter.drawPoint(xOffset+x,yOffset+y);
            }
            // <---- Node
        }
    }


}
