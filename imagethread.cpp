﻿#include "imagethread.h"
#include <QDebug>
ImageThread::ImageThread()
{
    m_leftCamDev  = new CameraDevice;
    m_rightCamDev = new CameraDevice;
    m_local_algro = new Local_Algorithm;
    CamSelect     = DEFAULT_CAMERA;
    isOpenLeftCam   =  false;
    isOpenRightCam  =  false;
    isOpenAllCam    =  false;
    iscloseLeftCam  =  false;
    iscloseRightCam =  false;
    iscloseAllCam   =  false;
    isCaliCam       =  false;
    isneedCamCali   =  true;
    ControlCamSet = Mode_ProcCAM;
    qRegisterMetaType<Mat>("Mat");
}

QImage ImageThread::convertMatToQImage(cv::Mat &mat)
{
    QImage img;
    int nChannel=mat.channels();
    if(nChannel==3)
    {
        cv::cvtColor(mat,mat,CV_BGR2RGB);
        img = QImage((const unsigned char*)mat.data,mat.cols,mat.rows,QImage::Format_RGB888);
    }
    else if(nChannel==4)
    {
        img = QImage((const unsigned char*)mat.data,mat.cols,mat.rows,QImage::Format_ARGB32);
    }
    else if(nChannel==1)
    {
        img = QImage((const unsigned char*)mat.data,mat.cols,mat.rows,QImage::Format_Indexed8);
    }

    return img;
}

void ImageThread::accept_isneedCamcali(bool arg)
{

    isneedCamCali = arg;
}

void ImageThread::accept_CamSetInfo(int leftCamindex, bool isleftSelect, int rightCamindex, bool isrightSelect, bool AllSelect,int camMode)
{
    ControlCamSet   = camMode;
    if(AllSelect)
    {
        CamSelect = ALL_CAMERA;
        m_leftCamDev->setCamName(leftCamindex);
        m_rightCamDev->setCamName(rightCamindex);
    }
    else
    {
        if(isleftSelect)
        {
            CamSelect = LEFT_CAMERA;
            m_leftCamDev->setCamName(leftCamindex);
        }
        if(isrightSelect)
        {
            CamSelect = RIGHT_CAMERA;
            m_rightCamDev->setCamName(rightCamindex);
        }
    }
}

void ImageThread::accept_ControlCaminfo(bool leftCamrelea, bool rightCamrelea, bool allCamrelea)
{
    isOpenLeftCam  = leftCamrelea;
    isOpenRightCam = rightCamrelea;
    isOpenAllCam   = allCamrelea;
}

void ImageThread::accept_CloseCaminfo(bool leftCamrelea, bool rightCamrelea, bool allCamrelea)
{
    iscloseLeftCam  = leftCamrelea;
    iscloseRightCam = rightCamrelea;
    iscloseAllCam   = allCamrelea;

}

void ImageThread::accept_CaliCaminfo(bool isCaliCame,int board_width,int board_height,int per_board_width,int per_board_height)
{
    isCaliCam = isCaliCame;
    m_local_algro->board_size.width = board_width;
    m_local_algro->board_size.height = board_height;
    m_local_algro->real_square_size.width = per_board_width;
    m_local_algro->real_square_size.height = per_board_height;
}


void ImageThread::run()
{
    switch(ControlCamSet)
    {
    case Mode_ProcCAM:
    {
        m_local_algro->returnLeftCam();
        m_local_algro->returnRightCam();
        m_local_algro->returnDoubleCam();
        switch (CamSelect) {
        case LEFT_CAMERA:
            if(isneedCamCali)
            {

                m_leftCam = new cv::VideoCapture(m_leftCamDev->returnCamdevIndex());
                while (oriLeftFrame.rows < 2) {
                    m_leftCam->open(0);
                    m_leftCam->set(CAP_PROP_FOURCC, 'GPJM');
                    m_leftCam->set(CAP_PROP_FRAME_WIDTH, 320);
                    m_leftCam->set(CAP_PROP_FRAME_HEIGHT, 240);
                    cont = 0;
                    while (oriLeftFrame.rows < 2 && cont<5) {
                        m_leftCam->operator >>(oriLeftFrame);
                        cont++;
                    }
                }
            }
            else
            {
                m_leftCam = new cv::VideoCapture(m_leftCamDev->returnCamdevIndex());
            }
            while(1)
            {
                if(isOpenLeftCam)
                {
                    if(iscloseLeftCam)
                    {
                        m_leftCam->release();
                        break;
                    }
                }
                else
                {
                    if(isneedCamCali)
                    {
                        m_leftCam->operator >>(oriLeftFrame);
                        cv::Mat leftcorrectImage;
                        undistort(oriLeftFrame,leftcorrectImage,m_local_algro->leftcameraMatrix,m_local_algro->leftdistCoeffs);
                        cv::Mat templeftframe;
                        cv::resize(leftcorrectImage,templeftframe,cv::Size(320,240));
                        new_send_leftImageDisp(templeftframe);
                    }
                    else
                    {
                        m_leftCam->operator >>(oriLeftFrame);
                        cv::Mat templeftframe;
                        cv::resize(oriLeftFrame,templeftframe,cv::Size(320,240));
                        new_send_leftImageDisp(templeftframe);
                    }
                }
            }
            break;
        case RIGHT_CAMERA:
            if(isneedCamCali)
            {
                m_rightCam    = new cv::VideoCapture(m_rightCamDev->returnCamdevIndex());
                while (oriRightFrame.rows < 2) {
                    m_rightCam->open(1);
                    m_rightCam->set(CAP_PROP_FOURCC, 'GPJM');
                    m_rightCam->set(CAP_PROP_FRAME_WIDTH, 320);
                    m_rightCam->set(CAP_PROP_FRAME_HEIGHT, 240);
                    cont = 0;
                    while (oriRightFrame.rows < 2 && cont<5) {
                        m_rightCam->operator >>(oriRightFrame);
                        cont++;
                    }
                }
            }
            else
            {
                m_rightCam    = new cv::VideoCapture(m_rightCamDev->returnCamdevIndex());
            }

            while(1)
            {
                if(isOpenRightCam)
                {
                    if(iscloseRightCam)
                    {
                        m_rightCam->release();
                        break;
                    }
                }
                else
                {
                    if(isneedCamCali)
                    {
                        m_rightCam->operator >>(oriRightFrame);
                        cv::Mat rightcorrectImage;
                        undistort(oriRightFrame,rightcorrectImage,m_local_algro->rightcameraMatrix,m_local_algro->rightdistCoeffs);
                        cv::Mat temprightframe;
                        cv::resize(rightcorrectImage,temprightframe,cv::Size(320,240));
                        new_send_rightImageDisp(temprightframe);
                    }
                    else
                    {
                        m_rightCam->operator >>(oriRightFrame);
                        cv::Mat temprightframe;
                        cv::resize(oriRightFrame,temprightframe,cv::Size(320,240));
                        new_send_rightImageDisp(temprightframe);
                    }
                }
            }
            break;
        case ALL_CAMERA:
            if(isneedCamCali)
            {
                m_leftCam  = new cv::VideoCapture(m_leftCamDev->returnCamdevIndex());
                m_rightCam = new cv::VideoCapture(m_rightCamDev->returnCamdevIndex());
                while (oriLeftFrame.rows < 2) {
                    m_leftCam->open(0);
                    m_leftCam->set(CAP_PROP_FOURCC, 'GPJM');
                    m_leftCam->set(CAP_PROP_FRAME_WIDTH, 320);
                    m_leftCam->set(CAP_PROP_FRAME_HEIGHT, 240);
                    cont = 0;
                    while (oriLeftFrame.rows < 2 && cont<5) {
                        m_leftCam->operator >>(oriLeftFrame);
                        cont++;
                    }
                }
                while (oriRightFrame.rows < 2) {
                    m_rightCam->open(1);
                    m_rightCam->set(CAP_PROP_FOURCC, 'GPJM');
                    m_rightCam->set(CAP_PROP_FRAME_WIDTH, 320);
                    m_rightCam->set(CAP_PROP_FRAME_HEIGHT, 240);
                    cont = 0;
                    while (oriRightFrame.rows < 2 && cont<5) {
                        m_rightCam->operator >>(oriRightFrame);
                        cont++;
                    }
                }
            }
            else
            {
                m_leftCam  = new cv::VideoCapture(m_leftCamDev->returnCamdevIndex());
                m_rightCam = new cv::VideoCapture(m_rightCamDev->returnCamdevIndex());
            }

            while(1)
            {
                if(isOpenAllCam)
                {

                    if(iscloseAllCam)
                    {
                        m_leftCam->release();
                        m_rightCam->release();
                        break;
                    }
                }
                else
                {
                    if(isneedCamCali)
                    {
                        m_leftCam->operator >>(oriLeftFrame);
                        m_rightCam->operator >>(oriRightFrame);
                        cv::Mat leftcorrectImage;
                        remap(oriLeftFrame, leftcorrectImage, m_local_algro->mapLx, m_local_algro->mapLy, INTER_LINEAR);
                        //undistort(oriLeftFrame,leftcorrectImage,m_local_algro->leftcameraMatrix,m_local_algro->leftdistCoeffs);
                        cv::Mat rightcorrectImage;
                        remap(oriRightFrame, rightcorrectImage, m_local_algro->mapRx,m_local_algro-> mapRy, INTER_LINEAR);
                        //undistort(oriRightFrame,rightcorrectImage,m_local_algro->rightcameraMatrix,m_local_algro->rightdistCoeffs);
                        cv::Ptr<cv::StereoBM> bm = cv::StereoBM::create(16, 9);
                        bm->setPreFilterType(CV_STEREO_BM_NORMALIZED_RESPONSE);
                        bm->setPreFilterCap(31);
                        bm->setBlockSize(9);
                        bm->setMinDisparity(0);
                        int numberOfDisparities =((rightcorrectImage.size().width/8) + 15) & -16;
                        bm->setNumDisparities(numberOfDisparities);
                        bm->setTextureThreshold(10);
                        bm->setUniquenessRatio(15);
                        bm->setSpeckleWindowSize(100);
                        bm->setSpeckleRange(32);
                        bm->setDisp12MaxDiff(1);
                        Ptr<StereoSGBM> sgbm = StereoSGBM::create(0,16,3);
                        sgbm->setPreFilterCap(15);
                        int sgbmWinSize = 11;
                        sgbm->setBlockSize(sgbmWinSize);

                        int cn = leftcorrectImage.channels();

                        sgbm->setP1(8*cn*sgbmWinSize*sgbmWinSize);
                        sgbm->setP2(64*cn*sgbmWinSize*sgbmWinSize);
                        sgbm->setMinDisparity(0);
                        sgbm->setNumDisparities(numberOfDisparities);
                        sgbm->setUniquenessRatio(10);
                        sgbm->setSpeckleWindowSize(100);
                        sgbm->setSpeckleRange(32);
                        sgbm->setDisp12MaxDiff(1);
                        sgbm->setMode(StereoSGBM::MODE_SGBM);

                        cv::Mat disImage;
                        Mat grayleftcorrectImage,grayrightcorrectImage;
                        cvtColor(leftcorrectImage,grayleftcorrectImage,COLOR_BGR2GRAY);
                        cvtColor(rightcorrectImage,grayrightcorrectImage,COLOR_BGR2GRAY);
                        //bm->compute(grayleftcorrectImage,grayrightcorrectImage,disImage);
                       copyMakeBorder(grayleftcorrectImage, grayleftcorrectImage, 0, 0, numberOfDisparities, 0, IPL_BORDER_REPLICATE);  //防止黑边
                        copyMakeBorder(grayrightcorrectImage, grayrightcorrectImage, 0, 0,numberOfDisparities, 0, IPL_BORDER_REPLICATE);
                        sgbm->compute(grayleftcorrectImage,grayrightcorrectImage,disImage);
                        disImage = disImage.colRange(numberOfDisparities,grayleftcorrectImage.cols);
                        disImage.convertTo(disImage, CV_8U, 255/(numberOfDisparities*16.));
                        cv::Mat templeftframe;
                        cv::resize(leftcorrectImage,templeftframe,cv::Size(320,240));
                        cv::Mat temprightframe;
                        cv::resize(rightcorrectImage,temprightframe,cv::Size(320,240));
                        new_send_allImageDisp(templeftframe,temprightframe);
                    }
                    else
                    {
                        m_leftCam->operator >>(oriLeftFrame);
                        m_rightCam->operator >>(oriRightFrame);
                        cv::Mat templeftframe;
                        cv::resize(oriLeftFrame,templeftframe,cv::Size(320,240));
                        cv::Mat temprightframe;
                        cv::resize(oriRightFrame,temprightframe,cv::Size(320,240));
                        new_send_allImageDisp(templeftframe,temprightframe);
                    }
                }
            }
            break;
        default:
            break;
        }
    }
        break;
    case Mode_CALICAM:
    {
        switch (CamSelect) {
        case LEFT_CAMERA:
            m_leftCam = new cv::VideoCapture(m_leftCamDev->returnCamdevIndex());
            while(1)
            {
                if(isOpenLeftCam)
                {
                    if(iscloseLeftCam)
                    {
                        m_leftCam->release();
                        break;
                    }
                }
                else
                {
                    if(isCaliCam)
                    {
                        m_leftCam->operator >>(oriLeftFrame);
                        if(!m_local_algro->m_LeftCaliPrePoc_1(oriLeftFrame,leftcalibretecamnum,20))
                        {
                            cv::Mat templeftframe;
                            cv::resize(oriLeftFrame,templeftframe,cv::Size(320,240));
                            new_send_leftCaliImageDisp(templeftframe);
                        }
                        else
                        {
                            isCaliCam = false;
                            if(m_local_algro->m_CalibrateCamera(true,false,false))
                            {
                                send_isdownCalibration(true);
                            }
                        }
                    }
                    else
                    {
                        m_leftCam->operator >>(oriLeftFrame);
                        if(m_local_algro->leftcameraMatrix.at<int>(0,0)==0)
                        {
                            cv::Mat templeftframe;
                            cv::resize(oriLeftFrame,templeftframe,cv::Size(320,240));
                            new_send_leftCaliImageDisp(templeftframe);
                        }
                        else
                        {
                            cv::Mat correctImage;
                            undistort(oriLeftFrame,correctImage,m_local_algro->leftcameraMatrix,m_local_algro->leftdistCoeffs);
                            cv::Mat templeftframe;
                            cv::resize(correctImage,templeftframe,cv::Size(320,240));
                            new_send_leftCaliImageDisp(templeftframe);
                        }
                    }
                }
            }
            break;
        case RIGHT_CAMERA:
            m_rightCam    = new cv::VideoCapture(m_rightCamDev->returnCamdevIndex());
            while(1)
            {
                if(isOpenRightCam)
                {
                    if(iscloseRightCam)
                    {
                        m_rightCam->release();
                        break;
                    }
                }
                else
                {
                    if(isCaliCam)
                    {
                        m_rightCam->operator >>(oriRightFrame);
                        if(!m_local_algro->m_RightCaliPrePoc_1(oriRightFrame,rightcalibretecamnum,20))
                        {
                            cv::Mat temprightframe;
                            cv::resize(oriRightFrame,temprightframe,cv::Size(320,240));
                            new_send_rightCaliImageDisp(temprightframe);
                        }
                        else
                        {
                            isCaliCam = false;
                            if(m_local_algro->m_CalibrateCamera(false,true,false))
                            {
                                send_isdownCalibration(true);
                            }
                        }
                    }
                    else
                    {
                        m_rightCam->operator >>(oriRightFrame);
                        if(m_local_algro->rightcameraMatrix.at<int>(0,0)==0)
                        {
                            cv::Mat temprightframe;
                            cv::resize(oriRightFrame,temprightframe,cv::Size(320,240));
                            new_send_rightCaliImageDisp(temprightframe);
                        }
                        else
                        {
                            cv::Mat correctImage;
                            undistort(oriRightFrame,correctImage,m_local_algro->rightcameraMatrix,m_local_algro->rightdistCoeffs);
                            cv::Mat temprightframe;
                            cv::resize(correctImage,temprightframe,cv::Size(320,240));
                            new_send_rightCaliImageDisp(temprightframe);
                        }
                    }
                }
            }
            break;
        case ALL_CAMERA:
            m_leftCam  = new cv::VideoCapture(m_leftCamDev->returnCamdevIndex());
            m_rightCam = new cv::VideoCapture(m_rightCamDev->returnCamdevIndex());
            while (oriLeftFrame.rows < 2) {
                m_leftCam->open(0);
                m_leftCam->set(CAP_PROP_FOURCC, 'GPJM');
                m_leftCam->set(CAP_PROP_FRAME_WIDTH, 320);
                m_leftCam->set(CAP_PROP_FRAME_HEIGHT, 240);
                cont = 0;
                while (oriLeftFrame.rows < 2 && cont<5) {
                    m_leftCam->operator >>(oriLeftFrame);
                    cont++;
                }
            }
            while (oriRightFrame.rows < 2) {
                m_rightCam->open(1);
                m_rightCam->set(CAP_PROP_FOURCC, 'GPJM');
                m_rightCam->set(CAP_PROP_FRAME_WIDTH, 320);
                m_rightCam->set(CAP_PROP_FRAME_HEIGHT, 240);
                cont = 0;
                while (oriRightFrame.rows < 2 && cont<5) {
                    m_rightCam->operator >>(oriRightFrame);
                    cont++;
                }
            }

            while(1)
            {
                if(isOpenAllCam)
                {

                    if(iscloseAllCam)
                    {
                        m_leftCam->release();
                        m_rightCam->release();
                        break;
                    }
                }
                else
                {
                    if(isCaliCam)
                    {
                        m_leftCam->operator >>(oriLeftFrame);
                        m_rightCam->operator >>(oriRightFrame);
                        if(!m_local_algro->m_LeftCaliPrePoc_1(oriLeftFrame,leftcalibretecamnum,20)&&!m_local_algro->m_RightCaliPrePoc_1(oriRightFrame,rightcalibretecamnum,20))
                        {
                            cv::Mat templeftframe;
                            cv::resize(oriLeftFrame,templeftframe,cv::Size(320,240));
                            cv::Mat temprightframe;
                            cv::resize(oriRightFrame,temprightframe,cv::Size(320,240));
                            new_send_allCaliImageDisp(templeftframe,temprightframe);
                        }
                        else
                        {
                            isCaliCam = false;
                            if(m_local_algro->m_CalibrateCamera(true,true,true))
                            {
                                send_isdownCalibration(true);
                            }
                        }
                    }
                    {
                        m_leftCam->operator >>(oriLeftFrame);
                        m_rightCam->operator >>(oriRightFrame);
                        if(m_local_algro->leftcameraMatrix.at<int>(0,0) ==0&&m_local_algro->rightcameraMatrix.at<int>(0,0) ==0)
                        {
                            cv::Mat templeftframe;
                            cv::resize(oriLeftFrame,templeftframe,cv::Size(320,240));
                            cv::Mat temprightframe;
                            cv::resize(oriRightFrame,temprightframe,cv::Size(320,240));
                            new_send_allCaliImageDisp(templeftframe,temprightframe);
                        }
                        else
                        {
                            cv::Mat leftcorrectImage;
                            undistort(oriLeftFrame,leftcorrectImage,m_local_algro->leftcameraMatrix,m_local_algro->leftdistCoeffs);
                            cv::Mat templeftframe;
                            cv::resize(leftcorrectImage,templeftframe,cv::Size(320,240));
                            cv::Mat rightcorrectImage;
                            undistort(oriRightFrame,rightcorrectImage,m_local_algro->rightcameraMatrix,m_local_algro->rightdistCoeffs);
                            cv::Mat temprightframe;
                            cv::resize(rightcorrectImage,temprightframe,cv::Size(320,240));
                            new_send_allCaliImageDisp(templeftframe,temprightframe);
                        }
                    }
                }
            }
            break;
        default:
            break;
        }
    }
        break;
    }
}
