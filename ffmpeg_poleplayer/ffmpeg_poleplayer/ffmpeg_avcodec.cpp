#include "ffmpeg_avcodec.h"
int refresh_video(void **opaque) {
	//SDL_mutex *lock = SDL_CreateMutex();

	AVState **in_avstate = (AVState **)(opaque);
	//SDL_mutexP(lock);
	int delay = 40;
	//SDL_mutexV(lock);
	thread_exit = 0;
	while (!thread_exit) {
		SDL_Event event;
		event.type = REFRESH_EVENT;
		SDL_PushEvent(&event); //发送事件 即handler
		if (*in_avstate == nullptr)
			break;
		delay = (int)(av_q2d((*in_avstate)->pVideoStream->r_frame_rate)*(*in_avstate)->speedratio);

		SDL_Delay(delay);
	}
	thread_exit = 0;
	//Break
	SDL_Event event;//代表一个事件 声明
	event.type = BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}
void WaitKeyboard(void **opaque)
{
	AVState **in_avstate = (AVState **)(opaque);
	int quit = 0;
	SDL_Event event;

	while (quit != 1)
	{
		//        printf ( "waiting for keyboard action\n" );
		//        while( SDL_PollEvent(&event)){
		SDL_WaitEvent(&event);

		switch (event.type)
		{
		case SDL_KEYDOWN:
			printf("key aa!\n");
			if (event.key.keysym.sym == SDLK_SPACE) //
			{
				printf("key good:%d\n", (*in_avstate)->playstate);
				if ((*in_avstate)->playstate >= 0)
				{
					if ((*in_avstate)->command == 1)
						(*in_avstate)->command = 0;
					else
						(*in_avstate)->command = 1;
				}
			}
			if (event.key.keysym.sym == SDLK_DOWN) // KP_ENTER
			{
				if ((*in_avstate)->playstate >= 0)
				{
					(*in_avstate)->command = 1;
				}
			}
			if (event.key.keysym.sym == SDLK_ESCAPE) //如果按的是ESC键  
			{
				quit = 1;                       //退出循环  
			}
			break;
		case SDL_QUIT:

			quit = 1;
			break;
		}
	}
}
void closefmt_ctx(AVFormatContext *ifmt_ctx, AVFormatContext *audio_ofmt_ctx, AVFormatContext *video_ofmt_ctx, int ret)
{
	avformat_close_input(&ifmt_ctx);
	/* close output */
	if (audio_ofmt_ctx && !(audio_ofmt_ctx->oformat->flags & AVFMT_NOFILE))
		avio_close(audio_ofmt_ctx->pb);
	avformat_free_context(audio_ofmt_ctx);
	if (video_ofmt_ctx && !(video_ofmt_ctx->oformat->flags & AVFMT_NOFILE))
		avio_close(video_ofmt_ctx->pb);
	avformat_free_context(video_ofmt_ctx);
	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred.\n");
		return;
	}
}
void  fill_audio(void *udata, Uint8 *stream, int len)
{
	//SDL 2.0  
	SDL_memset(stream, 0, len);
	if (audio_len == 0)
		return;
	len = (len>audio_len ? audio_len : len);   /*  Mix  as  much  data  as  possible  */

	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}
int initAVState(AVState **in_avstate, char *filename)
{
	AVFormatContext *ifmt_ctx;

	AVCodecContext *audio_decodec_ctx, *video_decodec_ctx;
	AVCodec *audio_decodec, *video_decodec;

	int ret, i;
	int videoindex;
	int audioindex;

	char *in_filename = filename;
	*in_avstate = (AVState *)malloc(sizeof(AVState));
	(*in_avstate)->FileName = filename;
	(*in_avstate)->pFormatCtx = nullptr;
	(*in_avstate)->pVideoCtx = nullptr;
	(*in_avstate)->pVideo = nullptr;
	(*in_avstate)->pAudioCtx = nullptr;
	(*in_avstate)->pAudio = nullptr;
	(*in_avstate)->VideoIndex = -1;
	(*in_avstate)->AudioIndex = -1;
	(*in_avstate)->pAudioStream = nullptr;
	(*in_avstate)->pAudioStream = nullptr;
	(*in_avstate)->video_topvec = nullptr;
	(*in_avstate)->video_endvec = nullptr;
	(*in_avstate)->video_currentvec = nullptr;
	(*in_avstate)->audio_topvec = nullptr;
	(*in_avstate)->audio_endvec = nullptr;
	(*in_avstate)->audio_currentvec = nullptr;
	(*in_avstate)->audiopts = 0.0;
	(*in_avstate)->videopts = 0.0;
	(*in_avstate)->speedratio = 1.0;
	(*in_avstate)->playstate = -1;
	(*in_avstate)->command = 0;
	(*in_avstate)->liveshow = 0;
	(*in_avstate)->audio_filter_ctx = nullptr;
	(*in_avstate)->video_filter_ctx = nullptr;



	audio_decodec_ctx = nullptr;
	video_decodec_ctx = nullptr;

	audio_decodec = nullptr;
	video_decodec = nullptr;
	ifmt_ctx = nullptr;

	videoindex = -1;
	audioindex = -1;
	(*in_avstate)->audio_filter_ctx = (FilteringContext *)av_malloc_array(1, sizeof(FilteringContext));



	ifmt_ctx = avformat_alloc_context();
	ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0);
	ret = avformat_find_stream_info(ifmt_ctx, 0);

	audioindex = -1;
	for (i = 0; i < ifmt_ctx->nb_streams; i++)
		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioindex = i;
			break;
		}

	if (audioindex == -1)
	{
		printf("Didn't find a audio stream.\n");

	}

	videoindex = -1;
	for (i = 0; i < ifmt_ctx->nb_streams; i++)
		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	if (videoindex == -1)
	{
		printf("Didn't find a video stream.\n");
	}

	if ((audioindex == -1) && (videoindex == -1))
	{
		printf("Didn't find a stream.\n");
		return -1;
	}

	av_dump_format(ifmt_ctx, 0, in_filename, 0);

	(*in_avstate)->pFormatCtx = ifmt_ctx;
	(*in_avstate)->VideoIndex = videoindex;
	(*in_avstate)->AudioIndex = audioindex;


	if (audioindex != -1)
	{
		if (ifmt_ctx->streams[audioindex]->codecpar->codec_id == AV_CODEC_ID_AAC)
		{
			audio_decodec = avcodec_find_decoder_by_name("libfdk_aac");
		}
		else
		{
			audio_decodec = avcodec_find_decoder(ifmt_ctx->streams[audioindex]->codecpar->codec_id);
		}
		audio_decodec_ctx = avcodec_alloc_context3(audio_decodec);//需要使用avcodec_free_context释放														  //事实上codecpar包含了大部分解码器相关的信息，这里是直接从AVCodecParameters复制到AVCodecContext
		avcodec_parameters_to_context(audio_decodec_ctx, ifmt_ctx->streams[audioindex]->codecpar);

		if (audio_decodec == NULL) {
			printf("Codec not found.\n");
			avformat_close_input(&ifmt_ctx);
			return -1;
		}
		if (avcodec_open2(audio_decodec_ctx, audio_decodec, NULL) < 0)
		{
			printf("Could not open codec.\n");
			avformat_close_input(&ifmt_ctx);
			return -1;
		}

		if (ifmt_ctx->streams[audioindex]->r_frame_rate.num != 0)
		{
			audio_decodec_ctx->time_base = { ifmt_ctx->streams[audioindex]->r_frame_rate.den,ifmt_ctx->streams[audioindex]->r_frame_rate.num };
			audio_decodec_ctx->framerate = { ifmt_ctx->streams[audioindex]->r_frame_rate.num,ifmt_ctx->streams[audioindex]->r_frame_rate.den };
		}
		else
		{
			ifmt_ctx->streams[audioindex]->r_frame_rate = { audio_decodec_ctx->time_base.den,audio_decodec_ctx->time_base.num };
			audio_decodec_ctx->framerate = { ifmt_ctx->streams[audioindex]->r_frame_rate.den,ifmt_ctx->streams[audioindex]->r_frame_rate.num };
		}
		avcodec_parameters_from_context(ifmt_ctx->streams[audioindex]->codecpar, audio_decodec_ctx);
		(*in_avstate)->pAudioCtx = audio_decodec_ctx;
		(*in_avstate)->pAudio = audio_decodec;
		(*in_avstate)->pAudioStream = ifmt_ctx->streams[audioindex];
		(*in_avstate)->audio_filter_ctx = (FilteringContext *)av_malloc_array(1, sizeof(FilteringContext));
		audio_initfilter((*in_avstate)->audio_filter_ctx, (*in_avstate)->pAudioCtx, (*in_avstate)->pAudioCtx);

	}

	if (videoindex != -1)
	{

		video_decodec = avcodec_find_decoder(ifmt_ctx->streams[videoindex]->codecpar->codec_id);
		video_decodec_ctx = avcodec_alloc_context3(video_decodec);//需要使用avcodec_free_context释放														  //事实上codecpar包含了大部分解码器相关的信息，这里是直接从AVCodecParameters复制到AVCodecContext
		avcodec_parameters_to_context(video_decodec_ctx, ifmt_ctx->streams[videoindex]->codecpar);
		if (video_decodec == NULL)
		{
			printf("Codec not found.\n");
			avformat_close_input(&ifmt_ctx);
			return -1;
		}
		if (avcodec_open2(video_decodec_ctx, video_decodec, NULL) < 0)
		{
			printf("Could not open codec.\n");
			avformat_close_input(&ifmt_ctx);
			return -1;
		}

		if (ifmt_ctx->streams[videoindex]->r_frame_rate.num != 0)
		{
			video_decodec_ctx->time_base = { ifmt_ctx->streams[videoindex]->r_frame_rate.den,ifmt_ctx->streams[videoindex]->r_frame_rate.num };
			video_decodec_ctx->framerate = { ifmt_ctx->streams[videoindex]->r_frame_rate.num,ifmt_ctx->streams[videoindex]->r_frame_rate.den };
		}
		else
		{
			ifmt_ctx->streams[videoindex]->r_frame_rate = { video_decodec_ctx->time_base.den,video_decodec_ctx->time_base.num };
			video_decodec_ctx->framerate = { ifmt_ctx->streams[videoindex]->r_frame_rate.den,ifmt_ctx->streams[audioindex]->r_frame_rate.num };
		}
		avcodec_parameters_from_context(ifmt_ctx->streams[videoindex]->codecpar, video_decodec_ctx);
		(*in_avstate)->pVideoCtx = video_decodec_ctx;
		(*in_avstate)->pVideo = video_decodec;
		(*in_avstate)->pVideoStream = ifmt_ctx->streams[videoindex];
		(*in_avstate)->video_filter_ctx = (FilteringContext *)av_malloc_array(1, sizeof(FilteringContext));
		video_initfilter((*in_avstate)->video_filter_ctx, (*in_avstate)->pVideoCtx, (*in_avstate)->pVideoCtx);
	}
	
	
	return 1;
}
void pushpacket(AVState **in_avstate, AVPacket *packet)
{
	if (packet->stream_index == (*in_avstate)->VideoIndex)
	{
		if ((*in_avstate)->video_endvec == nullptr)
		{

			(*in_avstate)->video_endvec = pushtailNode((*in_avstate)->video_endvec, (void*)packet);
			(*in_avstate)->video_topvec = (*in_avstate)->video_endvec;
			(*in_avstate)->video_currentvec = (*in_avstate)->video_topvec;
		}
		else
		{
			(*in_avstate)->video_endvec = pushtailNode((*in_avstate)->video_endvec, (void*)packet);
		}
	}
	if (packet->stream_index == (*in_avstate)->AudioIndex)
	{

		if ((*in_avstate)->audio_endvec == nullptr)
		{
			(*in_avstate)->audio_endvec = pushtailNode((*in_avstate)->audio_endvec, (void*)packet);
			(*in_avstate)->audio_topvec = (*in_avstate)->audio_endvec;
			(*in_avstate)->audio_currentvec = (*in_avstate)->audio_topvec;
		}
		else
		{
			(*in_avstate)->audio_endvec = pushtailNode((*in_avstate)->audio_endvec, (void*)packet);
		}
	}
}

void poppacket(AVState **in_avstate, int stream_index, AVPacket **packet)
{
	if (stream_index == (*in_avstate)->VideoIndex)
	{
		if ((*in_avstate)->video_topvec != nullptr)
			(*in_avstate)->video_topvec = popheadNode((*in_avstate)->video_topvec, (void**)packet);
		if ((*in_avstate)->video_topvec == nullptr)
		{
			(*in_avstate)->video_endvec = nullptr;
			return;
		}
	}
	if (stream_index == (*in_avstate)->AudioIndex)
	{
		if ((*in_avstate)->audio_topvec != nullptr)
			(*in_avstate)->audio_topvec = popheadNode((*in_avstate)->audio_topvec, (void**)packet);
		if ((*in_avstate)->audio_topvec == nullptr)
		{
			(*in_avstate)->audio_endvec = nullptr;
			return;
		}
	}
}

void getcurrentpacket(AVState **in_avstate, int stream_index, AVPacket **packet)
{
	AVPacket *tmppacket = nullptr;
	if (stream_index == (*in_avstate)->VideoIndex)
	{
		if ((*in_avstate)->video_currentvec != nullptr)
		{
			getcurrentdata((*in_avstate)->video_currentvec, (void**)&tmppacket);
			*packet = av_packet_clone(tmppacket);
		}
		if ((*in_avstate)->video_currentvec == nullptr)
		{
			*packet = nullptr;
		}
	}
	if (stream_index == (*in_avstate)->AudioIndex)
	{
		if ((*in_avstate)->audio_currentvec != nullptr)
		{
			getcurrentdata((*in_avstate)->audio_currentvec, (void**)&tmppacket);
			*packet = av_packet_clone(tmppacket);
		}
		if ((*in_avstate)->audio_currentvec == nullptr)
		{
			*packet = nullptr;
		}
	}
}
void findnextpacketpos(AVState **in_avstate, int stream_index)
{
	if (stream_index == (*in_avstate)->VideoIndex)
	{
		(*in_avstate)->video_currentvec = getnextnode((*in_avstate)->video_currentvec);
	}
	if (stream_index == (*in_avstate)->AudioIndex)
	{
		(*in_avstate)->audio_currentvec = getnextnode((*in_avstate)->audio_currentvec);
	}
}
void findforepacketpos(AVState **in_avstate, int stream_index)
{
	if (stream_index == (*in_avstate)->VideoIndex)
	{
		(*in_avstate)->video_currentvec = getforenode((*in_avstate)->video_currentvec);
	}
	if (stream_index == (*in_avstate)->AudioIndex)
	{
		(*in_avstate)->audio_currentvec = getforenode((*in_avstate)->audio_currentvec);
	}
}
void freeAVState(AVState **in_avstate)
{
	if (*in_avstate == nullptr)
		return;
	if ((*in_avstate)->pFormatCtx != nullptr)
		avformat_close_input(&((*in_avstate)->pFormatCtx));
	AVPacket *pop_packet = nullptr;
	while ((*in_avstate)->video_topvec != nullptr)
	{
		(*in_avstate)->video_topvec = popheadNode((*in_avstate)->video_topvec, (void**)&pop_packet);
		//AVPacket *show_pakcet = (AVPacket *)(*pop_packet);

		if (pop_packet != nullptr)
		{

			av_packet_free(&pop_packet);
			pop_packet = nullptr;
		}
	}
	(*in_avstate)->video_endvec = nullptr;
	(*in_avstate)->video_currentvec = nullptr;

	while ((*in_avstate)->audio_topvec != nullptr)
	{
		(*in_avstate)->audio_topvec = popheadNode((*in_avstate)->audio_topvec, (void**)&pop_packet);
		//AVPacket *show_pakcet = (AVPacket *)(*pop_packet);
		if (pop_packet != nullptr)
		{	
			av_packet_free(&pop_packet);
			pop_packet = nullptr;
		}

	}
	(*in_avstate)->audio_endvec = nullptr;
	(*in_avstate)->audio_currentvec = nullptr;
	if ((*in_avstate)->audio_filter_ctx!=nullptr)
	{
		av_freep((*in_avstate)->audio_filter_ctx);
		(*in_avstate)->audio_filter_ctx = nullptr;
	}
	if ((*in_avstate)->video_filter_ctx != nullptr)
	{
		av_freep((*in_avstate)->video_filter_ctx);
		(*in_avstate)->video_filter_ctx = nullptr;
	}
	free(*in_avstate);
	*in_avstate = nullptr;
	return;
}
void audio_decfun(void **param)
{
	//SDL_mutex *lock = SDL_CreateMutex();
	int ret = -1;
	AVState **in_avstate = (AVState **)(param);


	AVFrame *paudioFrame;
	SDL_AudioSpec wanted_spec;
	struct SwrContext *au_convert_ctx;
	int64_t in_channel_layout;
	uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;

	in_channel_layout = 0;
	au_convert_ctx = nullptr;
	//nb_samples: AAC-1024 MP3-1152  

	int out_nb_samples;
	if ((*in_avstate)->pAudioCtx->frame_size <= 0)
	{
		out_nb_samples = 1024;
		(*in_avstate)->pAudioCtx->frame_size = 1024;
	}
	else
		out_nb_samples = (*in_avstate)->pAudioCtx->frame_size;

	AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
	int out_sample_rate = (*in_avstate)->pAudioCtx->sample_rate;//44100;
	int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
	//Out Buffer Size  
	out_audiobuffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);

	out_audiobuffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
	paudioFrame = av_frame_alloc();

	//SDL_AudioSpec  
	wanted_spec.freq = out_sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = out_channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = out_nb_samples;
	wanted_spec.callback = fill_audio;
	wanted_spec.userdata = (*in_avstate)->pAudioCtx;



	if (SDL_OpenAudio(&wanted_spec, NULL)<0) {
		printf("can't open audio.\n");
		return;
	}
	in_channel_layout = av_get_default_channel_layout((*in_avstate)->pAudioCtx->channels);

	au_convert_ctx = swr_alloc();
	au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate,
		in_channel_layout, (*in_avstate)->pAudioCtx->sample_fmt, (*in_avstate)->pAudioCtx->sample_rate, 0, NULL);
	swr_init(au_convert_ctx);

	SDL_PauseAudio(0);
	AVPacket *audio_packet = nullptr;


	while (1)
	{
		if ((*in_avstate)->playstate == -1)
		{
			SDL_Delay(10);
			continue;
		}
		if ((*in_avstate)->command == 0)
		{
			SDL_Delay(20);
			continue;
		}
		

		if ((*in_avstate)->liveshow == 1)
		{
			poppacket(in_avstate, (*in_avstate)->AudioIndex, &audio_packet);
		}
		else
		{
			getcurrentpacket(in_avstate, (*in_avstate)->AudioIndex, &audio_packet);
			findnextpacketpos(in_avstate, (*in_avstate)->AudioIndex);
		}
		
		if (audio_packet == nullptr)
		{
			(*in_avstate)->playstate = (*in_avstate)->playstate + 1;
			break;
		}
		//	SDL_mutexP(lock);
		(*in_avstate)->audiopts = (double)audio_packet->pts*av_q2d((*in_avstate)->pAudioStream->time_base);
		//	SDL_mutexV(lock);


		ret = avcodec_send_packet((*in_avstate)->pAudioCtx, audio_packet);

		if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
			continue;
		ret = -1;
		
		ret = avcodec_receive_frame((*in_avstate)->pAudioCtx, paudioFrame);
		if (ret < 0)
		{
			continue;
		}
		
		AVFrame *tmpFrame = av_frame_clone(paudioFrame);
		filtframe(tmpFrame, paudioFrame, (*in_avstate)->audio_filter_ctx);
		if (tmpFrame != nullptr)
		{
			av_frame_free(&tmpFrame);
			tmpFrame = nullptr;
		}
		if (audio_packet != nullptr)
		{
			av_packet_free(&audio_packet);
			audio_packet = nullptr;
		}
		
		
		/*
		AVFrame *filtFrame = nullptr;
		filtFrame = av_frame_alloc();
		filtFrame->nb_samples = (*in_avstate)->pAudioCtx->frame_size;//av_audio_fifo_size(audiofifo);//audio_encodec_ctx->frame_size;
		filtFrame->channel_layout = (*in_avstate)->pAudioCtx->channel_layout;
		filtFrame->format = (*in_avstate)->pAudioCtx->sample_fmt;

		filtFrame->sample_rate = (*in_avstate)->pAudioCtx->sample_rate;

		av_frame_get_buffer(filtFrame, (*in_avstate)->pAudioCtx->frame_size);
		int convertsize = 0;
		convertsize = swr_convert(au_convert_ctx, filtFrame->data, (*in_avstate)->pAudioCtx->frame_size, (const uint8_t**)paudioFrame->data, paudioFrame->nb_samples);
		//printf("%d,%d,%d,%d\n", convertsize, (*in_avstate)->pAudioCtx->frame_size, filtFrame->nb_samples, paudioFrame->nb_samples);
		paudioFrame = av_frame_clone(filtFrame);
		filtframe(filtFrame, paudioFrame, (*in_avstate)->audio_filter_ctx);
		av_frame_free(&filtFrame);
		//out_audiobuffer = (Uint8 *)malloc(sizeof(Uint8));
		//memcpy((void*)out_audiobuffer, (void*)filtFrame->data, (*in_avstate)->pAudioCtx->frame_size);
		*/
		swr_convert(au_convert_ctx, &out_audiobuffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **)paudioFrame->data, paudioFrame->nb_samples);
		while (audio_len > 0)//Wait until finish  
		{
			SDL_Delay(1);
		}
		audio_chunk = (Uint8 *)out_audiobuffer;
		//audio_chunk = (Uint8 *)(paudioFrame->data);
		audio_len = out_audiobuffer_size;
		audio_pos = audio_chunk;;
	}


}
void video_decfun(void **param)
{
	//	SDL_mutex *lock = SDL_CreateMutex();
	int ret = -1;
	AVState **in_avstate = (AVState **)(param);
	SDL_Window *screen;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
	AVFrame *pvideoFrame, *pvideoFrameYUV, *pvideoFrameYUV_hc;
	unsigned char *out_videobuffer, *out_videobuffer_hc;
	struct SwsContext *img_convert_ctx, *img_convert_ctx_hc;

	sdlTexture = nullptr;
	sdlRenderer = nullptr;
	screen = nullptr;

	pvideoFrame = nullptr;
	pvideoFrameYUV = nullptr;
	pvideoFrameYUV_hc = nullptr;
	out_videobuffer = nullptr;
	out_videobuffer_hc = nullptr;
	img_convert_ctx = nullptr;
	img_convert_ctx_hc = nullptr;


	pvideoFrame = av_frame_alloc();
	pvideoFrameYUV = av_frame_alloc();

	pvideoFrameYUV_hc = av_frame_alloc();


	out_videobuffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
		(*in_avstate)->pVideoCtx->width, (*in_avstate)->pVideoCtx->height, 1));
	av_image_fill_arrays(pvideoFrameYUV->data, pvideoFrameYUV->linesize, out_videobuffer,
		AV_PIX_FMT_YUV420P, (*in_avstate)->pVideoCtx->width, (*in_avstate)->pVideoCtx->height, 1);

	out_videobuffer_hc = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
		(*in_avstate)->pVideoCtx->width, (*in_avstate)->pVideoCtx->height, 1));
	av_image_fill_arrays(pvideoFrameYUV_hc->data, pvideoFrameYUV_hc->linesize, out_videobuffer_hc,
		AV_PIX_FMT_YUV420P, (*in_avstate)->pVideoCtx->width, (*in_avstate)->pVideoCtx->height, 1);

	printf("-------------------------------------------------\n");
	int k = 0;
	while ((k++ < 5) && ((img_convert_ctx == NULL) || (img_convert_ctx_hc == NULL)))
	{

		img_convert_ctx = sws_getContext((*in_avstate)->pVideoCtx->width, (*in_avstate)->pVideoCtx->height, (*in_avstate)->pVideoCtx->pix_fmt,
			(*in_avstate)->pVideoCtx->width, (*in_avstate)->pVideoCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
		img_convert_ctx_hc = sws_getContext((*in_avstate)->pVideoCtx->width, (*in_avstate)->pVideoCtx->height, AV_PIX_FMT_YUV420P,
			(*in_avstate)->pVideoCtx->width, (*in_avstate)->pVideoCtx->height, AV_PIX_FMT_YUV420P,
			SWS_BICUBIC,
			NULL, NULL, NULL);
	}
	if ((img_convert_ctx == NULL) || (img_convert_ctx_hc == NULL))
	{
		printf("格式不对\n");
		return;
	}

	//SDL 2.0 Support for multiple windows  
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		(*in_avstate)->pVideoCtx->width, (*in_avstate)->pVideoCtx->height,
		SDL_WINDOW_OPENGL);

	if (!screen) {
		printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return;
	}

	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	//IYUV: Y + U + V  (3 planes)  
	//YV12: Y + V + U  (3 planes)  
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, (*in_avstate)->pVideoCtx->width, (*in_avstate)->pVideoCtx->height);

	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = (*in_avstate)->pVideoCtx->width;;
	sdlRect.h = (*in_avstate)->pVideoCtx->height;
	AVPacket *video_packet = nullptr;
	SDL_Event event;
	while (1)
	{
		if ((*in_avstate)->playstate == -1)
		{
			SDL_Delay(10);
			continue;
		}
		if ((*in_avstate)->command == 0)
		{
			SDL_Delay(20);
			continue;
		}
		if ((*in_avstate)->liveshow == 1)
		{
			poppacket(in_avstate, (*in_avstate)->VideoIndex, &video_packet);
		}
		else
		{
			getcurrentpacket(in_avstate, (*in_avstate)->VideoIndex, &video_packet);
			findnextpacketpos(in_avstate, (*in_avstate)->VideoIndex);
		}
		if (video_packet == nullptr)
		{
			(*in_avstate)->playstate = (*in_avstate)->playstate + 1;
			break;
		}

		
		//	SDL_mutexP(lock);

		(*in_avstate)->videopts = (double)video_packet->pts*av_q2d((*in_avstate)->pVideoStream->time_base);
		//	SDL_mutexV(lock);

		while ((*in_avstate)->videopts > (*in_avstate)->audiopts)
		{
			if ((*in_avstate)->playstate >= 1)
				break;
			SDL_Delay(5);
		}
		if ((*in_avstate)->videopts < (*in_avstate)->audiopts)
		{
			(*in_avstate)->speedratio = 0.1;

		}
		else
		{
			(*in_avstate)->speedratio = 1.0;
		}

		ret = avcodec_send_packet((*in_avstate)->pVideoCtx, video_packet);
		if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
		{
			printf("Decode Error00.\n");
			continue;
			//return -1;
		}

		ret = -1;
		ret = avcodec_receive_frame((*in_avstate)->pVideoCtx, pvideoFrame);
		if (ret < 0) {
			printf("Decode Error.\n");
			if (pvideoFrame == nullptr)
			{
				printf("question!\n");
			}
			continue;
		}

		
		AVFrame *tmpFrame = av_frame_clone(pvideoFrame);
		tmpFrame->width = (*in_avstate)->pVideoCtx->width;
		tmpFrame->height = (*in_avstate)->pVideoCtx->height;

		filtframe(tmpFrame, pvideoFrame, (*in_avstate)->video_filter_ctx);

		if (tmpFrame != nullptr)
		{
			av_frame_free(&tmpFrame);
			tmpFrame = nullptr;
		}

		if (video_packet != nullptr)
		{
			av_packet_free(&video_packet);
			video_packet = nullptr;
		}
		//printf("test0:%d,%d\n", pvideoCodecCtx->width, pvideoCodecCtx->height);
		//printf("test1:%d,%d,%d,%d\n", pvideoFrame->linesize, pvideoCodecCtx->height, pvideoFrameYUV->linesize, pvideoFrameYUV->height);
		sws_scale(img_convert_ctx, (const unsigned char* const*)pvideoFrame->data, pvideoFrame->linesize, 0, (*in_avstate)->pVideoCtx->height,
			pvideoFrameYUV->data, pvideoFrameYUV->linesize);
		sws_scale(img_convert_ctx_hc, (const unsigned char* const*)pvideoFrameYUV->data, pvideoFrameYUV->linesize, 0, (*in_avstate)->pVideoCtx->height,
			pvideoFrameYUV_hc->data, pvideoFrameYUV_hc->linesize);



		SDL_WaitEvent(&event);
		if (event.type == REFRESH_EVENT)
		{

			SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
				pvideoFrameYUV_hc->data[0], pvideoFrameYUV_hc->linesize[0],
				pvideoFrameYUV_hc->data[1], pvideoFrameYUV_hc->linesize[1],
				pvideoFrameYUV_hc->data[2], pvideoFrameYUV_hc->linesize[2]);
			SDL_RenderClear(sdlRenderer);
			SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
			SDL_RenderPresent(sdlRenderer);

			//SDL_Delay(40);

		}
		else if (event.type == SDL_WINDOWEVENT)
		{
			//If Resize SDL_WINDOWEVENT事件 可以拉伸播放器界面
			SDL_GetWindowSize(screen, &((*in_avstate)->pVideoCtx->width), &((*in_avstate)->pVideoCtx->height));
		}
		else if (event.type == SDL_QUIT)
		{
			thread_exit = 1;
		}
		else if (event.type == BREAK_EVENT)
		{
			break;
		}

	}

}



/*AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, */
int audio_initfilter(FilteringContext *audio_filter_ctx, AVCodecContext *audio_decodec_ctx, AVCodecContext *audio_encodec_ctx)
{
	const char*filter_spec = "anull";

	audio_filter_ctx->buffersrc_ctx = NULL;
	audio_filter_ctx->buffersink_ctx = NULL;
	audio_filter_ctx->filter_graph = NULL;
	char args[512];
	int ret = 0;
	AVFilter*buffersrc = NULL;
	AVFilter*buffersink = NULL;
	AVFilterContext*buffersrc_ctx = NULL;
	AVFilterContext*buffersink_ctx = NULL;
	AVFilterInOut*outputs = avfilter_inout_alloc();
	AVFilterInOut*inputs = avfilter_inout_alloc();
	AVFilterGraph*filter_graph = avfilter_graph_alloc();

	buffersrc = avfilter_get_by_name("abuffer");
	buffersink = avfilter_get_by_name("abuffersink");
	_snprintf_s(args, sizeof(args),
		"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%I64x",
		audio_decodec_ctx->time_base.num, audio_decodec_ctx->time_base.den, audio_decodec_ctx->sample_rate,
		av_get_sample_fmt_name(audio_decodec_ctx->sample_fmt),
		audio_decodec_ctx->channel_layout);

	ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, NULL, filter_graph);
	ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, NULL, filter_graph);
	ret = av_opt_set_bin(buffersink_ctx, "sample_fmts", (uint8_t*)&audio_encodec_ctx->sample_fmt, sizeof(audio_encodec_ctx->sample_fmt), AV_OPT_SEARCH_CHILDREN);
	ret = av_opt_set_bin(buffersink_ctx, "channel_layouts", (uint8_t*)&audio_encodec_ctx->channel_layout, sizeof(audio_encodec_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
	ret = av_opt_set_bin(buffersink_ctx, "sample_rates", (uint8_t*)&audio_encodec_ctx->sample_rate, sizeof(audio_encodec_ctx->sample_rate), AV_OPT_SEARCH_CHILDREN);
	outputs->name = av_strdup("in");
	outputs->filter_ctx = buffersrc_ctx;
	outputs->pad_idx = 0;
	outputs->next = NULL;
	inputs->name = av_strdup("out");
	inputs->filter_ctx = buffersink_ctx;
	inputs->pad_idx = 0;
	inputs->next = NULL;
	ret = avfilter_graph_parse_ptr(filter_graph, filter_spec, &inputs, &outputs, NULL);
	ret = avfilter_graph_config(filter_graph, NULL);

	/* Fill FilteringContext */
	audio_filter_ctx->buffersrc_ctx = buffersrc_ctx;
	audio_filter_ctx->buffersink_ctx = buffersink_ctx;
	audio_filter_ctx->filter_graph = filter_graph;

	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);
	return 0;
}
/*AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, */
int video_initfilter(FilteringContext *video_filter_ctx, AVCodecContext *video_decodec_ctx, AVCodecContext *video_encodec_ctx)
{
	const char*filter_spec = "null";

	video_filter_ctx->buffersrc_ctx = NULL;
	video_filter_ctx->buffersink_ctx = NULL;
	video_filter_ctx->filter_graph = NULL;
	char args[512];
	int ret = 0;
	AVFilter*buffersrc = NULL;
	AVFilter*buffersink = NULL;
	AVFilterContext*buffersrc_ctx = NULL;
	AVFilterContext*buffersink_ctx = NULL;
	AVFilterInOut*outputs = avfilter_inout_alloc();
	AVFilterInOut*inputs = avfilter_inout_alloc();
	AVFilterGraph*filter_graph = avfilter_graph_alloc();

	buffersrc = avfilter_get_by_name("buffer");
	buffersink = avfilter_get_by_name("buffersink");
	_snprintf_s(args, sizeof(args),
		//snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		video_decodec_ctx->width, video_decodec_ctx->height, video_decodec_ctx->pix_fmt,
		video_decodec_ctx->time_base.num, video_decodec_ctx->time_base.den,
		video_decodec_ctx->sample_aspect_ratio.num,
		video_decodec_ctx->sample_aspect_ratio.den);

	//printf("%d,%d\n", video_decodec_ctx->time_base.num, video_decodec_ctx->time_base.den);

	ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, NULL, filter_graph);
	ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, NULL, filter_graph);
	ret = av_opt_set_bin(buffersink_ctx, "pix_fmts", (uint8_t*)&video_encodec_ctx->pix_fmt, sizeof(video_encodec_ctx->pix_fmt), AV_OPT_SEARCH_CHILDREN);
	outputs->name = av_strdup("in");
	outputs->filter_ctx = buffersrc_ctx;
	outputs->pad_idx = 0;
	outputs->next = NULL;
	inputs->name = av_strdup("out");
	inputs->filter_ctx = buffersink_ctx;
	inputs->pad_idx = 0;
	inputs->next = NULL;
	ret = avfilter_graph_parse_ptr(filter_graph, filter_spec, &inputs, &outputs, NULL);
	ret = avfilter_graph_config(filter_graph, NULL);

	/* Fill FilteringContext */
	video_filter_ctx->buffersrc_ctx = buffersrc_ctx;
	video_filter_ctx->buffersink_ctx = buffersink_ctx;
	video_filter_ctx->filter_graph = filter_graph;

	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);
	return 0;
}

int filtframe(AVFrame *frame, AVFrame *filt_frame, FilteringContext *fc_filter_ctx)
{
	int ret = 0;
	frame->pts = av_frame_get_best_effort_timestamp(frame);

	ret = av_buffersrc_add_frame_flags(fc_filter_ctx->buffersrc_ctx, frame, 0);
	if (ret < 0)
	{
		printf("av_buffersrc_add_frame_flags!!!!\n");
		return ret;
	}

	ret = av_buffersink_get_frame(fc_filter_ctx->buffersink_ctx, filt_frame);
	if (ret < 0)
	{
		printf("av_buffersink_get_frame!!!!\n");
		return ret;
	}

	filt_frame->pict_type = AV_PICTURE_TYPE_NONE;
	return ret;
}


int init_resampler(AVCodecContext *input_codec_context,
	AVCodecContext *output_codec_context,
	SwrContext **resample_context) {
	int error;

	*resample_context = swr_alloc_set_opts(NULL,
		av_get_default_channel_layout(output_codec_context->channels), output_codec_context->sample_fmt, output_codec_context->sample_rate,
		av_get_default_channel_layout(input_codec_context->channels), input_codec_context->sample_fmt, input_codec_context->sample_rate,
		0, NULL);
	if (!*resample_context) {
		fprintf(stderr, "Could not allocate resample context\n");
		return AVERROR(ENOMEM);
	}

	if ((error = swr_init(*resample_context)) < 0) {
		fprintf(stderr, "Could not open resample context\n");
		swr_free(resample_context);
		return error;
	}
	return 0;
}