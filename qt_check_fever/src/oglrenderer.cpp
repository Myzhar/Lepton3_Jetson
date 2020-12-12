#include "oglrenderer.h"

#include <QPainter>
#include <QSurfaceFormat>

#include <QDebug>
#include <QFont>

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

void OglRenderer::updateFlirImages(cv::Mat& rgb, cv::Mat &raw16)
{
    mOcvRGB = rgb.clone();
    mFlirImg = QImage(mOcvRGB.data, rgb.cols, rgb.rows,
                      QImage::Format_RGB888);

    mOcvRaw16 = raw16.clone();

    if(mOnlyFlir)
    {
        this->update();
    }
}

void OglRenderer::updateZedObjects(sl::Objects &objects)
{
    mZedObjects = objects;
}

void OglRenderer::setOverlayOffsetScale(qint8 hOffset, qint8 vOffset, qreal scale )
{
    mOvHorOffset = hOffset;
    mOvVerOffset = vOffset;
    if(mOvScale!=scale)
    {
        mAlphaChValid = false;
    }
    mOvScale = scale;
}

void OglRenderer::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QFont font = painter.font();
    font.setPointSize(16);
    painter.setFont(font);

    if(mOnlyFlir)
    {
        int xOffset = (this->width() - mFlirImg.width())/2;
        int yOffset = (this->height() - mFlirImg.height())/2;

        painter.drawImage(QRect(xOffset,yOffset,mFlirImg.width(),mFlirImg.height()), mFlirImg);
        return;
    }

    int xOffset = (this->width() - mRenderImage.width())/2;
    int yOffset = (this->height() - mRenderImage.height())/2;

    painter.drawImage(QRect(xOffset,yOffset,mRenderImage.width(),mRenderImage.height()), mRenderImage);

    QImage flirScaled;
    int xOffset_flir=0;
    int yOffset_flir=0;
    if(!mFlirImg.isNull())
    {
        flirScaled = mFlirImg.scaledToHeight( mOvScale*mRenderImage.height() );
        flirScaled = flirScaled.convertToFormat(QImage::Format_ARGB32);

        xOffset_flir = mOvHorOffset + xOffset + (mRenderImage.width() - flirScaled.width())/2;
        yOffset_flir = mOvVerOffset + yOffset + (mRenderImage.height() - flirScaled.height())/2;

        if(mAlphaCh.isNull() || !mAlphaChValid)
        {
            mAlphaCh = QImage(flirScaled.size(), QImage::Format_Alpha8 );
            mAlphaChValid=true;
        }

        if(!mCalibrating) {
            mAlphaCh.fill(QColor(0,0,0,0));
        }
        else
        {
            mAlphaCh.fill(QColor(ALPHA_VAL,ALPHA_VAL,ALPHA_VAL,ALPHA_VAL));
        }
    }

    qreal min_dist=100.0;
    qreal temperature=-273.15;

    if(mZedObjects.object_list.size() > 0)
    {
        int penSize = 5;
        foreach (sl::ObjectData obj, mZedObjects.object_list)
        {
            painter.setBrush( Qt::NoBrush);
            QPen pen;
            quint8 r = (obj.id+1)*452335;
            quint8 g = (obj.id+1)*4653523;
            quint8 b = (obj.id+1)*2351366;
            r = r>100?r:100;
            g = g>100?g:100;
            b = b>100?b:100;
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

            int valid_count=0;
            double temp_sum=0.0;

            //if(!mCalibrating)
            {
                uint16_t* data16 = (uint16_t*)(&mOcvRaw16.data[0]);

                if(!mAlphaCh.isNull() && mAlphaChValid)
                {
                    QPainter p(&mAlphaCh);
                    p.setPen(Qt::NoPen);
                    p.setBrush(QBrush(QColor(ALPHA_VAL,ALPHA_VAL,ALPHA_VAL,ALPHA_VAL)));
                    rect.moveCenter(QPoint(rect.center().x()-xOffset_flir,rect.center().y()-yOffset_flir));

                    if(!mCalibrating)
                    {
                        p.drawRoundedRect(rect,3*penSize,3*penSize);
                    }

                    double ratio = static_cast<double>(mOcvRaw16.rows)/flirScaled.height();

                    int init_U = rect.left();
                    init_U = qMax(0,init_U);
                    int init_V = rect.top();
                    init_V = qMax(0,init_V);
                    int fin_U = rect.right();
                    fin_U = qMin(fin_U,mAlphaCh.width());
                    int fin_V = rect.bottom();
                    fin_V = qMin(fin_V,mAlphaCh.height());

                    if(mCalibrating)
                    {
                        cv::rectangle(mOcvRGB, cv::Rect(init_U*ratio,init_V*ratio,(fin_U-init_U)*ratio,(fin_V-init_V)*ratio), cv::Scalar::all(255) );
                        //cv::imshow("Test",mOcvRGB);
                    }

                    for( int V=init_V; V<fin_V; V++ )
                    {
                        int v = qRound(static_cast<double>(V)*ratio+0.5);

                        for( int U=init_U; U<fin_U; U++ )
                        {
                            int u = qRound(static_cast<double>(U)*ratio+0.5);

                            int i = u+v*mOcvRaw16.cols;
                            double temp = (data16[i]*temp_scale_factor+0.05);

                            if( temp >= TEMP_MIN_HUMAN && temp <= TEMP_MAX_HUMAN )
                            {
                                temp += mSimulFever;

                                valid_count++;
                                temp_sum += temp;

                                QColor color;

                                if( temp >= TEMP_MIN_HUMAN && temp < TEMP_WARN )
                                {
                                    color = COL_NORM_TEMP;
                                }
                                else if( temp >= TEMP_WARN && temp < TEMP_FEVER )
                                {
                                    color = COL_WARN_TEMP;
                                }
                                else if( temp >= TEMP_FEVER && temp <= TEMP_MAX_HUMAN )
                                {
                                    color = COL_FEVER_TEMP;
                                }

                                flirScaled.setPixelColor(U,V,color);
                            }
                        }
                    }
                }
            }

            if(valid_count>10)
            {
                temperature = temp_sum/valid_count;
            }

            if(mShowSkeleton)
            {

                pen.setWidth(2*penSize);
                painter.setPen(pen);

                float x,y;
                // ----> Ears and Eyes
                for( int i=static_cast<int>(sl::BODY_PARTS::RIGHT_EYE); i<=static_cast<int>(sl::BODY_PARTS::LEFT_EAR); i++)
                {
                    x = obj.keypoint_2d[i].x;
                    y = obj.keypoint_2d[i].y;
                    if( x>0 && y>0 )
                    {
                        painter.drawPoint(xOffset+x,yOffset+y);
                    }
                }
                // <---- Ears and Eyes

                // ----> Nose
                x = obj.keypoint_2d[static_cast<int>(sl::BODY_PARTS::NOSE)].x;
                y = obj.keypoint_2d[static_cast<int>(sl::BODY_PARTS::NOSE)].y;
                if( x>0 && y>0 )
                {
                    painter.drawPoint(xOffset+x,yOffset+y);
                }
                // <---- Nose

                /*/ ----> Neck
            x = obj.keypoint_2d[static_cast<int>(sl::BODY_PARTS::NECK)].x;
            y = obj.keypoint_2d[static_cast<int>(sl::BODY_PARTS::NECK)].y;
            if( x>0 && y>0 )
            {
                painter.drawPoint(xOffset+x,yOffset+y);
            }
            // <---- Neck */

                // ----> Skeleton
                pen.setWidth(3);
                painter.setPen(pen);
                //Display sekeleton if available
                auto keypoints = obj.keypoint_2d;
                if (keypoints.size()>0)
                {
                    for (auto &limb : sl::BODY_BONES)
                    {
                        sl::float2 kp_1 = keypoints[(int)limb.first];
                        sl::float2 kp_2 = keypoints[(int)limb.second];
                        if (kp_1.x>0 && kp_2.x>0)
                        {
                            painter.drawLine( QPointF(xOffset+kp_1.x, yOffset+kp_1.y),
                                              QPointF(xOffset+kp_2.x, yOffset+kp_2.y));
                        }
                    }
                }
                // <---- Neck
            }

            // -----> Text info
            QRect text_rect;
            text_rect.setLeft(rect_left);
            text_rect.setRight(rect_right);
            text_rect.setBottom(rect_top-5);
            text_rect.setTop(rect_top-27);

            QString temp_st = "--.- °C";
            if( valid_count>10 ){
                temp_st = tr("%1 °C").arg(temp_sum/valid_count, 3, 'f', 1);
            }
            painter.drawText(text_rect, temp_st );

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
                if(valid_count>10)
                {
                    temperature = temp_sum/valid_count;
                }
            }
        }
    }

    if(!mFlirImg.isNull())
    {
        if(!mAlphaCh.isNull() && mAlphaChValid)
        {
            flirScaled.setAlphaChannel(mAlphaCh);
        }

        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawImage(QRect(xOffset_flir,yOffset_flir,flirScaled.width(),flirScaled.height()), flirScaled);
    }

    emit nearestPerson(min_dist,temperature);
}
