#include"VideoContactSheet.h"
#include"DegitPixel.h"
#include<stack>
extern "C"{
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../external/stb/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../external/stb/stb_image_resize.h"
}
using namespace std;
#define AV_TIME_BASE 1000000
#define NEXT_PACKET 114514
#define FOUND_FRAME 1919810
void VideoContactSheet::clear(){
  av_packet_free(&pkt);
	avcodec_free_context(&dec_ctx);
	avformat_close_input(&fmt_ctx);
	avformat_free_context(fmt_ctx);
  av_frame_free(&pAVFrame);
  av_frame_free(&FrameRGB);
  av_frame_free(&Frame_comp);
}
void VideoContactSheet::endwith(string end_message){
  cout<<end_message<<endl;
  clear();
  exit(0);
}
string VideoContactSheet::getfilename(string imgname){
  int len=filename.size();
  for(int i=len;i>=1;i--)
    if(filename[i-1]=='\\'||filename[i-1]=='/'){
      string res=filename.substr(0,i)+imgname;
      return res;
    }
  return imgname;
}
void VideoContactSheet::convert_and_output(AVFrame *frame){
  string temp_num="0";temp_num[0]+=imgtag;
  string temp="frame_"+temp_num+".png";
  string res=getfilename(temp);
  int width=frame->width,height=frame->height;
  
  SwsContext* sws_ctx=sws_getContext(width,height,AVPixelFormat(frame->format),width,height,AV_PIX_FMT_RGB24,SWS_BICUBIC,NULL,NULL,NULL);
  sws_scale(sws_ctx,(uint8_t const* const*)frame->data,frame->linesize,0,frame->height,FrameRGB->data,FrameRGB->linesize);
  sws_freeContext(sws_ctx);
  stbi_write_png(res.c_str(),width,height,3,FrameRGB->data[0],0);
}
void loadtext(stack<int>&temp,int content){
  if(content<10){
    temp.push(content);
    temp.push(0);
  }
  else while(content>0){
    temp.push(content%10);
    content/=10;
  }
}
void VideoContactSheet::add_text(uint8_t* odata,int timestamp,int width,int height){
  int hour=timestamp/3600,minute=timestamp/60%60,sec=timestamp%60;
  stack<int>temp;while(!temp.empty())temp.pop();
  loadtext(temp,sec);temp.push(10);loadtext(temp,minute);temp.push(10);loadtext(temp,hour);
  int cnt=0;
  while(!temp.empty()){
    for(int i=0;i<20;i++){
      int row=((imgtag-1)/3+1)*height-20+i,col=(imgtag-1)%3;
      memcpy(odata+row*width*3+col*width+20*3*cnt,degit[temp.top()]+i*20*3,20*3);
    }
    temp.pop();
    cnt++;
  }
};
void VideoContactSheet::convert_and_compose(AVFrame *frame){
  int width=frame->width,height=frame->height;
  SwsContext* sws_ctx=sws_getContext(width,height,AVPixelFormat(frame->format),width,height,AV_PIX_FMT_RGB24,SWS_BICUBIC,NULL,NULL,NULL);
  sws_scale(sws_ctx,(uint8_t const* const*)frame->data,frame->linesize,0,frame->height,FrameRGB->data,FrameRGB->linesize);
  sws_freeContext(sws_ctx);
  width*=3;
  for(int i=0;i<height;i++){
    int row=(imgtag-1)/3*height+i,col=(imgtag-1)%3;
    memcpy(Frame_comp->data[0]+row*width*3+col*width,FrameRGB->data[0]+i*width,width);
  }
  _timestamp[imgtag]=av_q2d(timebase)*(double)frame->best_effort_timestamp;
}
void VideoContactSheet::open_codec(){
  AVCodec *dec=avcodec_find_decoder(stream->codecpar->codec_id);
  if(!dec)endwith("warning: no codec");
  dec_ctx=avcodec_alloc_context3(dec);
  int ret=avcodec_parameters_to_context(dec_ctx,stream->codecpar);
  if(ret<0)delete dec,endwith("warning: fail to set decoder parameter");
  AVDictionary *opts=NULL;
  ret=avcodec_open2(dec_ctx,dec,&opts);
  delete opts;
  if(ret<0)delete dec,endwith("warning: fail to open codec");
}
VideoContactSheet::VideoContactSheet(string _filename){
  filename=_filename;
  fmt_ctx=avformat_alloc_context();
  pkt=av_packet_alloc();
  pAVFrame=av_frame_alloc();
  FrameRGB=av_frame_alloc();
  Frame_comp=av_frame_alloc();
	int ret=avformat_open_input(&fmt_ctx,filename.c_str(),NULL,NULL);
  if(ret<0)endwith("warning: no file");
  vidx=-1;
  for(int i=0;i<fmt_ctx->nb_streams;++i)
    if(fmt_ctx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){vidx=i;break;}
  if(vidx==-1)endwith("warning: no video stream");
	stream=fmt_ctx->streams[vidx];
  timebase=stream->time_base;
  open_codec();
  int numBytes=avpicture_get_size(AV_PIX_FMT_RGB24,dec_ctx->width,dec_ctx->height);
  uint8_t *out_buffer_rgb=(uint8_t*)av_malloc(numBytes*sizeof(uint8_t));
  avpicture_fill((AVPicture*)FrameRGB,out_buffer_rgb,AV_PIX_FMT_RGB24,dec_ctx->width,dec_ctx->height);
  numBytes=avpicture_get_size(AV_PIX_FMT_RGB24,dec_ctx->width*3,dec_ctx->height*2);
  uint8_t *out_buffer_comp=(uint8_t*)av_malloc(numBytes*sizeof(uint8_t));
  avpicture_fill((AVPicture*)Frame_comp,out_buffer_comp,AV_PIX_FMT_RGB24,dec_ctx->width*3,dec_ctx->height*2);
}
int VideoContactSheet::decode_video(AVFrame* frame,int64_t timestamp){
  int ret=avcodec_send_packet(dec_ctx,pkt);
  if(ret<0)endwith("warning: fail to send packet");
  while(1){
    if((ret=avcodec_receive_frame(dec_ctx,frame))<0){
      if(ret==AVERROR_EOF||ret==AVERROR(EAGAIN))return NEXT_PACKET;
      return ret;
    }
    if(frame->best_effort_timestamp<timestamp){
      av_frame_unref(frame);
      return NEXT_PACKET;
    }
    
    if(frame->format==AV_PIX_FMT_YUV420P||frame->format==AV_PIX_FMT_YUV444P||frame->format==AV_PIX_FMT_RGBA){
      if(taskid==2)convert_and_output(frame);
      else convert_and_compose(frame);
    }
    else endwith("warning: wrong format");
    return FOUND_FRAME;
  }
}
void VideoContactSheet::frame_extraction(){
  int64_t time_stamp[10];
  for(int i=1;i<=6;i++){
    time_stamp[i]=(double)(fmt_ctx->duration/AV_TIME_BASE)/av_q2d(timebase)*(double)((i-1)*0.2);
    if(i==6)time_stamp[i]-=(double)(fmt_ctx->duration/AV_TIME_BASE)/av_q2d(timebase)*0.05;
  }
  for(int i=1;i<=6;i++){
    imgtag=i;
    int ret=av_seek_frame(fmt_ctx,vidx,time_stamp[i],AVSEEK_FLAG_BACKWARD);
    if(ret<0)endwith("warning: no such frame");
    while((ret=av_read_frame(fmt_ctx,pkt))>=0){
      if(pkt->stream_index==vidx){
        ret=decode_video(pAVFrame,time_stamp[i]);
        av_packet_unref(pkt);
        if(ret<0)endwith("warning: no such frame");
        if(ret==FOUND_FRAME)break;
      }
      else av_packet_unref(pkt);
    }
  }
}
void VideoContactSheet::resize_and_output(){
  int iw=dec_ctx->width*3;
  int ih=dec_ctx->height*2;
  int ow=iw;
  int oh=ih;
  if(max(iw,ih)>2160){
    if(iw>ih)ow=2160,oh=ih*ow/iw;
    else oh=2160,ow=iw*oh/ih;
  }
  auto *odata=(unsigned char *)malloc(ow*oh*3);
  stbir_resize(Frame_comp->data[0],iw,ih,0,odata,ow,oh,0,STBIR_TYPE_UINT8,3,STBIR_ALPHA_CHANNEL_NONE,0,STBIR_EDGE_CLAMP,STBIR_EDGE_CLAMP,STBIR_FILTER_BOX,STBIR_FILTER_BOX,STBIR_COLORSPACE_SRGB,NULL);
  if(taskid==4){
    for(int i=1;i<=6;i++){
      imgtag=i;
      add_text(odata,_timestamp[i],ow,oh/2);
    }
  }
  string temp=taskid==3?"combined.png":"contactsheet.png";
  string res=getfilename(temp);
  stbi_write_png(res.c_str(),ow,oh,3,odata,0);
  delete odata;
}
void VideoContactSheet::work(int _taskid){
  taskid=_taskid;
  frame_extraction();
  if(taskid>=3)resize_and_output();
  clear();
}