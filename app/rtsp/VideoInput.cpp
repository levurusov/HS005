/*
 * Ingenic IMP RTSPServer VideoInput
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define  QCAM_AV_C_API
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_utils.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>

#include "qcam.h"
#include "qcam_audio_input.h"
#include "qcam_audio_output.h"
#include "qcam_log.h"
#include "qcam_motion_detect.h"
#include "qcam_sys.h"
#include "qcam_video_input.h"

#include <fstream>

#include "bitmapinfo.h"
#include "logodata_100x100_bgra.h"

#include "Options.hh"
#include "VideoInput.hh"
#include "H264VideoStreamSource.hh"

#define TAG 						"sample-RTSPServer"

#define IVS_MOVE 	0x00
#define IVS_FACE 	0x01
#define IVS_FIGURE  0x02

static int ivs_flag = 0;
//extern void 
/* Encoder param */
/* chn0:main-0 h264, chn1:second h264, chn2:main-0 jpeg, chn3:main-1 h264 */

Boolean VideoInput::fHaveInitialized = False;

LNode gLinkListHead;
int start_flag = 0;
static pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;


#if 0	 
int ivs_face_start(int grp_num, int chn_num, IMPIVSInterface **interface)
{
	int ret = 0;
	face_param_input_t param;

	memset(&param, 0, sizeof(face_param_input_t));
	param.frameInfo.width = 640;
	param.frameInfo.height = 360;
	*interface = FaceInterfaceInit(&param);
	if (*interface == NULL) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}

	ret = IMP_IVS_CreateChn(chn_num, *interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_RegisterChn(grp_num, chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
		return -1;
	}

	ret = IMP_IVS_StartRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
		return -1;
	}

	return 0;
}

int ivs_figure_start(int grp_num, int chn_num, IMPIVSInterface **interface)
{
	int ret = 0;
	figure_param_input_t param;

	memset(&param, 0, sizeof(figure_param_input_t));
	param.frameInfo.width = 640;
	param.frameInfo.height = 360;
	*interface = FigureInterfaceInit(&param);
	if (*interface == NULL) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}

	ret = IMP_IVS_CreateChn(chn_num, *interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_RegisterChn(grp_num, chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
		return -1;
	}

	ret = IMP_IVS_StartRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
		return -1;
	}

	return 0;
}

int ivs_move_start(int grp_num, int chn_num, IMPIVSInterface **interface)
{
	int ret = 0;
	move_param_input_t param;

	memset(&param, 0, sizeof(move_param_input_t));
	param.sense = 4;
	param.frameInfo.width = 640;
	param.frameInfo.height = 360;
	*interface = MoveInterfaceInit(&param);
	if (*interface == NULL) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}

	ret = IMP_IVS_CreateChn(chn_num, *interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_RegisterChn(grp_num, chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
		return -1;
	}

	ret = IMP_IVS_StartRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
		return -1;
	}

	return 0;
}

int ivs_face_process()
{
	int ret,i;
	face_param_output_t *result = NULL;
	
	ret = IMP_IVS_PollingResult(0, 2000);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", 0, 2000);
		return -1;
	}
	ret = IMP_IVS_GetResult(0, (void **)&result);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", 0);
		return -1;
	}
	if(result->count)
		IMP_LOG_INFO(TAG, "frame[%d], result->ret=%d\n", i, result->count);
	
	ret = IMP_IVS_ReleaseResult(0, (void *)result);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 0);
		return -1;
	}
	return 0;
}

int ivs_figure_process()
{
	int ret,i,m;
	figure_param_output_t *result = NULL;
	int x0 = 0, x1 = 0, y0 = 0, y1 = 0;
	
	ret = IMP_IVS_PollingResult(0, 2000);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", 0, 2000);
		return -1;
	}
	ret = IMP_IVS_GetResult(0, (void **)&result);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", 0);
		return -1;
	}
	if(result->count > 0) {
		for (m = 0; m < result->count; m++) {
			x0 = result->rects[m].ul.x;
			y0 = result->rects[m].ul.y;
			x1 = result->rects[m].br.x;
			y1 = result->rects[m].br.y;
			//IMP_LOG_DBG(TAG, "(%d,%d,%d,%d)\n", x0, y0, x1, y1);
			printf("rect[%d], (%d,%d,%d,%d)\n", m, x0, y0, x1, y1);
		}
	}
	//IMP_LOG_INFO(TAG, "frame[%d], result->ret=%d\n", i, result->count);

	ret = IMP_IVS_ReleaseResult(0, (void *)result);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 0);
		return -1;
	}
	return 0;
}

int ivs_move_process()
{
	int ret,i;
	move_param_output_t *result = NULL;
	
	ret = IMP_IVS_PollingResult(0, 2000);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", 0, 2000);
		return -1;
	}
	ret = IMP_IVS_GetResult(0, (void **)&result);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", 0);
		return -1;
	}

    if(result->ret > 0)
    {
        int i;
        for(i = 0 ;i < result->count;i++){
            IMP_LOG_INFO(TAG,"Rect%d(%d,%d) -- (%d,%d)\n",i,result->rects[i].ul.x,result->rects[i].ul.y,result->rects[i].br.x,result->rects[i].br.y);
        }

    }
	ret = IMP_IVS_ReleaseResult(0, (void *)result);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 0);
		return -1;
	}
	return 0;
}
#endif


VideoInput* VideoInput::createNew(UsageEnvironment& env, int streamNum) {
    if (!fHaveInitialized) {
		if (!initialize(env)) return NULL;
		fHaveInitialized = True;
    }

	VideoInput *videoInput = new VideoInput(env, streamNum);

    return videoInput;
}
void osd_on()
{
//	set_osd_rgn(0,"logo.bmp",0,100);
	set_osd_rect(0);
}


VideoInput::VideoInput(UsageEnvironment& env, int streamNum)
	: Medium(env), fVideoSource(NULL), fpsIsStart(False), fontIsStart(False),
	osdIsStart(False), osdStartCnt(0), nrFrmFps(0),
	totalLenFps(0), startTimeFps(0), streamNum(streamNum), scheduleTid(-1),
	ivsTid(-1), orgfrmRate(gconf_FPS_Num), hasSkipFrame(false) {
//	memset(ivsIsStart, 0, sizeof(ivsIsStart));

}

VideoInput::~VideoInput() {	
    /*step 4, stop stream and uninit */
    QCamVideoInput_Uninit();
}



void ysx_video_cb(int channel, const struct timeval *tv, const void *data, const int len, const int keyframe)
{
	if(start_flag == 0)
		return;
	

	int link_length = GetLength(&gLinkListHead);
	if (link_length > MAX_VID_BUF) {
		/*drop the frame */
		fprintf(stderr, "Now video length is %d, we will drop this frame\n", link_length);
		return ;
	}

	pthread_mutex_lock(&list_lock);
	AVPacket *pkt = (AVPacket *)malloc(sizeof(AVPacket));
	pkt->data = (uint8_t *)malloc(len);
	pkt->size = len;
	memcpy(pkt->data,data,len);

	InsertNode(&gLinkListHead,1,pkt);
	pthread_mutex_unlock(&list_lock);
	
}

void *save_vid_stream(void *arg)
{
	FILE *fp ;
	int link_length = -1;
	LNode *list;

	fp = fopen("stream.h264","w");
	if(!fp)
	{
		printf("open file stream.h264 error !\n");
		return 0;
	}

	list = &gLinkListHead;
	while(1)
	{

		link_length = GetLength(list);
		if(link_length)
		{
		
			pthread_mutex_lock(&list_lock);
			AVPacket *pkt = GetNodePkt(list, link_length);
			printf("%s:link_length = %d , getNode:size = %d\n",__FUNCTION__,link_length,pkt->size);	
			free(pkt->data);
			free(pkt);
			DeleteNode(list, link_length);
			pthread_mutex_unlock(&list_lock);			
		}
		else
			usleep(500*1000);

	}
	fclose(fp);

}

bool VideoInput::initialize(UsageEnvironment& env) 
{
	int ret;
	printf("@@@@@@@@@ VERSION: %s\n", __TIME__);
	ret = InitList(&gLinkListHead);
    if(ret == false){
        printf("InitList ERROR !\n");
		return false;
    }

    QCamVideoInputChannel ysx_chn;

    ysx_chn.bitrate 		= 1024;
	ysx_chn.cb				= ysx_video_cb;
	ysx_chn.channelId 		= 0;
	ysx_chn.fps 			= 15;
    ysx_chn.gop 			= 1;
//	ysx_chn.res 			= QCAM_VIDEO_RES_1080P;
//	ysx_chn.res 			= QCAM_VIDEO_RES_720P;
	ysx_chn.res 			= QCAM_VIDEO_RES_500W;
//	ysx_chn.res				= QCAM_VIDEO_RES_360P;
	ysx_chn.payloadType 	= 0;
	ysx_chn.vbr 			= 1;      /*choose CBR mode*/


    ret = QCamVideoInput_Init();
    if(ret < 0){
        printf("QCamVideoInput_Init ERROR !\n");
		return false;
    }

    ret = QCamVideoInput_AddChannel(ysx_chn);
    if(ret < 0){
        printf("QCamVideoInput_AddChannel ERROR !\n");
		return false;
    }
    

	ret = QCamVideoInput_Start();
	if(ret < 0){
		printf("QCamVideoInput_Start ERROR !\n");
		return -1;
	}


	/*step 4 ,add osd */
	QCamVideoInputOSD osd_attr;
	memset(&osd_attr,0,sizeof(QCamVideoInputOSD));
	osd_attr.pic_enable = 0;
	osd_attr.time_enable = 1;
	
	if(resolution == QCAM_VIDEO_RES_1080P)
	{
		osd_attr.time_x = 1464; //1920-19*24 = 1464
		osd_attr.time_y = 0;
	}else{
		osd_attr.time_x = 976; //1280-19*16 = 976
		osd_attr.time_y = 0;
	}
	//ret = QCamVideoInput_SetOSD(CHN_MAIN,&osd_attr);
	if(ret < 0){
		printf("QCamVideoInput_Start ERROR !\n");
		return -1;
	}

	QCamVideoInput_SetInversion(gconf_Inversion);
	
//	osd_on();
	QCamSetIRMode((QCAM_IR_MODE)IRMode);

	return true;
}

FramedSource* VideoInput::videoSource() {
	IMP_Encoder_FlushStream(streamNum);
	fVideoSource = new H264VideoStreamSource(envir(), *this);
	return fVideoSource;
}

int VideoInput::getStream(void* to, unsigned int* len, struct timeval* timestamp, unsigned fMaxSize) {
	int ret;
	int link_length = -1;
	LNode *list;
	int length = 0;
	
	list = &gLinkListHead;
	link_length = GetLength(list);
	if(link_length)
	{
	
		pthread_mutex_lock(&list_lock);
		AVPacket *pkt = GetNodePkt(list, link_length);
//		printf("%s:link_length = %d , getNode:size = %d\n",__FUNCTION__,link_length,pkt->size);	
		if(pkt->size > fMaxSize)
		{
			printf("drop frame:length=%d, fMaxSize=%d\n", pkt->size, fMaxSize);
			length = 0;			
		}else{
			memcpy(to,pkt->data,pkt->size);
			length = pkt->size;
		}
		*len = length;
//		printf("###RTSP VIDEO len = %d\n", *len);
		free(pkt->data);
		free(pkt);
		DeleteNode(list, link_length);		
		pthread_mutex_unlock(&list_lock);
	}
	
	gettimeofday(timestamp, NULL);
	return 0;
}

int VideoInput::pollingStream(void)
{
	int link_length = -1;
	LNode *list;

	list = &gLinkListHead;

	link_length = GetLength(list);
//	printf("poll stream lenght = %d\n",link_length);
	while(link_length == 0){
		usleep(100*1000);
		link_length = GetLength(list);
//		printf("poll stream lenght = %d\n",link_length);
	}

	return 0;
}
int VideoInput::streamOn(void)
{
	start_flag = 1;
	QCamVideoInput_SetIFrame(CHN_MAIN);
	return 0;
}

int VideoInput::streamOff(void)
{
	int ret;
	start_flag = 0;
	return 0;
}

