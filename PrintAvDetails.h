//
// Created by shaoqi on 17-4-18.
//

#ifndef SIMPLEST_FFMPEG_PLAYER_PRINTAVDETAILS_H
#define SIMPLEST_FFMPEG_PLAYER_PRINTAVDETAILS_H

#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <cstdio>

void PrintAvDetails(AVFormatContext *pFormatCtx,
                    AVCodecContext *pCodecCtx,
                    AVCodec *pCodec,
                    int videoIndex,
                    int audioIndex = -1)
{
    int iHour, iMinute, iSecond, iTotalSeconds;//HH:MM:SS
    AVDictionaryEntry *dict = NULL;

    puts("AVFormatContext信息：");
    puts("---------------------------------------------");
    printf("文件名：%s\n", pFormatCtx->filename);
    iTotalSeconds = (int) pFormatCtx->duration/*微秒*/ / 1000000;
    iHour = iTotalSeconds / 3600;//小时
    iMinute = iTotalSeconds % 3600 / 60;//分钟
    iSecond = iTotalSeconds % 60;//秒
    printf("持续时间：%02d:%02d:%02d\n", iHour, iMinute, iSecond);
    printf("平均混合码率：%ld kb/s\n", pFormatCtx->bit_rate / 1000);
    printf("视音频个数：%d\n", pFormatCtx->nb_streams);
    puts("---------------------------------------------");

    puts("AVInputFormat信息:");
    puts("---------------------------------------------");
    printf("封装格式名称：%s\n", pFormatCtx->iformat->name);
    printf("封装格式长名称：%s\n", pFormatCtx->iformat->long_name);
    printf("封装格式扩展名：%s\n", pFormatCtx->iformat->extensions);
    printf("封装格式ID：%d\n", pFormatCtx->iformat->raw_codec_id);
    puts("---------------------------------------------");

    puts("AVStream信息:");
    puts("---------------------------------------------");
    if( videoIndex >=0 ) {
        printf("视频流标识符：%d\n", pFormatCtx->streams[videoIndex]->index);
        printf("视频流长度：%ld微秒\n", pFormatCtx->streams[videoIndex]->duration);
    }

    if( audioIndex >=0 ) {
        printf("音频流标识符：%d\n", pFormatCtx->streams[audioIndex]->index);
        printf("音频流长度：%ld微秒\n", pFormatCtx->streams[audioIndex]->duration);
    }
    puts("---------------------------------------------");
    puts("AVCodecContext信息:");
    puts("---------------------------------------------");
    printf("视频码率：%ld kb/s\n", pCodecCtx->bit_rate / 1000);
    printf("视频大小：%d * %d\n", pCodecCtx->width, pCodecCtx->height);
    puts("---------------------------------------------");

    puts("AVCodec信息:");
    puts("---------------------------------------------");
    printf("视频编码格式：%s\n", pCodec->name);
    printf("视频编码详细格式：%s\n", pCodec->long_name);
    puts("---------------------------------------------");
    if( videoIndex >=0 )
        printf("视频时长：%ld微秒\n", pFormatCtx->streams[videoIndex]->duration);
    if( audioIndex >=0 )
    {
        printf("音频时长：%ld微秒\n", pFormatCtx->streams[audioIndex]->duration);
        printf("音频采样率：%d\n", pFormatCtx->streams[audioIndex]->codecpar->sample_rate);
        printf("音频信道数目：%d\n", pFormatCtx->streams[audioIndex]->codecpar->channels);
    }

    puts("AVFormatContext元数据：");
    puts("---------------------------------------------");
    while (dict = av_dict_get(pFormatCtx->metadata, "", dict, AV_DICT_IGNORE_SUFFIX))
    {
        printf("[%s] = %s\n", dict->key, dict->value);
    }
    puts("---------------------------------------------");

    puts("AVStream视频元数据：");
    puts("---------------------------------------------");
    dict = NULL;
    if( videoIndex >=0 ) {
        while (dict = av_dict_get(pFormatCtx->streams[videoIndex]->metadata, "", dict, AV_DICT_IGNORE_SUFFIX))
        {
            printf("[%s] = %s\n", dict->key, dict->value);
        }
    }
    puts("---------------------------------------------");

    puts("AVStream音频元数据：");
    puts("---------------------------------------------");
    dict = NULL;
    if( audioIndex >=0 ) {
        while (dict = av_dict_get(pFormatCtx->streams[audioIndex]->metadata, "", dict, AV_DICT_IGNORE_SUFFIX))
        {
            printf("[%s] = %s\n", dict->key, dict->value);
        }
    }
    puts("---------------------------------------------");
}


#endif //SIMPLEST_FFMPEG_PLAYER_PRINTAVDETAILS_H
