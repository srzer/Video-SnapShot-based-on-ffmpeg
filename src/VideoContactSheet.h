#pragma once
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<string>
#include<iostream>
using namespace std;
extern "C" {
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/pixdesc.h>
}
class VideoContactSheet{
  private:
    string filename;
    AVFormatContext *fmt_ctx=NULL;
	  AVStream *stream=NULL;
    AVCodecContext *dec_ctx=NULL;
    AVFrame *pAVFrame=NULL;
    AVFrame *FrameRGB=NULL;
    AVFrame *Frame_comp=NULL;
    AVRational timebase;
    AVPacket *pkt=NULL;
    int vidx;
    int imgtag;
    int taskid;
    int _timestamp[10];
    void open_codec();
    void add_text(uint8_t* odata,int timestamp,int width,int height);
    int decode_video(AVFrame* frame,int64_t timestamp);
    void frame_extraction();
    void resize_and_output();
    void clear();
    void endwith(string end_message);
    string getfilename(string imgname);
    void convert_and_output(AVFrame *frame);
    void convert_and_compose(AVFrame *frame);
  public:
    VideoContactSheet(string filename);
    void work(int _taskid);
};