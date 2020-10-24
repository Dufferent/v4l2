#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/types.h"
#include "linux/fb.h"
#include "linux/videodev2.h"
#include "sys/ioctl.h"
#include "sys/mman.h"
#include "string.h"

typedef unsigned char u8;
typedef unsigned int u32;
#define FB_WIDTH    800
#define FB_HEIGHT   480
#define FB_DEEP 4

class lcd{
    public:
    int width;
    int height;
    int deep;
    int fd;
    u32 *mmbuf;

    lcd()
    {
    }
    lcd(int x,int y,int dp,char *arg)
    {
        width = x;
        height = y;
        deep = dp;
        fd = Lcd_Init(arg);
        printf("w:%d\r\nh:%d\r\ndeep:%d\r\nfds:%d\r\n",width,height,deep,fd);
        if(fd < 0)
        {
            printf("fb open failed!\r\n");
            close(fd);
            exit(-1);
        }
        
        mmbuf = (u32*)mmap(NULL,width*height*deep,PROT_WRITE|PROT_READ,MAP_SHARED,fd,0);
        if(mmbuf == NULL)
        {
            printf("memery remap failed!\r\n");
            close(fd);
            exit(-1);
        }
    }
    ~lcd()
    {
    }
    int Lcd_Init(char *arg)
    {
        return open(arg,O_RDWR);
    }
    void Drow_Point(int x,int y,int color)
    {
        mmbuf[y*width + x] = color;
    }
    void Fill_All(u32 color)
    {
        for(int y=0;y<height;y++)
        for(int x=0;x<width;x++)
        {
            Drow_Point(x,y,color);
        }
    }
    void Lcd_DeInit()
    {
        munmap(mmbuf,width*height*deep);
        close(fd);
    }
    void Fill_Graph(u32 *graph,u32 cap_width,u32 cap_height)
    {
        for(int y=0;y<height;y++)
        for(int x=0;x<width;x++)
        {
            if(y<cap_height && x<cap_width)
                Drow_Point(x,y,graph[y*cap_width + x]);
            else
                Drow_Point(x,y,0x00);
        } 
    }
};

#define CAP_WIDTH     640
#define CAP_HEIGHT    480
#define BUF_CT 4

#define FIELD_PIX_MAX(x) ((x>255)?(255):x)
#define FIELD_PIX_MIN(x) ((x<0)?(0):x)



class cap{
    public:
    int cap_width;
    int cap_height;
    int cap_buf_ct;
    int fd;
    int ret;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers rbuf;
    struct v4l2_buffer buf;
    u32 *rgb;
    u8 **start;
    cap()
    {}
    ~cap()
    {}
    cap(int x,int y,int buf_ct)
    {
        cap_width = x;
        cap_height = y;
        cap_buf_ct = buf_ct;
        start = (u8 **)malloc(sizeof(u8*)*cap_buf_ct);
        rgb   = (u32*)malloc(sizeof(u32)*cap_width*cap_height);
    }
    void Cap_Init(char* arg)
    {
        fd = open(arg,O_RDWR);
        if(fd < 0)
        {
            printf("cap open failed!\r\n");
            exit(-1);
        }
        //step1:设置摄像头格式 <必须要先进行设置>
        //<注：以下所有摄像头的IO操作都用地址传结构体>
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.height = cap_height;
        fmt.fmt.pix.width  = cap_width;
        fmt.fmt.pix.field  = V4L2_FIELD_ALTERNATE; 
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        ret = ioctl(fd,VIDIOC_S_FMT,&fmt);
        if(ret == -1)
        {
            printf("cap set fmt failed!\r\n");
            close(fd);
            exit(-1);
        }
        //step2:检查摄像头格式设置  <一定要和设置的FMT同一个结构体地址>
        ret = ioctl(fd,VIDIOC_G_FMT,&fmt);
        if(ret == -1)
        {
            printf("cap get fmt failed!\r\n");
            close(fd);
            exit(-1);
        }
        printf("w:\t%d\r\n",fmt.fmt.pix.width);
        printf("h:\t%d\r\n",fmt.fmt.pix.height);
        printf("pixfmt:\t%s\r\n",(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV)?("true"):("false"));

        //step3:申请缓冲区
        rbuf.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        rbuf.count = cap_buf_ct;
        rbuf.memory = V4L2_MEMORY_MMAP;
        ret = ioctl(fd,VIDIOC_REQBUFS,&rbuf);
        if(ret == -1)
        {
            printf("cap rquest buf failed!\r\n");
            close(fd);
            exit(-1);
        }

        //step4：将缓冲区列队 <注：以下所有的缓冲区都要保持一致>
        for(int i=0;i<cap_buf_ct;i++)
        {
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            ret = ioctl(fd,VIDIOC_QUERYBUF,&buf);
            if(ret == -1)
            {
                printf("cap querry %d号 buf failed!\r\n",i);
                close(fd);
                exit(-1);
            }
            printf("buf_size:%2d\r\n",buf.bytesused);
            printf("buf_lenth:%2d\r\n",buf.length);
            start[i] = (u8*)mmap(NULL,buf.length,PROT_READ,MAP_SHARED,fd,buf.m.offset);
            if(start[i] == NULL)
                printf("%d号缓冲区映射失败！\r\n",i);
        }

        //step5:缓冲区入队
        for(int i=0;i<cap_buf_ct;i++)
        {
            buf.index = i;
            ret = ioctl(fd,VIDIOC_QBUF,&buf);
            if(ret == -1)
            {
                printf("cap queue %d号 buf failed!\r\n",i);
                close(fd);
                exit(-1);
            }
        }
    }
    void Cap_On()
    {
        //step6:开启摄像头
        static int on = 1;
        ret = ioctl(fd,VIDIOC_STREAMON,&on);
        if(ret == -1)
        {
            printf("cap start failed!\r\n");
            close(fd);
            exit(-1);
        }
    }
    void Cap_Off()
    {
        //step10:关闭摄像头
        static int off = 0;
        ret = ioctl(fd,VIDIOC_STREAMOFF,&off);
        if(ret == -1)
        {
            printf("cap off failed!\r\n");
            close(fd);
            exit(-1);
        }
        close(fd);
    }
    void Read_Frame()
    {
        //step7:弹出缓冲区
        ret = ioctl(fd,VIDIOC_DQBUF,&buf);
        if(ret == -1)
        {
            printf("cap dqueue buf failed!\r\n");
            close(fd);
            exit(-1);
        }

        //step8:读出数据
        PIXFMT_CONVERT(start[buf.index],rgb);

        //step9:重新入队
        ret = ioctl(fd,VIDIOC_QBUF,&buf);
        if(ret == -1)
        {
            printf("cap dqueue buf failed!\r\n");
            close(fd);
            exit(-1);
        }
    }

    void yuyv_2_rgb888(u8 y1,u8 u,u8 y2,u8 v,u32 *pix32,u32 ct)
    {
        u8 r1,g1,b1,r2,g2,b2;
        r1 = 1.164*(y1-16) + 1.159*(v-128); 
        g1 = 1.164*(y1-16) - 0.380*(u-128)+ 0.813*(v-128); 
        b1 = 1.164*(y1-16) + 2.018*(u-128); 

        r2 = 1.164*(y2-16) + 1.159*(v-128); 
        g2 = 1.164*(y2-16) - 0.380*(u-128)+ 0.813*(v-128); 
        b2 = 1.164*(y2-16) + 2.018*(u-128); 

        r1 = FIELD_PIX_MAX(r1);
        r1 = FIELD_PIX_MIN(r1);
        g1 = FIELD_PIX_MAX(g1);
        g1 = FIELD_PIX_MIN(g1);
        b1 = FIELD_PIX_MAX(b1);
        b1 = FIELD_PIX_MIN(b1);
        r2 = FIELD_PIX_MAX(r2);
        r2 = FIELD_PIX_MIN(r2);
        g2 = FIELD_PIX_MAX(g2);
        g2 = FIELD_PIX_MIN(g2);
        b2 = FIELD_PIX_MAX(b2);
        b2 = FIELD_PIX_MIN(b2);

        pix32[ct] = (r1<<16)|(g1<<8)|b1;
        pix32[ct+1] = (r2<<16)|(g2<<8)|b2;
    }
    //yuyv:4:2:2 <y1,u,y2,v各占1字节 4字节两个像素>
    void PIXFMT_CONVERT(u8 *addr,u32 *rgbbuf)
    {
        u8 y1,u,y2,v;
        u32 ct32 = 0;
        for(int ii=0;ii<cap_width*cap_height*2;)
        {
            y1 = addr[ii];
            u  = addr[ii+1];
            y2 = addr[ii+2];
            v  = addr[ii+3];
            ii+=4;
            yuyv_2_rgb888(y1,u,y2,v,rgbbuf,ct32);
            ct32 += 2;
        }
        printf("rgb buf get!\r\n");
    }
    void Cap_DeInit()
    {
        for(int i=0;i<cap_buf_ct;i++)
            munmap(start[i],buf.length);
        free(rgb);
        free(start);
    }
};


int main(int argc,char** argv)
{
    class lcd mylcd(FB_WIDTH,FB_HEIGHT,FB_DEEP,argv[1]);//lcd对象
    class cap mycap(CAP_WIDTH,CAP_HEIGHT,BUF_CT);
    mycap.Cap_Init(argv[2]);
    mycap.Cap_On();
    mycap.Read_Frame();
    mylcd.Fill_Graph(mycap.rgb,mycap.cap_width,mycap.cap_height);
    mycap.Cap_Off();
    mycap.Cap_DeInit();
    mylcd.Lcd_DeInit(); //lcd析构
    return 0;
}

