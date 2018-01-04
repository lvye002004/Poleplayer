//#pragma once
#ifndef FFMPEG_AVCODEC_H
#define FFMPEG_AVCODEC_H


#include<stdio.h>
#include <stdlib.h>
#include"vec.h"
#define __STDC_CONSTANT_MACROS  


extern "C"
{
#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "libavutil/imgutils.h"  
#include "libswresample/swresample.h"  
#include "libavdevice/avdevice.h"
#include <libavutil/mathematics.h>  
#include <libavutil/time.h>  

#include "libavfilter/avfiltergraph.h"  
#include "libavfilter/buffersink.h"  
#include "libavfilter/buffersrc.h"  
#include "libavutil/avutil.h"  
#include "libavutil/opt.h"  
#include "libavutil/pixdesc.h"

#include "libavutil/audio_fifo.h"
#include "libavutil/avstring.h"
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>

#include "SDL2/SDL.h"  
	//#include "SDL2/SDL_mutex.h"
#undef main
};
//Output YUV420P data as a file   
#define OUTPUT_YUV420P 0  
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio 
typedef struct FilteringContext {
	AVFilterContext*buffersink_ctx;
	AVFilterContext*buffersrc_ctx;
	AVFilterGraph*filter_graph;
} FilteringContext;
typedef struct AVState
{
	AVFormatContext *pFormatCtx;
	AVCodecContext  *pVideoCtx;
	AVCodecContext  *pAudioCtx;
	AVCodec         *pVideo;
	int              VideoIndex;
	AVCodec         *pAudio;
	int              AudioIndex;
	AVStream        *pVideoStream;
	AVStream        *pAudioStream;

	FilteringContext *audio_filter_ctx;
	FilteringContext *video_filter_ctx;
	VEC *audio_topvec;
	VEC *audio_endvec;
	VEC *audio_currentvec;
	VEC *video_topvec;
	VEC *video_endvec;
	VEC *video_currentvec;
	char *FileName;
	double audiopts;
	double videopts;
	double speedratio;
	int command;
	int playstate;
	int liveshow;
}AVState;
static  Uint8  *audio_chunk;
static  Uint32  audio_len;
static  Uint8  *audio_pos;
static uint8_t  *out_audiobuffer = nullptr;
static int out_audiobuffer_size = 0;
static int thread_exit = 0;


//static FilteringContext *filter_ctx;

#define REFRESH_EVENT  (SDL_USEREVENT + 1)

#define BREAK_EVENT  (SDL_USEREVENT + 2)
void video_decfun(void **param);
void audio_decfun(void **param);
void freeAVState(AVState **in_avstate);
void poppacket(AVState **in_avstate, int stream_index, AVPacket **packet);
void getcurrentpacket(AVState **in_avstate, int stream_index, AVPacket **packet);
void findnextpacketpos(AVState **in_avstate, int stream_index);
void findforepacketpos(AVState **in_avstate, int stream_index);
void pushpacket(AVState **in_avstate, AVPacket *packet);
int initAVState(AVState **in_avstate, char *filename);
void  fill_audio(void *udata, Uint8 *stream, int len);
void WaitKeyboard(void **opaque);
int refresh_video(void **opaque);

int audio_initfilter(FilteringContext *audio_filter_ctx, AVCodecContext *audio_decodec_ctx, AVCodecContext *audio_encodec_ctx);
int video_initfilter(FilteringContext *video_filter_ctx, AVCodecContext *video_decodec_ctx, AVCodecContext *video_encodec_ctx);
int filtframe(AVFrame *frame, AVFrame *filt_frame, FilteringContext *fc_filter_ctx);
void closefmt_ctx(AVFormatContext *ifmt_ctx, AVFormatContext *audio_ofmt_ctx, AVFormatContext *video_ofmt_ctx, int ret);
#endif