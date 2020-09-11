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

void OglRenderer::setOverlayOffsetScale(qint8 hOffset, qint8 vOffset, qreal scale )
{
    mOvHorOffset = hOffset;
    mOvVerOffset = vOffset;
    mOvScale = scale;
}

void OglRenderer::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int xOffset = (this->width() - mRenderImage.width())/2;
    int yOffset = (this->height() - mRenderImage.height())/2;

    painter.drawImage(QRect(xOffset,yOffset,mRenderImage.width(),mRenderImage.height()), mRenderImage);

    qreal min_dist=100.0;
    qreal temperature=-273.15;

    if(mZedObjects.object_list.size() > 0)
    {
        int penSize = 3;
        foreach (sl::ObjectData obj, mZedObjects.object_list)
        {
            painter.setBrush( Qt::NoBrush);
            QPen pen;
            quint8 r = (obj.id+60)*45235235;
            quint8 g = (obj.id+30)*34653524;
            quint8 b = (obj.id+90)*23451366;
            pen.setColor( QColor(r,g,b,175) );
            pen.setWidth(penSize);
            pen.setCapStyle(Qt::RoundCap);
            painter.setPen(pen);

            int rect_left = xOffset+obj.head_bounding_box_2d[0][0];
            int rect_top = yOffset+obj.head_bounding_box_2d[0][1];
            int rect_right = xOffset+obj.head_bounding_box_2d[2][0];
            int rect_bottom = yOffset+obj.head_bounding_box_2d[2][1];

            QRect rect;
            rect.setTopLeft( QPoint(rect_left,rect_top));
            rect.setBottomRight( QPoint(rect_right,rect_bottom));
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
            // <---- Nose

            // -----> Text info
            QRect text_rect;
            text_rect.setLeft(rect_left);
            text_rect.setRight(rect_right);
            text_rect.setBottom(rect_top-5);
            text_rect.setTop(rect_top-20);

            // TODO Calculate mean temperature inside the Bounding Box and print on screen emitting a signal to
            // update the GUI
            painter.drawText(text_rect, "Temp" );

            text_rect.setBottom(rect_bottom+25);
            text_rect.setTop(rect_bottom+5);
            QString nose_dist_str;
            float nose_dist=500.0f;
            float nose_X = obj.keypoint[static_cast<int>(sl::BODY_PARTS::NOSE)][0];
            float nose_Y = obj.keypoint[static_cast<int>(sl::BODY_PARTS::NOSE)][1];
            float nose_Z = obj.keypoint[static_cast<int>(sl::BODY_PARTS::NOSE)][2];
            if( std::isfinite(nose_X) && std::isfinite(nose_Y) && std::isfinite(nose_Z) )
            {
                nose_dist = sqrt(nose_X*nose_X+nose_Y*nose_Y+nose_Z*nose_Z);
                nose_dist_str = tr("%1 m").arg(nose_dist,3,'f',1);
                painter.drawText(text_rect,nose_dist_str);
            }
            // <----- Text info

            if(nose_dist<min_dist)
            {
                min_dist = nose_dist;
                temperature = 36.5;
            }
        }
    }

    emit nearestPerson(min_dist,temperature);
}
