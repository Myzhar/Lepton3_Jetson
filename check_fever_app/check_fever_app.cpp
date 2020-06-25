#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <iomanip>
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

cv::Mat frame16; // 16bit Lepton frame RAW frame

uint16_t min_raw16;
uint16_t max_raw16;

std::string temp_str;
std::string win_name = "Temperature stream";
int raw_cursor_x = -1;
int raw_cursor_y = -1;

// Hypothesis: sensor is linear in 14bit dynamic
// If the range of the sensor is [0,150] °C in High Gain mode
double temp_scale_factor = 0.0092; // 150/(2^14-1))
// <---- Global variables

// ----> Constants
const uint16_t H_TXT_INFO = 70;
const double IMG_RESIZE_FACT = 4.0;

const cv::Scalar NORM_TEMP_COL(15,200,15);
const cv::Scalar WARN_TEMP_COL(10,170,200);
const cv::Scalar FEVER_TEMP_COL(40,40,220);
const cv::Scalar TXT_COL(241,240,236);

const double min_human_temp = 34.5f;
const double warn_temp = 37.0f;
const double fever_temp = 37.5f;
const double max_human_temp = 42.0f;
// <---- Constants

// ----> Global functions
void close_handler(int s);
void keyboard_handler(int key);

void set_rgb_mode(bool enable);
void normalizeFrame(cv::Mat& in_frame16, cv::Mat& out_frame8, double min_temp, double max_temp , double rescale_fact);

void mouseCallBackFunc(int event, int x, int y, int flags, void* userdata);

cv::Mat& analizeFrame(const  cv::Mat& frame16);
// <---- Global functions

int main (int argc, char *argv[])
{
    cout << "Check Fever App for Lepton3 on Nvidia Jetson" << endl;

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

    // Disable RRGB mode to get raw 14bit values
    set_rgb_mode(false); // 16 bit raw data required

    // ----> High gain mode [0,150] Celsius
    if( lepton3->setGainMode( LEP_SYS_GAIN_MODE_HIGH ) == LEP_OK )
    {
        LEP_SYS_GAIN_MODE_E gainMode;
        if( lepton3->getGainMode( gainMode ) == LEP_OK )
        {
            string str = (gainMode==LEP_SYS_GAIN_MODE_HIGH)?string("High"):((gainMode==LEP_SYS_GAIN_MODE_LOW)?string("Low"):string("Auto"));
            cout << " * Gain mode: " << str << endl;
        }
    }
    // <---- High gain mode [0,150] Celsius

    // ----> Enable Radiometry to get values indipendent from camera temperature
    if( lepton3->enableRadiometry( true ) != LEP_OK)
    {
        cerr << "Failed to enable radiometry!" << endl;
        return EXIT_FAILURE;
    }
    // <---- Enable Radiometry to get values indipendent from camera temperature

    uint64_t frameIdx=0;

    uint8_t w,h;

    // ----> People detection thresholds
    uint16_t min_norm_raw = static_cast<uint16_t>(min_human_temp/temp_scale_factor);
    uint16_t warn_raw = static_cast<uint16_t>(warn_temp/temp_scale_factor);
    uint16_t fever_raw = static_cast<uint16_t>(fever_temp/temp_scale_factor);
    uint16_t max_raw = static_cast<uint16_t>(max_human_temp/temp_scale_factor);
    // <---- People detection thresholds

    // ----> Set OpenCV output window and mouse callback
    //Create a window
    cv::namedWindow(win_name, cv::WINDOW_AUTOSIZE);

    //set the callback function for any mouse event
    cv::setMouseCallback(win_name, mouseCallBackFunc, NULL);
    // <---- Set OpenCV output window and mouse callback

    StopWatch stpWtc;

    stpWtc.tic();

    bool initialized = false;
    cv::Mat rgbThermFrame;
    cv::Mat textInfoFrame;
    cv::Mat displayImg;
    cv::Mat thermFrame;

    while(!close)
    {
        // Retrieve last available frame
        const uint16_t* data16 = lepton3->getLastFrame16( w, h, &min_raw16, &max_raw16 );

        // ----> Initialize OpenCV data
        if(!initialized || h!=frame16.rows || w!=frame16.cols)
        {
            frame16 = cv::Mat( h, w, CV_16UC1 );
            displayImg = cv::Mat( h*IMG_RESIZE_FACT+H_TXT_INFO, w*IMG_RESIZE_FACT, CV_8UC3, cv::Scalar(0,0,0));
            textInfoFrame = displayImg(cv::Rect(0,0,w*IMG_RESIZE_FACT,H_TXT_INFO) );
            initialized = true;
        }
        // <---- Initialize OpenCV data

        if( data16 )
        {
            double period_usec = stpWtc.toc();
            stpWtc.tic();
            double freq = (1000.*1000.)/period_usec;

            // ----> Get temperature under cursor
            double curs_temp = -273.15;
            if(raw_cursor_x>=0 && raw_cursor_y>=0 &&
                    raw_cursor_x<w && raw_cursor_y<h)
            {
                // RAW value
                uint16_t value = frame16.at<uint16_t>(raw_cursor_y, raw_cursor_x);
                // Rescaling to get temperature
                curs_temp = value*temp_scale_factor+0.05;
            }
            // <---- Get temperature under cursor

            memcpy( frame16.data, data16, w*h*sizeof(uint16_t) );

            // ----> Normalization for displaying
            normalizeFrame( frame16, thermFrame, 24.0, 38.5, temp_scale_factor );
            cv::cvtColor( thermFrame, thermFrame, cv::COLOR_GRAY2BGR );
            // <---- Normalization for displaying

            // ----> Human temperature colors
            for( int i=0; i<(w*h); i++ )
            {
                double temp = data16[i]*temp_scale_factor+0.05;
                int x = i%w;
                int y = i/w;

                cv::Vec3b& temp_col = thermFrame.at<cv::Vec3b>(y,x);

                if( temp >= min_human_temp && temp < warn_temp )
                {
                    temp_col[0] = (double)temp_col[0]/255. * NORM_TEMP_COL[0];
                    temp_col[1] = (double)temp_col[1]/255. * NORM_TEMP_COL[1];
                    temp_col[2] = (double)temp_col[2]/255. * NORM_TEMP_COL[2];
                }
                else if( temp >= warn_temp && temp < fever_temp )
                {
                    temp_col[0] = WARN_TEMP_COL[0];
                    temp_col[1] = WARN_TEMP_COL[1];
                    temp_col[2] = WARN_TEMP_COL[2];
                }
                else if( temp >= fever_temp && temp < max_human_temp )
                {
                    temp_col[0] = FEVER_TEMP_COL[0];
                    temp_col[1] = FEVER_TEMP_COL[1];
                    temp_col[2] = FEVER_TEMP_COL[2];
                }
            }
            // <---- Human temperature colors

            // ----> Image resizing
            cv::resize( thermFrame, rgbThermFrame, cv::Size(), IMG_RESIZE_FACT, IMG_RESIZE_FACT, cv::INTER_CUBIC);
            rgbThermFrame.copyTo( displayImg(cv::Rect(0,H_TXT_INFO,w*IMG_RESIZE_FACT,h*IMG_RESIZE_FACT)) );
            // <---- Image resizing

            // ----> Add text info
            textInfoFrame.setTo(0);
            cv::putText( textInfoFrame, "Cursor temperature: ", cv::Point(10,20),cv::FONT_HERSHEY_SIMPLEX, 0.5, TXT_COL);
            cv::Scalar temp_col;
            if( curs_temp >= min_human_temp && curs_temp < warn_temp )
            {
                temp_col = WARN_TEMP_COL;
            }
            else if( curs_temp >= warn_temp && curs_temp < fever_temp )
            {
                temp_col = WARN_TEMP_COL;
            }
            else if( curs_temp >= fever_temp && curs_temp < max_human_temp )
            {
                temp_col = FEVER_TEMP_COL;
            }
            else
            {
                temp_col = TXT_COL;
            }

            std::stringstream sstr;
            sstr << std::fixed << std::setprecision(1) << curs_temp << " C";
            cv::putText( textInfoFrame, sstr.str(), cv::Point(175,20),cv::FONT_HERSHEY_SIMPLEX, 0.5, temp_col, 2);
            // <---- Add text info

            // Display final  result
            cv::imshow( win_name, displayImg );

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
    bool rgb_mode = enable;

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

void normalizeFrame( cv::Mat& in_frame16, cv::Mat& out_frame8, double min_temp, double max_temp , double rescale_fact)
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

void mouseCallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if ( event == cv::EVENT_MOUSEMOVE )
    {
        raw_cursor_x = x/IMG_RESIZE_FACT;
        raw_cursor_y = (y-H_TXT_INFO)/IMG_RESIZE_FACT;
    }
}
