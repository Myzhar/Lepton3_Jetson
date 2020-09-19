#include "leptongrabber.h"
#include <QDebug>
#include <iostream>

LeptonGrabber::LeptonGrabber(QObject *parent) : QObject(parent)
{

}

LeptonGrabber::~LeptonGrabber()
{
    mStopRequest = true;

    if( mGrabThread.joinable() )
    {
        mGrabThread.join();
    }
}

bool LeptonGrabber::openLepton()
{
    emit statusMessage(tr("Opening FLIR Lepton"));

    Lepton3::DebugLvl deb_lvl = Lepton3::DBG_NONE;

    mLepton.reset( new Lepton3( "/dev/spidev0.0", "/dev/i2c-0", deb_lvl ) );
    mLepton->start();

    if( !set_rgb_mode(false) )
    {
        emit statusMessage(tr("Error disabling FLIR RGB mode"));
        return false;
    }

    // ----> High gain mode [0,150] Celsius
    if( mLepton->setGainMode( LEP_SYS_GAIN_MODE_HIGH ) == LEP_OK )
    {
        LEP_SYS_GAIN_MODE_E gainMode;
        if( mLepton->getGainMode( gainMode ) == LEP_OK )
        {
            std::string str =
                    (gainMode==LEP_SYS_GAIN_MODE_HIGH)?std::string("High"):
                                                       ((gainMode==LEP_SYS_GAIN_MODE_LOW)?std::string("Low"):std::string("Auto"));
            std::cout << " * Gain mode: " << str << std::endl;
        }
    }
    else
    {
        emit statusMessage(tr("Error setting FLIR gain mode"));
        return false;
    }
    // <---- High gain mode [0,150] Celsius

    // ----> Enable Radiometry to get values indipendent from camera temperature
    if( mLepton->enableRadiometry( true ) != LEP_OK)
    {
        emit statusMessage(tr("Error enabling FLIR radiometry"));
        return false;
    }
    // <---- Enable Radiometry to get values indipendent from camera temperature

    return true;
}

void LeptonGrabber::startCapture()
{
    emit statusMessage(tr("Starting FLIR capture"));
    mGrabThread = std::thread( &LeptonGrabber::grabFunc, this );
}

cv::Mat& LeptonGrabber::getLastImageGray16()
{
    return mLeptonGray16;
}

cv::Mat& LeptonGrabber::getLastImageRGB()
{
    return mLeptonRGB;
}

void LeptonGrabber::grabFunc()
{
    qDebug() << tr("Lepton Grabber thread started");

    mStopRequest = false;

    cv::Mat frame16; // 16bit Lepton frame RAW frame
    uint16_t min_raw16;
    uint16_t max_raw16;
    bool initialized = false;
    uint8_t w,h;

    uint16_t frameIdx=0;

    emit statusMessage( "Performing Lepton FFC" );

    if( mLepton->doFFC() == LEP_OK )
    {
        emit statusMessage( "Lepton FFC completed" );
    }

    forever
    {
        if( mStopRequest )
        {
            break;
        }

        // Retrieve last available frame
        const uint16_t* data16 = mLepton->getLastFrame16( w, h, &min_raw16, &max_raw16 );

        if( data16 )
        {
            // ----> Initialize OpenCV data
            if(!initialized || h!=frame16.rows || w!=frame16.cols)
            {
                frame16 = cv::Mat( h, w, CV_16UC1 );
                initialized = true;
                mLeptonW=w;
                mLeptonH=h;
            }
            // <---- Initialize OpenCV data

            if(frameIdx==30)
            {
                emit statusMessage( "Lepton running" );
            }

            if(++frameIdx == 1000)
            {
                emit statusMessage( "Performing Lepton FFC" );

                if( mLepton->doFFC() == LEP_OK )
                {
                    emit statusMessage( "Lepton FFC completed" );
                }
                frameIdx=0;
            }

            {
                const std::lock_guard<std::mutex> lock(mImgMutex);
                memcpy( frame16.data, data16, w*h*sizeof(uint16_t) );

                mLeptonGray16 = frame16.clone();

                // ----> Normalization for displaying
                cv::Mat thermFrame;                
                normalizeFrame( frame16, thermFrame, 27.5, 40, mTempScaleFactor );
                // Conversion to RGB
                cv::cvtColor( thermFrame, mLeptonRGB, cv::COLOR_GRAY2BGR );
                // <---- Normalization for displaying

                //cv::imshow("Test", mLeptonRGB );
                //cv::waitKey(5);

                emit leptonImageReady();
            }

        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    qDebug() << tr("Lepton Grabber thread finished");
}

void LeptonGrabber::normalizeFrame(cv::Mat& in_frame16, cv::Mat& out_frame8, double min_temp, double max_temp , double rescale_fact)
{
    // ----> Rescaling/Normalization to 8bit
    uint16_t min_scale = (uint16_t)(min_temp/rescale_fact);
    uint16_t max_scale = (uint16_t)(max_temp/rescale_fact);

    cv::Mat rescaled;
    in_frame16.copyTo(rescaled);

    double diff = static_cast<double>(max_scale - min_scale); // Image range
    double scale = 255./diff; // Scale factor

    rescaled -= min_scale; // Bias
    rescaled *= scale; // Rescale data

    rescaled.convertTo( out_frame8, CV_8U );
    // <---- Rescaling/Normalization to 8bit
}


bool LeptonGrabber::set_rgb_mode(bool enable)
{
    bool rgb_mode = enable;

    if( mLepton->enableRadiometry( !rgb_mode ) < 0)
    {
        std::cerr << "Failed to set radiometry status" << std::endl;
        return false;
    }
    else
    {
        if(!rgb_mode)
        {
            std::cout << " * Radiometry enabled " << std::endl;
        }
        else
        {
            std::cout << " * Radiometry disabled " << std::endl;
        }
    }

    // NOTE: if radiometry is enabled is unuseful to keep AGC enabled
    //       (see "FLIR LEPTON 3® Long Wave Infrared (LWIR) Datasheet" for more info)

    if( mLepton->enableAgc( rgb_mode ) < 0)
    {
        std::cerr << "Failed to set radiometry status" << std::endl;
        return false;
    }
    else
    {
        if(!rgb_mode)
        {
            std::cout << " * AGC disabled " << std::endl;
        }
        else
        {
            std::cout << " * AGC enabled " << std::endl;
        }
    }

    if( mLepton->enableRgbOutput( rgb_mode ) < 0 )
    {
        std::cerr << "Failed to enable RGB output" << std::endl;
        return false;
    }
    else
    {
        if(rgb_mode)
        {
            std::cout << " * RGB enabled " << std::endl;
        }
        else
        {
            std::cout << " * RGB disabled " << std::endl;
        }
    }

    return true;
}
