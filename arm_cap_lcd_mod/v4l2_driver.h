//generaric driver module
#ifndef __V4L2_DRIVER_H
#define __V4L2_DRIVER_H

/* headers */
//C标准库
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "poll.h"
//下面四个头文件是linux系统编程特有的
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "unistd.h"
//操作摄像头设备
#include <linux/videodev2.h>
#include "linux/v4l2-common.h"
#include "linux/v4l2-controls.h"
#include "linux/v4l2-dv-timings.h"
#include "linux/v4l2-mediabus.h"
#include "linux/v4l2-subdev.h"

/* World Define */
#define IMG_WIDTH  640
#define IMG_HEIGHT 480

#define SCR_WIDTH  1024
#define SCR_HEIGHT 600
#define SCR_DEEP   4     //正点原子的7寸TFT色深 为 24 -> 3B

#define IMG_FMT  V4L2_PIX_FMT_YUYV;
#define COUNT 4//缓冲区个数
#define BUF_TYPE V4L2_BUF_TYPE_VIDEO_CAPTURE//缓冲区类型
#define MMER V4L2_MEMORY_MMAP//内存映射模式
#define false 0;
#define true  1;

/* Value extern */
extern u_int8_t map_stat;
extern u_int8_t req_stat;
extern u_int8_t que_stat;

extern unsigned char recbuf[IMG_WIDTH*IMG_HEIGHT*3];
/* structors defination  */
struct v4l2_format cap_format;//视屏输出图像帧格式结构体
struct v4l2_requestbuffers reqbuf;//请求缓冲区的结构体
struct v4l2_buffer imgbuf[COUNT];//内存映射后的图像储存结构体

/* bmp img structor */
typedef struct{
     u_int16_t    bfType;                // the flag of bmp, value is "BM"
     u_int32_t    bfSize;                // size BMP file ,unit is bytes
     u_int32_t    bfReserved;            // 0
     u_int32_t    bfOffBits;             // must be 54
}bmphead;

typedef struct{
     u_int32_t    biSize;            // must be 0x28
     u_int32_t    biWidth;           //
     u_int32_t    biHeight;          //
     u_int16_t    biPlanes;          // must be 1
     u_int16_t    biBitCount;        //
     u_int32_t    biCompression;     //
     u_int32_t    biSizeImage;       //
     u_int32_t    biXPelsPerMeter;   //
     u_int32_t    biYPelsPerMeter;   //
     u_int32_t    biClrUsed;         //
     u_int32_t    biClrImportant;    //
}bmpbody;

/* Functions extern */
u_int8_t Cap_Init(char* dev,int *cap);
u_int8_t exit_cap(int *cap);
u_int8_t Acquire_date(int *cap);
u_int8_t capture_stop(int *cap);
u_int8_t capture_start(int *cap);
void yuyv_to_rgb888(char *yuyv);
u_int8_t BMP_OUT(int *cap);
void BMP_INIT(bmphead *head,bmpbody *body);

void process_get(unsigned char addr[]);
void draw_bmp(unsigned int *p,unsigned int bmp[]);
void draw_point(unsigned int *p, unsigned int color, int pos_x, int pos_y);
u_int8_t LCD_SHOW(int *cap,int fb,unsigned int *paddr);

unsigned char gray_deal(char r,char g,char b);
void yuyv_to_rgb888_with_gray(char *yuyv);

#endif