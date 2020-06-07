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

// >>>>> Demo configuration
#define USE_RGB false

#define SHOW_STREAM true
// <<<<< Demo configuration

using namespace std;

static bool close = false;

void close_handler(int s)
{
    if(s==2)
    {
        cout << endl << "Ctrl+C pressed..." << endl;
        close = true;
    }
}

int main (int argc, char *argv[])
{
    cout << "OpenCV demo for Lepton3 on BeagleBoard Blue" << endl;

    // >>>>> Enable Ctrl+C
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = close_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
    // <<<<< Enable Ctrl+C

    Lepton3::DebugLvl deb_lvl = Lepton3::DBG_NONE;

    if( argc == 2 )
    {
        int dbg = atoi(argv[1]);

        switch( dbg )
        {
        case 1:
            deb_lvl = Lepton3::DBG_INFO;
            break;

        case 2:
            deb_lvl = Lepton3::DBG_FULL;
            break;

        default:
        case 0:
            deb_lvl = Lepton3::DBG_NONE;
            break;
        }
    }

    Lepton3 lepton3( "/dev/spidev0.0", "/dev/i2c-0", deb_lvl );
    
    if( lepton3.enableRadiometry( !USE_RGB ) < 0)
    {
        cerr << "Failed to set radiometry status" << endl;
    }
    else
    {
        if(!USE_RGB)
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
    
    if( lepton3.enableAgc( USE_RGB ) < 0)
    {
        cerr << "Failed to set radiometry status" << endl;
    }
    else
    {
        if(!USE_RGB)
        {
            cout << " * AGC disabled " << endl;
        }
        else
        {
            cout << " * AGC enabled " << endl;
        }
    }

    if( lepton3.enableRgbOutput( USE_RGB ) < 0 )
    {
        cerr << "Failed to enable RGB output" << endl;
    }
    else
    {
        if(USE_RGB)
        {
            cout << " * RGB enabled " << endl;
        }
        else
        {
            cout << " * RGB disabled " << endl;
        }
    }

    LEP_SYS_GAIN_MODE_E gainMode;
    if( lepton3.getGainMode( gainMode ) == LEP_OK )
    {
        string str = (gainMode==LEP_SYS_GAIN_MODE_HIGH)?string("High"):((gainMode==LEP_SYS_GAIN_MODE_LOW)?string("Low"):string("Auto"));
        cout << " * Gain mode: " << str << endl;
    }

    lepton3.start();

    uint64_t frameIdx=0;

    StopWatch stpWtc;

    stpWtc.tic();

    while(!close)
    {
        uint16_t min;
        uint16_t max;
        uint8_t w,h;

        bool rgb = lepton3.isRgbEnable();

        const uint16_t* data16 = lepton3.getLastFrame16( w, h, &min, &max );
        const uint8_t* dataRGB = lepton3.getLastFrameRGB( w, h );

        cv::Mat dispFrame;

        if( data16 || dataRGB )
        {
            double period_usec = stpWtc.toc();
            stpWtc.tic();

            double freq = (1000.*1000.)/period_usec;
            cv::Mat frame16( h, w, CV_16UC1 );
            cv::Mat frameRGB( h, w, CV_8UC3 );

            if(rgb && dataRGB)
            {
                memcpy( frameRGB.data, dataRGB, 3*w*h*sizeof(uint8_t) );
                cv::cvtColor(frameRGB,dispFrame, cv::COLOR_RGB2BGR );
            }
            else if( !rgb && data16 )
            {
                memcpy( frame16.data, data16, w*h*sizeof(uint16_t) );

                // >>>>> Rescaling/Normalization to 8bit
                double diff = static_cast<double>(max - min); // Image range
                double scale = 255./diff; // Scale factor

                frame16 -= min; // Bias
                frame16 *= scale; // Rescale data

                cv::Mat frame8;
                frame16.convertTo( frame8, CV_8UC1 );
                // <<<<< Rescaling/Normalization to 8bit

                dispFrame = frame8;
            }

            cv::Mat rescaledImg;
            cv::resize( dispFrame, rescaledImg, cv::Size(), 3.0,3.0);
            cv::imshow( "Stream", rescaledImg );
            int key = cv::waitKey(5);

            if( key == 'q' || key == 'Q')
            {
                close=true;
            }

            frameIdx++;

            if( deb_lvl>=Lepton3::DBG_INFO  )
            {
                cout << "> Frame period: " << period_usec <<  " usec - FPS: " << freq << endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(110));
    }

    return EXIT_SUCCESS;
}
