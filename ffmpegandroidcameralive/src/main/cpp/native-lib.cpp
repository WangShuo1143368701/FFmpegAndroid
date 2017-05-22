
#include <jni.h>
#include <string>
#include "native-lib.h";

extern "C"
{
#include <libswscale/swscale.h>
#include <libavutil/log.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>

#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

#include <android/log.h>
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "(>_<)ws-----------", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)ws-----------", format, ##__VA_ARGS__)

AVFormatContext *ofmt_ctx;
AVStream *video_st;
AVCodecContext *pCodecCtx;
AVCodec *pCodec;
AVPacket enc_pkt;
AVFrame *pFrameYUV;

int framecnt = 0;
int yuv_width;
int yuv_height;
int y_length;
int uv_length;
int64_t start_time;

//Output FFmpeg's av_log()
void custom_log(void *ptr, int level, const char *fmt, va_list vl) {
    FILE *fp = fopen("/storage/emulated/0/av_log.txt", "a+");
    if (fp) {
        vfprintf(fp, fmt, vl);
        fflush(fp);
        fclose(fp);
    }
}

JNIEXPORT jint JNICALL Java_com_ws_ffmpegandroidcameralive_WSPlayer_initialize
        (JNIEnv *env, jobject obj, jint width, jint height, jstring jurlpath) {
    const char *out_path = env->GetStringUTFChars(jurlpath, 0) ;
    //const char *out_path = "rtmp://192.168.9.135:1935/wstv/home";//ws
    yuv_width = width;
    yuv_height = height;
    y_length = width * height;
    uv_length = width * height / 4;

    //FFmpeg av_log() callback
    av_log_set_callback(custom_log);

    av_register_all();
    avformat_network_init();//ws add
    //output initialize
    avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", out_path);
    //output encoder initialize
    pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!pCodec) {
        LOGE("Can not find encoder!\n");
        return -1;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        LOGE("Could not allocate video codec context\n");
        return -1;
    }
    pCodecCtx->pix_fmt =  AV_PIX_FMT_YUV420P;//PIX_FMT_YUV420P新版加
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    pCodecCtx->bit_rate = 400000;
    pCodecCtx->gop_size = 250;
    /* Some formats want stream headers to be separate. */
    if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;

    //H264 codec param
    //pCodecCtx->me_range = 16;
    //pCodecCtx->max_qdiff = 4;
    //pCodecCtx->qcompress = 0.6;
    pCodecCtx->qmin = 10;
    pCodecCtx->qmax = 51;
    //Optional Param
    pCodecCtx->max_b_frames = 0;
    // Set H264 preset and tune
    AVDictionary *param = 0;
    //av_dict_set(&param, "preset", "ultrafast", 0);
    //av_dict_set(&param, "tune", "zerolatency", 0);
    av_opt_set(pCodecCtx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);

    if (avcodec_open2(pCodecCtx, pCodec, &param) < 0) {
        LOGE("Failed to open encoder!\n");
        return -1;
    }

    //Add a new stream to output,should be called by the user before avformat_write_header() for muxing
    video_st = avformat_new_stream(ofmt_ctx, pCodec);//avformat_new_stream创建流通道
    if (video_st == NULL) {
        return -1;
    }
    video_st->time_base.num = 1;
    video_st->time_base.den = 30;
    video_st->codec = pCodecCtx;

    //Open output URL,set before avformat_write_header() for muxing
    if (avio_open(&ofmt_ctx->pb, out_path, AVIO_FLAG_READ_WRITE) < 0) {
        LOGE("Failed to open output file!\n");
        return -1;
    }

    //Write File Header
    avformat_write_header(ofmt_ctx, NULL);

    start_time = av_gettime();
    return 0;
}


JNIEXPORT jint JNICALL Java_com_ws_ffmpegandroidcameralive_WSPlayer_start
        (JNIEnv *env, jobject obj, jbyteArray yuv) {
    int ret;
    int enc_got_frame = 0;
    int i = 0;

    pFrameYUV = av_frame_alloc();//旧版 avcodec_alloc_frame() //分配一个AVFrame结构体。
    uint8_t *out_buffer = (uint8_t *) av_malloc(
            avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    avpicture_fill((AVPicture *) pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width,
                   pCodecCtx->height);

    //安卓摄像头数据为NV21格式，此处将其转换为YUV420P格式
    jbyte *in = (jbyte *) env->GetByteArrayElements(yuv, 0);
    memcpy(pFrameYUV->data[0], in, y_length);
    for (i = 0; i < uv_length; i++) {
        *(pFrameYUV->data[2] + i) = *(in + y_length + i * 2);
        *(pFrameYUV->data[1] + i) = *(in + y_length + i * 2 + 1);
    }

    pFrameYUV->format = AV_PIX_FMT_YUV420P;
    pFrameYUV->width = yuv_width;
    pFrameYUV->height = yuv_height;

    enc_pkt.data = NULL;
    enc_pkt.size = 0;
    av_init_packet(&enc_pkt);//初始化 AVPacker
    ret = avcodec_encode_video2(pCodecCtx, &enc_pkt, pFrameYUV, &enc_got_frame);
    av_frame_free(&pFrameYUV);

    if (enc_got_frame == 1) {
        LOGI("Succeed to encode frame: %5d\tsize:%5d\n", framecnt, enc_pkt.size);
        framecnt++;
        enc_pkt.stream_index = video_st->index;

        //Write PTS
        AVRational time_base = ofmt_ctx->streams[0]->time_base;//{ 1, 1000 };
        AVRational r_framerate1 = {60, 2};//{ 50, 2 };
        AVRational time_base_q = {1, AV_TIME_BASE};
        //Duration between 2 frames (us)
        int64_t calc_duration = (double) (AV_TIME_BASE) * (1 / av_q2d(r_framerate1));    //内部时间戳
        //Parameters
        //enc_pkt.pts = (double)(framecnt*calc_duration)*(double)(av_q2d(time_base_q)) / (double)(av_q2d(time_base));
        enc_pkt.pts = av_rescale_q(framecnt * calc_duration, time_base_q, time_base);
        enc_pkt.dts = enc_pkt.pts;
        enc_pkt.duration = av_rescale_q(calc_duration, time_base_q,
                                        time_base); //(double)(calc_duration)*(double)(av_q2d(time_base_q)) / (double)(av_q2d(time_base));
        enc_pkt.pos = -1;

        //Delay
        int64_t pts_time = av_rescale_q(enc_pkt.dts, time_base, time_base_q);
        int64_t now_time = av_gettime() - start_time;
        if (pts_time > now_time)
            av_usleep(pts_time - now_time);

        ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);//将AVPacket（存储视频压缩码流数据）写入文件。
        av_free_packet(&enc_pkt);
    }

    return 0;
}

JNIEXPORT jint JNICALL Java_com_ws_ffmpegandroidcameralive_WSPlayer_stop
        (JNIEnv *env, jobject obj) {
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(ofmt_ctx->streams[0]->codec->codec->capabilities &
          CODEC_CAP_DELAY))
        return 0;
    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2(ofmt_ctx->streams[0]->codec, &enc_pkt,
                                    NULL, &got_frame);
        if (ret < 0)
            break;
        if (!got_frame) {
            ret = 0;
            break;
        }
        LOGI("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);

        //Write PTS
        AVRational time_base = ofmt_ctx->streams[0]->time_base;//{ 1, 1000 };
        AVRational r_framerate1 = {60, 2};
        AVRational time_base_q = {1, AV_TIME_BASE};
        //Duration between 2 frames (us)
        int64_t calc_duration = (double) (AV_TIME_BASE) * (1 / av_q2d(r_framerate1));    //内部时间戳
        //Parameters
        enc_pkt.pts = av_rescale_q(framecnt * calc_duration, time_base_q, time_base);
        enc_pkt.dts = enc_pkt.pts;
        enc_pkt.duration = av_rescale_q(calc_duration, time_base_q, time_base);

        //转换PTS/DTS（Convert PTS/DTS）
        enc_pkt.pos = -1;
        framecnt++;
        ofmt_ctx->duration = enc_pkt.duration * framecnt;

        /* mux encoded frame */
        ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    //Write file trailer
    av_write_trailer(ofmt_ctx);
    return 0;
}

JNIEXPORT jint JNICALL Java_com_ws_ffmpegandroidcameralive_WSPlayer_close
        (JNIEnv *env, jobject obj) {
    if (video_st)
        avcodec_close(video_st->codec);
    avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    return 0;
}





