#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <string>
#include <chrono>
#include <thread>

#include "Lepton3.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "stopwatch.hpp"

using namespace std;

// ----> Global variables
Lepton3* lepton3=nullptr;
static bool close = false;
static bool rgb_mode = true;
// <---- Global variables

// ----> Global functions
void close_handler(int s);
void keyboard_handler(int key);

void set_rgb_mode(bool enable);
// <---- Global functions



int main (int argc, char *argv[])
{
    cout << "OpenCV demo for Lepton3 on Nvidia Jetson" << endl;

    // ----> Set Ctrl+C handler
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = close_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    // <---- Set Ctrl+C handler

    Lepton3::DebugLvl deb_lvl = Lepton3::DBG_NONE;

    lepton3 = new Lepton3( "/dev/spidev0.0", "/dev/i2c-0", deb_lvl ); // use SPI1 and I2C-1 ports
    lepton3->start();

    // Set initial data mode
    set_rgb_mode(rgb_mode);

    uint64_t frameIdx=0;
    uint16_t min;
    uint16_t max;
    uint8_t w,h;

    StopWatch stpWtc;

    stpWtc.tic();

    while(!close)
    {
        const uint16_t* data16 = lepton3->getLastFrame16( w, h, &min, &max );
        const uint8_t* dataRGB = lepton3->getLastFrameRGB( w, h );

        cv::Mat dispFrame;

        if( data16 || dataRGB )
        {
            double period_usec = stpWtc.toc();
            stpWtc.tic();

            double freq = (1000.*1000.)/period_usec;

            cv::Mat frame16( h, w, CV_16UC1 );
            cv::Mat frameRGB( h, w, CV_8UC3 );

            if(rgb_mode && dataRGB)
            {
                memcpy( frameRGB.data, dataRGB, 3*w*h*sizeof(uint8_t) );
                cv::cvtColor(frameRGB, dispFrame, cv::COLOR_RGB2BGR );
            }
            else if( !rgb_mode && data16 )
            {
                memcpy( frame16.data, data16, w*h*sizeof(uint16_t) );

                //cout << " * Central value: " << (int)(frame16.at<uint16_t>(w/2 + h/2*w )) << endl;

                // ----> Rescaling/Normalization to 8bit
                double diff = static_cast<double>(max - min); // Image range
                double scale = 255./diff; // Scale factor

                frame16 -= min; // Bias
                frame16 *= scale; // Rescale data

                frame16.convertTo( dispFrame, CV_8UC3 );
                // <---- Rescaling/Normalization to 8bit
            }

            cv::Mat rescaledImg;
            cv::resize( dispFrame, rescaledImg, cv::Size(), 3.0, 3.0);
            cv::imshow( "Stream", rescaledImg );
            int key = cv::waitKey(5);
            if( key == 'q' || key == 'Q')
            {
                close=true;
            }

            keyboard_handler(key);

            frameIdx++;

            if( deb_lvl>=Lepton3::DBG_INFO  )
            {
                cout << "> Frame period: " << period_usec <<  " usec - FPS: " << freq << endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    delete lepton3;

    return EXIT_SUCCESS;
}

void close_handler(int s)
{
    if(s==2)
    {
        cout << endl << "Ctrl+C pressed..." << endl;
        close = true;
    }
}

void keyboard_handler(int key)
{
    switch(key)
    {

    case 'c': // Set RGB mode
        set_rgb_mode(true);
        break;

    case 'r': // Set radiometry mode
        set_rgb_mode(false);
        break;

    case 'h':
        if( lepton3->setGainMode( LEP_SYS_GAIN_MODE_HIGH ) == LEP_OK )
        {

            LEP_SYS_GAIN_MODE_E gainMode;
            if( lepton3->getGainMode( gainMode ) == LEP_OK )
            {
                string str = (gainMode==LEP_SYS_GAIN_MODE_HIGH)?string("High"):((gainMode==LEP_SYS_GAIN_MODE_LOW)?string("Low"):string("Auto"));
                cout << " * Gain mode: " << str << endl;
            }
        }
        break;

    case 'l':
        if( lepton3->setGainMode( LEP_SYS_GAIN_MODE_LOW ) == LEP_OK )
        {

            LEP_SYS_GAIN_MODE_E gainMode;
            if( lepton3->getGainMode( gainMode ) == LEP_OK )
            {
                string str = (gainMode==LEP_SYS_GAIN_MODE_HIGH)?string("High"):((gainMode==LEP_SYS_GAIN_MODE_LOW)?string("Low"):string("Auto"));
                cout << " * Gain mode: " << str << endl;
            }
        }
        break;

    case 'a':
        if( lepton3->setGainMode( LEP_SYS_GAIN_MODE_AUTO ) == LEP_OK )
        {

            LEP_SYS_GAIN_MODE_E gainMode;
            if( lepton3->getGainMode( gainMode ) == LEP_OK )
            {
                string str = (gainMode==LEP_SYS_GAIN_MODE_HIGH)?string("High"):((gainMode==LEP_SYS_GAIN_MODE_LOW)?string("Low"):string("Auto"));
                cout << " * Gain mode: " << str << endl;
            }
        }
        break;

    case 'f':
        if( lepton3->doFFC() == LEP_OK )
        {
            cout << " * FFC completed" << endl;
        }
        break;

    case 'F':
        if( lepton3->doRadFFC() == LEP_OK )
        {
            cout << " * Radiometry FFC completed" << endl;
        }
        break;

    default:
        break;
    }
}

void set_rgb_mode(bool enable)
{
    rgb_mode = enable;

    if( lepton3->enableRadiometry( !rgb_mode ) < 0)
    {
        cerr << "Failed to set radiometry status" << endl;
    }
    else
    {
        if(!rgb_mode)
        {
            cout << " * Radiometry enabled " << endl;
        }
        else
        {
            cout << " * Radiometry disabled " << endl;
        }
    }

    // NOTE: if radiometry is enabled is unuseful to keep AGC enabled
    //       (see "FLIR LEPTON 3® Long Wave Infrared (LWIR) Datasheet" for more info)

    if( lepton3->enableAgc( rgb_mode ) < 0)
    {
        cerr << "Failed to set radiometry status" << endl;
    }
    else
    {
        if(!rgb_mode)
        {
            cout << " * AGC disabled " << endl;
        }
        else
        {
            cout << " * AGC enabled " << endl;
        }
    }

    if( lepton3->enableRgbOutput( rgb_mode ) < 0 )
    {
        cerr << "Failed to enable RGB output" << endl;
    }
    else
    {
        if(rgb_mode)
        {
            cout << " * RGB enabled " << endl;
        }
        else
        {
            cout << " * RGB disabled " << endl;
        }
    }
}
