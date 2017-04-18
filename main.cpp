#include <stdio.h>
#include <iostream>
#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "SDL/SDL.h"
};
#else
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <SDL/SDL.h>
#ifdef __cplusplus
};
#endif
#endif

#include "PrintAvDetails.h"

//Full Screen  
#define SHOW_FULLSCREEN 0
//Output YUV420P   
#define OUTPUT_YUV420P 0


bool avDecode(AVCodecContext *pCodecCtx,
              AVFrame * pFrame,
              AVFrame * pFrameYUV,
              SDL_Overlay *bmp,
              SwsContext *swsCtx,
              AVPacket *packet,
              SDL_Rect &rect)
{
    // Decode
    if(0 != avcodec_send_packet(pCodecCtx, packet))
    {
        return false;
    }
    if(0 != avcodec_receive_frame(pCodecCtx, pFrame))
    {
        return false;
    }

    SDL_LockYUVOverlay(bmp);
    pFrameYUV->data[0] = bmp->pixels[0];
    pFrameYUV->data[1] = bmp->pixels[2];
    pFrameYUV->data[2] = bmp->pixels[1];
    pFrameYUV->linesize[0] = bmp->pitches[0];
    pFrameYUV->linesize[1] = bmp->pitches[2];
    pFrameYUV->linesize[2] = bmp->pitches[1];

    sws_scale(swsCtx,
              (const uint8_t* const*)pFrame->data,
              pFrame->linesize,
              0,
              pCodecCtx->height,
              pFrameYUV->data,
              pFrameYUV->linesize);

#if OUTPUT_YUV420P
    int y_size=pCodecCtx->width*pCodecCtx->height;
            fwrite(pFrameYUV->data[0],1,y_size, fpYuv);    //Y
            fwrite(pFrameYUV->data[1],1,y_size/4, fpYuv);  //U
            fwrite(pFrameYUV->data[2],1,y_size/4, fpYuv);  //V
#endif
    SDL_UnlockYUVOverlay(bmp);
    SDL_DisplayYUVOverlay(bmp, &rect);
    //Delay 40ms
    SDL_Delay(40);
    return true;
}

int main(int argc, char* argv[])
{
    const char *filepath = "../test.h265";
    // 初始化
    av_register_all();
    avformat_network_init();

    // avformat环境
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    if(avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
        printf("Couldn't open input stream.\n");
        return -1;
    }
    if(avformat_find_stream_info(pFormatCtx, NULL)<0) {
        printf("Couldn't find stream information.\n");
        return -1;
    }
    int videoIndex = -1;
    for(int i = 0; i < pFormatCtx->nb_streams; ++i) {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            break;
        }
    }
    if(videoIndex == -1) {
        printf("Didn't find a video stream.\n");
        return -1;
    }

    // avcodec环境
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
    if (pCodecCtx == NULL)
    {
        printf("Could not allocate AVCodecContext\n");
        return -1;
    }
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoIndex]->codecpar);

    // 获取avcodec[解码器]
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        printf("Codec not found.\n");
        return -1;
    }
    // 打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        printf("Could not open codec.\n");
        return -1;
    }

    // 帧
    AVFrame * pFrame = av_frame_alloc();
    AVFrame * pFrameYUV = av_frame_alloc();

    // SDL Start ----------------------------
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf( "Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

#if SHOW_FULLSCREEN
    const SDL_VideoInfo *vi = SDL_GetVideoInfo();
    int screen_w = vi->current_w;
    int screen_h = vi->current_h;
    SDL_Surface *screen = SDL_SetVideoMode(screen_w, screen_h, 0,SDL_FULLSCREEN);
#else
    int screen_w = pCodecCtx->width;
    int screen_h = pCodecCtx->height;
    SDL_Surface *screen = SDL_SetVideoMode(screen_w, screen_h, 0,0);
#endif

    if(!screen) {
        printf("SDL: could not set video mode - exiting:%s\n",SDL_GetError());
        return -1;
    }

    SDL_Overlay *bmp = SDL_CreateYUVOverlay(pCodecCtx->width,
                                            pCodecCtx->height,
                                            SDL_YV12_OVERLAY,
                                            screen);

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = screen_w;
    rect.h = screen_h;
    // SDL End------------------------

    //Output Information-----------------------------
    printf("------------- File Information ------------------\n");
    av_dump_format(pFormatCtx, 0, filepath, 0);
    PrintAvDetails(pFormatCtx, pCodecCtx, pCodec, videoIndex);
    printf("-------------------------------------------------\n");

#if OUTPUT_YUV420P
    FILE *fpYuv = fopen("output.yuv","wb+");
#endif

    SDL_WM_SetCaption("Simplest FFmpeg Player", NULL);
    SwsContext *swsCtx = sws_getContext(pCodecCtx->width,
                                        pCodecCtx->height,
                                        pCodecCtx->pix_fmt,
                                        pCodecCtx->width,
                                        pCodecCtx->height,
                                        AV_PIX_FMT_YUV420P,
                                        SWS_BICUBIC,
                                        NULL,
                                        NULL,
                                        NULL);
    //------------------------------

    // decode
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    while(0 == av_read_frame(pFormatCtx, packet))
    {
        if(packet->stream_index == videoIndex)
        {
            if(!avDecode(pCodecCtx,
                         pFrame,
                         pFrameYUV,
                         bmp,
                         swsCtx,
                         packet,
                         rect))
            {
                printf("avdecode error\n");
                break;
            }
        }
        av_packet_unref(packet);
    }

    //FIX: Flush Frames remained in Codec  
    while (1)
    {
        if(!avDecode(pCodecCtx,
                     pFrame,
                     pFrameYUV,
                     bmp,
                     swsCtx,
                     packet,
                     rect))
        {
            break;
        }
    }

    // release
#if OUTPUT_YUV420P
    fclose(fpYuv);
#endif

    sws_freeContext(swsCtx);
    SDL_Quit();
    av_free(pFrameYUV);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}  