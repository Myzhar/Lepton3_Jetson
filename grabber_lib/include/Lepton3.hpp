#ifndef LEPTON3
#define LEPTON3

#include <ctime>
#include <stdint.h>

#include <iostream>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "LEPTON_Types.h"
#include "LEPTON_ErrorCodes.h"
#include "LEPTON_SYS.h"
#include "LEPTON_OEM.h"

#include "stopwatch.hpp"

#define MULTI_BUFF_SIZE 2

class Lepton3
{
public:     
    typedef enum _debug_lvl { DBG_NONE=0, DBG_INFO=1, DBG_FULL=2 } DebugLvl;


    /*! Default constructor */
    Lepton3(std::string spiDevice="/dev/spidev1.0", std::string i2c_port="/dev/i2c-0", DebugLvl dbgLvl=DBG_FULL);
    
    /*! Destructor */
    virtual ~Lepton3();

    
    void start(); //!< Start grabbing thread
    void stop();  //!< Stop grabbing thread

    /*! Returns last available 16bit frame as vector not normalized
     *   
     * @param width return the width of the frame
     * @param height return the height of the frame
     * @param min if not NULL return the minimum value
     * @param max if not NULL return the maximum value
     *
     * @return a pointer to last available data vector
     */
    const uint16_t* getLastFrame16( uint8_t& width, uint8_t& height,
                                  uint16_t* min=NULL, uint16_t* max=NULL );

    /*! Returns last available RGB frame as vector
     *
     * @param width return the width of the frame
     * @param height return the height of the frame
     *
     * @return a pointer to last available data vector
     */
    const uint8_t* getLastFrameRGB( uint8_t& width, uint8_t& height );
    
    // ----> Controls
    LEP_RESULT getSensorTemperatureK(float& tempK); //!< Get Temperature of the Flir Sensor in °K
    
    LEP_RESULT enableRadiometry( bool enable );    //!< Enable/Disable radiometry
    LEP_RESULT getRadiometryStatus(bool &status);  //!< Verify if Radiometry is enabled or not

    LEP_RESULT getAgcStatus(bool &status);         //!< Verify if AGC is enabled or not
    LEP_RESULT enableAgc( bool enable );           //!< Enable/Disable AGC
    
    LEP_RESULT getGainMode( LEP_SYS_GAIN_MODE_E& mode); //!< Get Lepton3 gain mode
    LEP_RESULT setGainMode( LEP_SYS_GAIN_MODE_E newMode); //!< Set Lepton3 gain mode

    // ----> Not yet available on Lepton3
    //LEP_RESULT getSpotROI( uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h ); //!< Get Spotmeter region
    //LEP_RESULT setSpotROI( uint16_t x, uint16_t y, uint16_t w, uint16_t h );     //!< Set Spotmeter region
    //LEP_RESULT getSpotInfo( float& valueK, float& minK, float& maxK, uint16_t& count ); //!< Get Spotmeter info
    // <---- Not yet available on Lepton3

    LEP_RESULT enableTelemetry( bool enable ); //!< Enable/Disable telemetry
    LEP_RESULT getTelemetryStatus( bool &status ); //!< Verify if telemetry is enabled

    // TODO Add telemetry parsing function

    LEP_RESULT getVideoOutputFormat( LEP_OEM_VIDEO_OUTPUT_FORMAT_E& format  ); //!< Get Video Output format
    
    bool isRgbEnable(){return mRgbEnabled;} //!< Verify if RGB video format is enabled
    LEP_RESULT enableRgbOutput( bool enable ); //!< Enable/Disable RGB video output format
    LEP_RESULT setRgbLut( ); //!< Set RGB LUT    
    
    LEP_RESULT doFFC(); //!< Performs FFC Normalization  
    LEP_RESULT doRadFFC(); //!< Performs Radiometric FFC Normalization
    
    LEP_RESULT resetCamera(); //!< Perform a reboot without recovering the former status
    LEP_RESULT rebootCamera(); //!< Perform a camera reboot recovering the former status

    LEP_RESULT saveParams(); //!< Save current parameters to OTP. They will be reloaded after reboot (@NOTE: requires 5.9V on PIN 17 - VPROG, see Datasheet pg 34)
    LEP_RESULT loadParams(); //!< Reload saved parameters from OTP.
    // <---- Controls

protected:
    void thread_func();

    bool SpiOpenPort();     //!< Open VoSPI port
    void SpiClosePort();    //!< Close VoSPI port
    int SpiReadSegment();   //!< Read a new VoSPI segment and returns its ID
    void resync();          //!< Resync VoSPI communication after de-sync

    void raw2data16();      //!< Convert RAW 14bit VoSPI frame to 16bit data matrix
    void raw2RGB();         //!< Convert RAW RGB888 VoSPI frame to RGB data matrix

    void setVoSPIData();    //!< Set VoSPI data values according to the configuration of the sensor

private:
    // ----> VoSPI
    std::string mSpiDevice; //!< SPI port device name
    int mSpiFd;             //!< SPI descriptor
    unsigned char mSpiMode; //!< SPI mode
    unsigned char mSpiBits; //!< SPI bits per words
    unsigned int mSpiSpeed; //!< SPI max speed

    uint8_t mPacketCount;   //!< VoSPI Packet for each segment
    uint8_t mPacketSize;    //!< VoSPI Packet size in bytes
    uint8_t mSegmentCount;  //!< VoSPI segment for each frame
    uint32_t mSegmSize;     //!< Size of the single segment

    uint8_t* mSpiRawFrameBuf[MULTI_BUFF_SIZE];      //!< VoSPI buffer to keep 4 consecutive valid segments
    uint32_t mSpiRawFrameBufSize;  //!< Size of the buffer for 4 segments

    uint16_t* mDataFrameBuf16[MULTI_BUFF_SIZE];      //!< RAW 16bit frame buffer
    uint8_t* mDataFrameBufRGB[MULTI_BUFF_SIZE];      //!< RGB888 frame buffer

    int mBuffIdx;

    double mSegmentFreq;    //!< VoSPI Segment output frequency
    
    struct spi_ioc_transfer mSpiTR; //!< Data structure for SPI ioctl

    int mCurrSegm;          //!< Index of the last valid acquired segment (-1 if Lost Sync)
    // <---- VoSPI

    // ----> Lepton control (CCI)
    bool mCciConnected;
    std::string mCciPort;
    LEP_CAMERA_PORT_DESC_T mCciConnPort;

    bool CciConnect();      //!< Connect CCI (I2C)
    // <---- Lepton control (CCI)

    std::thread mThread;
    std::mutex mBuffMutex;

    bool mStop;

    DebugLvl mDebugLvl;
    
    StopWatch mThreadWatch;

    bool mDataValid[MULTI_BUFF_SIZE];
    
    uint16_t mMin;
    uint16_t mMax;

    bool mRgbEnabled;

    int mResyncCount;    //!< Number of consecutive resync
    int mTotResyncCount; //!< Number of total resync
};



#endif // LEPTONTHREAD
