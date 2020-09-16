#include "zedgrabber.h"
#include <QDebug>

ZedGrabber::ZedGrabber(QObject *parent) : QObject(parent)
{

}

ZedGrabber::~ZedGrabber()
{
    mStopRequest = true;

    if( mGrabThread.joinable() )
    {
        mGrabThread.join();
    }
}

bool ZedGrabber::openZed()
{
    emit statusMessage(tr("Opening ZED2"));

    if(mZed.isOpened())
    {
        mZed.close();
    }

    sl::InitParameters initParams;
    initParams.camera_fps = mZedFramerate;
    initParams.camera_resolution = mZedResol;
    initParams.coordinate_units = sl::UNIT::METER;

    sl::ERROR_CODE res = mZed.open( initParams );

    if( sl::ERROR_CODE::SUCCESS != res)
    {
        return false;
    }

    emit statusMessage(tr("ZED2 ready"));

    if( mZed.getCameraInformation().camera_model != sl::MODEL::ZED2)
    {
        return false;
    }

    mZedW = mZed.getCameraInformation().camera_configuration.resolution.width;
    mZedH = mZed.getCameraInformation().camera_configuration.resolution.height;

    return true;
}

void ZedGrabber::startCapture()
{
    emit statusMessage(tr("Starting ZED capture"));
    mGrabThread = std::thread( &ZedGrabber::grabFunc, this );
}

sl::Mat& ZedGrabber::getLastImage()
{
    return mZedRgb;
}

sl::Objects& ZedGrabber::getLastObjDet()
{
    return mZedObj;
}

void ZedGrabber::grabFunc()
{
    qDebug() << tr("Grabber thread started");

    mStopRequest = false;

    sl::ERROR_CODE err;

    sl::RuntimeParameters runtimeParams;
    runtimeParams.enable_depth = true;

    mObjRtParams.detection_confidence_threshold = 50.f;

    emit statusMessage(tr("Starting ZED2 tracking module..."));

    // ----> Positional tracking required by Object Detection module
    sl::PositionalTrackingParameters posTrackParams;
    posTrackParams.set_as_static=true;
    err = mZed.enablePositionalTracking(posTrackParams);
    if(err != sl::ERROR_CODE::SUCCESS)
    {
        qDebug() << tr("Error starting Positional Tracking: %1").arg(sl::toString(err).c_str());
        return;
    }
    // <---- Positional tracking required by Object Detection module

    emit statusMessage(tr("Starting ZED2 AI module..."));

    //std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    // ----> Object Detection to detect Skeletons
    sl::ObjectDetectionParameters objDetPar;
    objDetPar.detection_model = sl::DETECTION_MODEL::HUMAN_BODY_FAST;
    objDetPar.enable_tracking = true;
    objDetPar.image_sync = false;
    objDetPar.enable_mask_output = false;
    err = mZed.enableObjectDetection( objDetPar );
    if(err != sl::ERROR_CODE::SUCCESS)
    {
        qDebug() << tr("Error starting Object Detectiom: %1").arg(sl::toString(err).c_str());
        return;
    }
    // <---- Object Detection to detect Skeletons

    emit statusMessage(tr("Stereolabs ZED2 ready"));

    forever
    {
        if( mStopRequest )
        {
            break;
        }

        err  = mZed.grab( runtimeParams );

        if(err != sl::ERROR_CODE::SUCCESS)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500/mZedFramerate));
        }

        {
            const std::lock_guard<std::mutex> lock(mImgMutex);
            err = mZed.retrieveImage(mZedRgb, sl::VIEW::LEFT );
            if(err != sl::ERROR_CODE::SUCCESS)
            {
                qDebug() << tr("Error Retrieving Image: %1").arg(sl::toString(err).c_str());
                std::this_thread::sleep_for(std::chrono::milliseconds(500/mZedFramerate));
                continue;
            }
            emit zedImageReady();
        }

        err = mZed.retrieveMeasure(mZedDepth, sl::MEASURE::DEPTH);
        if(err != sl::ERROR_CODE::SUCCESS)
        {
            qDebug() << tr("Error Retrieving Depth: %1").arg(sl::toString(err).c_str());
            std::this_thread::sleep_for(std::chrono::milliseconds(500/mZedFramerate));
            continue;
        }

        {
            const std::lock_guard<std::mutex> lock(mObjMutex);
            err = mZed.retrieveObjects(mZedObj, mObjRtParams );
            if(err != sl::ERROR_CODE::SUCCESS)
            {
                qDebug() << tr("Error Retrieving Objects: %1").arg(sl::toString(err).c_str());
                std::this_thread::sleep_for(std::chrono::milliseconds(500/mZedFramerate));
                continue;
            }

            if(mZedObj.is_new)
            {
                emit zedObjListReady();
                //qDebug() << tr("Found %1 persons").arg(mZedObj.object_list.size());
            }
        }
    }

    qDebug() << tr("Grabber thread finished");
}
