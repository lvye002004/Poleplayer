#include<stdio.h>
#include <stdlib.h>
#include"vec.h"
#include "ffmpeg_avcodec.h"
#define __STDC_CONSTANT_MACROS  

int main(int argc, char* argv[])
{
	const char *in_filename;
	int ret;
	int frame_index;
	AVPacket *dec_packet;
	dec_packet = nullptr;
	frame_index = -1;
	//
	in_filename = "G:\\video\\dll\\6.rmvb";//
//F:\\xunleidown\\黑白照相馆HD高清国语中字[飘花www.piaohua.com].mkv";
	//F:\\xunleidown\\S破LT狼.HDTC720P清晰版【关注微信公众号：dycncn 高清更新抢先看】.mp4
//E:\\vswork\\testvideo\\44.flv//F:\\xunleidown\\lanjingling.rmvb//rtmp://live.hkstv.hk.lxdns.com/live/hks

	av_register_all();
	avfilter_register_all();
	avformat_network_init();
	avdevice_register_all();
	
	printf("start play!\n");

	AVState *in_pavstate = nullptr;
	initAVState(&in_pavstate,(char*)in_filename);
	in_pavstate->liveshow = 1;
	printf("%d,%d\n", in_pavstate->pVideoStream->r_frame_rate.num, in_pavstate->pVideoStream->r_frame_rate.den);
	
	printf("%s\n",in_pavstate->FileName);
	printf("%d,%d\n", in_pavstate->pVideoCtx->time_base.num, in_pavstate->pVideoCtx->time_base.den);
	
	frame_index = 0;
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	SDL_Thread *refresh_thread = SDL_CreateThread((SDL_ThreadFunction)refresh_video,"event", (void**)(&in_pavstate));//创建线程
	SDL_Thread *keyboard_thread = SDL_CreateThread((SDL_ThreadFunction)WaitKeyboard, "command", (void**)(&in_pavstate));//创建线程
	SDL_Thread *audiothread = SDL_CreateThread((SDL_ThreadFunction)audio_decfun,"audio",(void**)(&in_pavstate));
	SDL_Thread *videothread = SDL_CreateThread((SDL_ThreadFunction)video_decfun, "video", (void**)(&in_pavstate));
	dec_packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	av_init_packet(dec_packet);
	int count = 0;
	while (1)
	{
		ret = av_read_frame(in_pavstate->pFormatCtx, dec_packet);

		if (ret < 0)
			break;
		AVPacket *push_packet = av_packet_clone(dec_packet);
		pushpacket(&in_pavstate, push_packet);
		count++;
		if ((in_pavstate->playstate == -1) && (count > 10))
		{
			in_pavstate->playstate = 0;
			in_pavstate->command = 1;
		}

	}
	int door = 0;
	if (in_pavstate->AudioIndex != -1)
		door++;
	if (in_pavstate->VideoIndex != -1)
		door++;
	
	while(1)
	{
		if (in_pavstate->playstate >= door)
			break;
		SDL_Delay(30);
	}
	
	
	if(dec_packet!=nullptr)
		av_packet_free(&dec_packet);
	freeAVState(&in_pavstate);
	SDL_CloseAudio();//Close SDL  
	SDL_Quit();
	//avformat_close_input(&ifmt_ctx);
	//closefmt_ctx(ifmt_ctx, audio_ofmt_ctx, video_ofmt_ctx, ret);
	printf("end play!\n");

	return 0;
}

/*
while (1)
{
AVStream *in_stream, *out_stream;


ret = av_read_frame(ifmt_ctx, dec_packet);

if (ret < 0)
break;
AVPacket *push_packet = av_packet_clone(dec_packet);

if (dec_packet->stream_index == videoindex)
{
if (video_topvec == nullptr)
{

video_topvec = pushheadNode(video_topvec, (void*)push_packet);
video_endvec = video_topvec;
}
else
{
video_topvec = pushheadNode(video_topvec, (void*)push_packet);
}
}
if (dec_packet->stream_index == audioindex)
{

if (audio_topvec == nullptr)
{
audio_topvec = pushheadNode(audio_topvec, (void*)push_packet);
audio_endvec = audio_topvec;
}
else
{
audio_topvec = pushheadNode(audio_topvec, (void*)push_packet);
}
}

}
AVPacket *pop_packet = nullptr;
while (video_endvec!=nullptr)
{
video_endvec = poptailNode(video_endvec, (void**)&pop_packet);
//AVPacket *show_pakcet = (AVPacket *)(*pop_packet);

if (pop_packet != nullptr)
{
printf("%d\n", pop_packet->size);
av_packet_free(&pop_packet);
pop_packet = nullptr;
}


}
video_topvec = nullptr;
while (audio_endvec != nullptr)
{
audio_endvec = poptailNode(audio_endvec, (void**)&pop_packet);
//AVPacket *show_pakcet = (AVPacket *)(*pop_packet);
if (pop_packet != nullptr)
{
printf("%d\n", pop_packet->size);
av_packet_free(&pop_packet);
pop_packet = nullptr;
}

}
audio_topvec = nullptr;
*/
/*
int main(int argc, char *argv[])
{

	int i;
	VEC *topvec, *endvec,*selectvec;
	topvec = nullptr;
	endvec = nullptr;
	AVPacket *packet = nullptr;
	for (i = 0;i < 10;i++)
	{
		packet = (AVPacket *)av_malloc(sizeof(AVPacket));
		av_init_packet(packet);
		packet->size = i;
		topvec = pushheadNode(topvec, (void*)packet);
		if (i == 0)
			endvec = topvec;
	}
	AVPacket *node_packet = nullptr;
	
	while (endvec != nullptr)
	{
		endvec = poptailNode(endvec, (void**)&node_packet);
		printf("%d\n", ((AVPacket*)node_packet)->size);
		av_packet_free((void*)node_packet);
		node_packet = nullptr;
	}
	topvec = nullptr;
	//emptyNode(topvec, endvec, (void**)&node_packet);
	


	system("pause");
	return 1;
}
*/

/*
int *data = nullptr;
for (i = 0;i < 10;i++)
{
data = (int*)malloc(sizeof(int));
data[0] = i;
topvec = pushheadNode(topvec, (void*)data);
if (i == 0)
endvec = topvec;

}
selectvec = topvec;
int *gdata = nullptr;;



while (endvec != nullptr)
{
endvec = poptailNode(endvec,(void**)&gdata);
printf("%d\n", ((int*)gdata)[0]);
free(gdata);
gdata = nullptr;
}

emptyNode(topvec,endvec,(void**)&gdata);
topvec = nullptr;
*/