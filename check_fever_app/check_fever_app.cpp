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
std::string win_name = "Jetson Nano Fever Control - Walter Lucetti";
int raw_cursor_x = -1;
int raw_cursor_y = -1;

// Hypothesis: sensor is linear in 14bit dynamic
// If the range of the sensor is [0,150] °C in High Gain mode
double temp_scale_factor = 0.0092; // 150/(2^14-1))

double simul_temp = 0.0;
// <---- Global variables

// ----> Defines
#define FONT_FACE cv::FONT_HERSHEY_SIMPLEX

// ----> Constants
const uint16_t H_TXT_INFO = 70;
const double IMG_RESIZE_FACT = 4.0;

const cv::Scalar COL_NORM_TEMP(15,200,15);
const cv::Scalar COL_WARN_TEMP(10,170,200);
const cv::Scalar COL_FEVER_TEMP(40,40,220);
const cv::Scalar COL_TXT(241,240,236);

const std::string STR_FEVER = "FEVER ALERT";
const std::string STR_WARN = "FEVER WARNING";
const std::string STR_NORM = "NO FEVER";
const std::string STR_NO_HUMAN = "---";

const double temp_min_human = 34.5f;
const double temp_warn = 37.0f;
const double temp_fever = 37.5f;
const double temp_max_human = 42.0f;

const double fever_thresh = 0.05;
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

    // ----> Setup GUI
    int baseline;
    cv::Size size_fever = cv::getTextSize( STR_FEVER, FONT_FACE, 0.95, 2, &baseline );
    cv::Size size_warn = cv::getTextSize( STR_WARN, FONT_FACE, 0.95, 2, &baseline );
    cv::Size size_norm = cv::getTextSize( STR_NORM, FONT_FACE, 0.95, 2, &baseline );
    cv::Size size_none = cv::getTextSize( STR_NO_HUMAN, FONT_FACE, 0.95, 2, &baseline );

    cv::Size size_status_str = cv::getTextSize( "STATUS", FONT_FACE, 0.75, 1, &baseline );
    // <---- Setup GUI

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

    // ----> Set OpenCV output window and mouse callback
    //Create a window
    cv::namedWindow(win_name, cv::WINDOW_AUTOSIZE);

    //set the callback function for any mouse event
    cv::setMouseCallback(win_name, mouseCallBackFunc, NULL);
    // <---- Set OpenCV output window and mouse callback

    StopWatch stpWtc;

    stpWtc.tic();

    bool doFFC = true;
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

            if( curs_temp >= temp_min_human && curs_temp <= temp_max_human )
            {
                curs_temp += simul_temp;
            }
            // <---- Get temperature under cursor

            // Data copy on OpenCV image
            memcpy( frame16.data, data16, w*h*sizeof(uint16_t) );

            // ----> Normalization for displaying
            normalizeFrame( frame16, thermFrame, temp_min_human-7.5, temp_fever+1.0, temp_scale_factor );
            // Conversion to RGB
            cv::cvtColor( thermFrame, thermFrame, cv::COLOR_GRAY2BGR );
            // <---- Normalization for displaying

            // ----> Human temperature colors

            int count_human=0;
            int count_warning=0;
            int count_fever=0;
            for( int i=0; i<(w*h); i++ )
            {
                // Temperature from thermal data adjusted for simulation
                double temp = (data16[i]*temp_scale_factor+0.05);

                if( temp >= temp_min_human && temp <= temp_max_human )
                {
                    count_human++;
                    temp += simul_temp;

                    int u = i%w;
                    int v = i/w;
                    cv::Vec3b& temp_col = thermFrame.at<cv::Vec3b>(v,u);

                    if( temp >= temp_min_human && temp < temp_warn )
                    {
                        temp_col[0] = (double)temp_col[0]/255. * COL_NORM_TEMP[0];
                        temp_col[1] = (double)temp_col[1]/255. * COL_NORM_TEMP[1];
                        temp_col[2] = (double)temp_col[2]/255. * COL_NORM_TEMP[2];
                    }
                    else if( temp >= temp_warn && temp < temp_fever )
                    {
                        temp_col[0] = (double)temp_col[0]/255. * COL_WARN_TEMP[0];
                        temp_col[1] = (double)temp_col[1]/255. * COL_WARN_TEMP[1];
                        temp_col[2] = (double)temp_col[2]/255. * COL_WARN_TEMP[2];
                        count_warning++;
                    }
                    else if( temp >= temp_fever && temp <= temp_max_human )
                    {
                        temp_col[0] = (double)temp_col[0]/255. * COL_FEVER_TEMP[0];
                        temp_col[1] = (double)temp_col[1]/255. * COL_FEVER_TEMP[1];
                        temp_col[2] = (double)temp_col[2]/255. * COL_FEVER_TEMP[2];
                        count_fever++;
                    }
                }
            }
            // <---- Human temperature colors

            // Image resizing
            cv::resize( thermFrame, rgbThermFrame, cv::Size(), IMG_RESIZE_FACT, IMG_RESIZE_FACT, cv::INTER_CUBIC);

            // ----> Draw cursor
            if(curs_temp!=-273.15)
            {
                cv::drawMarker( rgbThermFrame, cv::Point(raw_cursor_x*IMG_RESIZE_FACT,raw_cursor_y*IMG_RESIZE_FACT),
                                cv::Scalar(200,20,20), cv::MARKER_CROSS, 15, 2, cv::LINE_AA );
            }
            // <---- Draw cursor

            rgbThermFrame.copyTo( displayImg(cv::Rect(0,H_TXT_INFO,w*IMG_RESIZE_FACT,h*IMG_RESIZE_FACT)) );

            // Clean text info area
            textInfoFrame.setTo(0);

            // ----> Cursor temperature text info
            cv::Scalar temp_col;
            if( curs_temp >= temp_min_human && curs_temp < temp_warn )
            {
                temp_col = COL_NORM_TEMP;
            }
            else if( curs_temp >= temp_warn && curs_temp < temp_fever )
            {
                temp_col = COL_WARN_TEMP;
            }
            else if( curs_temp >= temp_fever && curs_temp < temp_max_human )
            {
                temp_col = COL_FEVER_TEMP;
            }
            else
            {
                temp_col = COL_TXT;
            }

            cv::putText( textInfoFrame, "Cursor temperature: ", cv::Point(10,20),FONT_FACE, 0.5, COL_TXT, 1, cv::LINE_AA );

            std::stringstream sstr_temp;
            sstr_temp << std::fixed << std::setprecision(1) << curs_temp << " C";
            if( curs_temp != -273.15 )
                cv::putText( textInfoFrame, sstr_temp.str(), cv::Point(180,20),FONT_FACE, 0.5, temp_col, 2, cv::LINE_AA);
            else
                cv::putText( textInfoFrame, "---", cv::Point(175,20),FONT_FACE, 0.5, temp_col, 2, cv::LINE_AA);
            // <---- Cursor temperature text info

            // ----> Simulated temperature text info
            std::stringstream sstr_simul;
            sstr_simul << std::fixed << std::showpos << std::setprecision(1) << "Simulated temperature: " << simul_temp << " C";
            cv::putText( textInfoFrame, sstr_simul.str(), cv::Point(10,50),FONT_FACE, 0.5, COL_TXT, 1, cv::LINE_AA);

            cv::line( textInfoFrame, cv::Point(textInfoFrame.cols/2,0), cv::Point(textInfoFrame.cols/2,textInfoFrame.rows),
                      cv::Scalar::all(255), 2, cv::LINE_AA );
            // <---- Simulated temperature text info

            // ----> Fever status text info
            std::string fever_str = STR_NO_HUMAN;
            cv::Scalar fever_col = COL_TXT;
            int txt_fever_x = textInfoFrame.cols/2 + (textInfoFrame.cols/2-size_none.width)/2;
            int txt_status_x = textInfoFrame.cols/2 + (textInfoFrame.cols/2-size_status_str.width)/2;
            if( count_human>0 )
            {
                if( ((double)count_fever/count_human)>fever_thresh )
                {
                    fever_str = STR_FEVER;
                    fever_col = COL_FEVER_TEMP;
                    txt_fever_x = textInfoFrame.cols/2 +(textInfoFrame.cols/2-size_fever.width)/2;
                }
                else if( ((double)count_warning/count_human)>fever_thresh )
                {
                    fever_str = STR_WARN;
                    fever_col = COL_WARN_TEMP;
                    txt_fever_x = textInfoFrame.cols/2 +(textInfoFrame.cols/2-size_warn.width)/2;
                }
                else
                {
                    fever_str = STR_NORM;
                    fever_col = COL_NORM_TEMP;
                    txt_fever_x = textInfoFrame.cols/2 +(textInfoFrame.cols/2-size_norm.width)/2;
                };
            }

            cv::putText( textInfoFrame, "STATUS", cv::Point(txt_status_x,25), FONT_FACE, 0.75, COL_TXT, 1, cv::LINE_AA);
            cv::putText( textInfoFrame, fever_str, cv::Point(txt_fever_x,58), FONT_FACE, 0.95, fever_col, 2, cv::LINE_AA);
            // <---- Fever status text info

            if(++frameIdx == 1000)
            {
                frameIdx=0;
                doFFC = true;
            }

            if( deb_lvl>=Lepton3::DBG_INFO  )
            {
                cout << "> Frame period: " << period_usec <<  " usec - FPS: " << freq << endl;
            }
        }

        if( doFFC )
        {
            cv::putText( displayImg , "FFC normalization...", cv::Point(10,100), FONT_FACE, 0.5, cv::Scalar(100,10,10), 2, cv::LINE_AA);
        }

        // Display final  result
        cv::imshow( win_name, displayImg );

        int key = cv::waitKey(5);
        if( key == 'q' || key == 'Q')
        {
            close=true;
        }

        keyboard_handler(key);

        if( doFFC )
        {
            if( lepton3->doFFC() == LEP_OK )
            {
                cout << " * FFC completed" << endl;
            }

            doFFC = false;
        }
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

    case '+':
    case 'u':
    case 'U':
        simul_temp += 0.1;
        if(simul_temp>3.0)
            simul_temp = 3.0;
        break;

    case '-':
    case 'd':
    case 'D':
        simul_temp -= 0.1;
        if(simul_temp<0.0)
            simul_temp = 0.0;
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
