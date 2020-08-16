#include "v4l2_driver.h"


u_int8_t map_stat = 0;
u_int8_t req_stat = 0;
u_int8_t que_stat = 0;

char* date[COUNT] = {0};
char recbuf[IMG_WIDTH*IMG_HEIGHT*3] = {0};
unsigned int ret_get[800*480] = {0};

bmphead bmph;
bmpbody bmpb;
/* functions definations */
u_int8_t Cap_Init(char* dev,int *cap)
{
    /* 第一步，打开摄像头 */
    int ret;
    int i;
    
    //FILE *ptr;
    *cap = open(dev,O_RDWR);//以读写权限打开设备文件
    if(-1 == *cap)
    {
        printf("cap open failed!\r\n");
        return false;
    }
    /* 第二部，设置摄像头图片帧格式 */
    cap_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//操作类型为获取图片
    cap_format.fmt.pix.width = IMG_WIDTH;//输出图片宽度
    cap_format.fmt.pix.height = IMG_HEIGHT;//输出图片高度
    cap_format.fmt.pix.pixelformat = IMG_FMT;//图片编码格式
    cap_format.fmt.pix.field = V4L2_FIELD_ALTERNATE;
    ret = ioctl(*cap,VIDIOC_S_FMT,&cap_format);//将设置配置到摄像头
    /*
    *VIDIOC->video io control
    * S    -> set
    * FMT  -> format
    * 视频格式设置
    */
    if(-1 == ret)
    {
        printf("设置摄像头输出图片帧格式出错\r\n");
        close(*cap);
        return false;
    }
    /* 第三部，获取摄像头格式信息，检查设置是否成功 */
    ret = ioctl(*cap,VIDIOC_G_FMT,&cap_format);
    /*
    *VIDIOC->video io control
    * G    -> get
    * FMT  -> format
    * 获取视频格式设置
    */
    if(-1 == ret)
    {
        printf("get cap info failed!\r\n");
        close(*cap);
        return false;
    }

    if(cap_format.fmt.pix.width != IMG_WIDTH)
    {
        printf("capout width set failed!width:%d!\r\n",cap_format.fmt.pix.width);
    } 
    else
    {
        printf("capout width set ok!\r\n");
    }
    
    if(cap_format.fmt.pix.height != IMG_HEIGHT)
    {
        printf("capout height set failed!height:%d\r\n",cap_format.fmt.pix.height);
    } 
    else
    {
        printf("capout height set ok!\r\n");
    }
    if(cap_format.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV)
    {
        printf("capout FMT set failed!!\r\n");
    } 
    else
    {
        printf("capout FMT set ok!\r\n");
    }
    
    /* 第四步，申请图像缓冲区 */
    reqbuf.count = COUNT;//缓冲区个数
    reqbuf.type = BUF_TYPE;//缓冲区类型
    reqbuf.memory = V4L2_MEMORY_MMAP;//内存映射
    ret = ioctl(*cap,VIDIOC_REQBUFS,&reqbuf);//请求缓冲区
    if(-1 == ret)
    {
        printf("缓冲区请求失败!\r\n");
        close(*cap);
        return false;
    }


    for(i=0 ;i<COUNT ;i++ )
    {
        /* 第五步，查询每个缓冲区的信息，进行内存映射 */
        imgbuf[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        imgbuf[i].memory = V4L2_MEMORY_MMAP;
        imgbuf[i].index = i;//获取缓冲区索引
        ret = ioctl(*cap,VIDIOC_QUERYBUF,&imgbuf[i]);
        if(-1 == ret)
        {
            printf("Index%d缓冲区映射失败!\r\n",i);
            req_stat = -1;
            break;
        }
        printf("Index:%d lens:%d offset:%d\r\n",i,imgbuf[i].length,imgbuf[i].m.offset);//打印对应缓冲区的偏移量和长度
        //将映射的缓冲区放到进程中来(date)
        date[i] = mmap(NULL,imgbuf[i].length,PROT_READ|PROT_WRITE,MAP_SHARED,*cap,imgbuf[i].m.offset);
        if(MAP_FAILED == date[i])
        {
            printf("mmap failed!\r\n");
            map_stat = -1;
            break;
        }
        //将mmap成功的imgbuf加入图像采集队列
        ret = ioctl(*cap,VIDIOC_QBUF,&imgbuf[i]);
        if(-1 == ret)
        {
            printf("缓冲区加入图像队列失败!\r\n");
            que_stat = -1;
            break;
        }
        /*
        *这样一来，imgbuf采集到摄像头的图片帧
        *就会映射到date数组里
        *我们就能通过处理date来对图像进行操作
        */
    }
    //error deal
    if(-1 == req_stat)
    {

    }
    if(-1 == map_stat)
    {
        close(*cap);
        return false;
    }
    if(-1 == que_stat)
    {
        close(*cap);
        return false;
    }

    /* 第六步，启动采集 */
    /*
    int on = V4L2_BUF_TYPE_VIDEO_CAPTURE;//采集开启标志位
    ret = ioctl(cap,VIDIOC_STREAMON,&on);
    if(-1 == ret)
    {
        printf("开启采集失败!\r\n");
        close(cap);
        return false;
    }
    */
    /* 第七步，让已经采集完毕的缓冲区推出采集队列 */
    /*
    ret = ioctl(cap,VIDIOC_DQBUF,&imgbuf);
    if(-1 == ret)
    {
        printf("弹出缓冲区失败!\r\n");
        close(cap);
        return false;
    }
    */
    /* 第八步，从弹出的缓冲区映射的数组内接收图片信息 */
    /*
    ptr = fopen("./cap.yuyv","a+");
    if(NULL == ptr)
    {
        printf("file operation failed!\r\n");
        close(cap);
        fclose(ptr);
        return false;
    }
    fwrite(date[imgbuf.index],imgbuf.bytesused,1,ptr);
    fclose(ptr);
    close(cap);
    */
    return true;
}

u_int8_t capture_start(int *cap)
{
    int on = V4L2_BUF_TYPE_VIDEO_CAPTURE;//采集开启标志位
    int ret;
    ret = ioctl(*cap,VIDIOC_STREAMON,&on);
    if(-1 == ret)
    {
        printf("开启采集失败!\r\n");
        close(*cap);
        return false;
    }
    return true;
}

u_int8_t capture_stop(int *cap)
{
    int off = V4L2_BUF_TYPE_VIDEO_CAPTURE;//采集关闭标志位
    int ret;
    ret = ioctl(*cap,VIDIOC_STREAMOFF,&off);
    if(-1 == ret)
    {
        printf("停止采集失败!\r\n");
        close(*cap);
        return false;
    }
    return true;
}

u_int8_t Acquire_date(int *cap)
{
    int ret;
    FILE *ptr;
    struct timeval tv;
    fd_set fds;

    
    FD_ZERO(&fds);
    FD_SET(*cap, &fds);
    tv.tv_sec = 2;//time out
	tv.tv_usec = 0;
	ret = select(*cap+1, &fds, NULL, NULL, &tv);//判断摄像头是否准备好，tv是定时
    if(-1 == ret){
        printf("select erro! \n");
	}
	else if(0 == ret){
		printf("select timeout! \n");//超时
		return 1;
	}

    
    ret = ioctl(*cap,VIDIOC_DQBUF,&imgbuf[0]);
    if(-1 == ret)
    {
        printf("弹出缓冲区失败!\r\n");
        close(*cap);
        return false;
    }
    ptr = fopen("./cap.yuyv","a+");
    if(NULL == ptr)
    {
        printf("file operation failed!\r\n");
        close(*cap);
        fclose(ptr);
        return false;
    }
    fwrite(date[imgbuf[0].index],imgbuf[0].bytesused,1,ptr);
    fclose(ptr);
    ret = ioctl(*cap,VIDIOC_QBUF,&imgbuf[0]);
    if(-1 == ret)
    {
        printf("缓冲区加入图像队列失败!\r\n");
        //que_stat = -1;
        close(*cap);
        return false;
    }
    return true;
}

u_int8_t exit_cap(int *cap)
{
    int i;
    for(i=0;i<COUNT;i++)
    {
        munmap(date[i],imgbuf[i].length);
    }
    close(*cap);
    return true;
}

#define RGB24_MAX(x) (x > 255)? 255  : x 
#define RGB24_MIN(x) (x < 0)  ? 0    : x

void yuyv_to_rgb888(char *yuyv)
{
    int pix;
    int rgb_pix = 0;
    unsigned char y1,u1,y2,v1;
    int r1,g1,b1,r2,g2,b2;
    //像素空间
    for(pix=0;pix<IMG_WIDTH*IMG_HEIGHT*2;pix+=4)
        {
            y1 = yuyv[pix];
            u1 = yuyv[pix + 1];
            y2 = yuyv[pix + 2];
            v1 = yuyv[pix + 3];

            /* to rgb 24 */
            r1 = y1 + 1.370705*(v1-128);
            g1 = y1 - 0.698001*(v1-128) - 0.337633*(u1-128);
            b1 = y1 + 1.732446*(u1-128);
            r2 = y2 + 1.370705*(v1-128);
            g2 = y2 - 0.698001*(v1-128) - 0.337633*(u1-128);
            b2 = y2 + 1.732446*(u1-128);

            r1 = (RGB24_MAX(r1));
            g1 = (RGB24_MAX(g1));
            b1 = (RGB24_MAX(b1));
            r2 = (RGB24_MAX(r2));
            g2 = (RGB24_MAX(g2));
            b2 = (RGB24_MAX(b2));
            r1 = (RGB24_MIN(r1));
            g1 = (RGB24_MIN(g1));
            b1 = (RGB24_MIN(b1));
            r2 = (RGB24_MIN(r2));
            g2 = (RGB24_MIN(g2));
            b2 = (RGB24_MIN(b2));

            recbuf[rgb_pix]     = r1;
            recbuf[rgb_pix + 1] = g1;
            recbuf[rgb_pix + 2] = b1;
            recbuf[rgb_pix + 3] = r2;
            recbuf[rgb_pix + 4] = g2;
            recbuf[rgb_pix + 5] = b2;
            rgb_pix+=6;
        }
}

u_int8_t BMP_OUT(int *cap)
{
    int ret;
    FILE *ptr;
    struct v4l2_buffer current_buf;
    struct timeval tv;
    fd_set fds;

    
    FD_ZERO(&fds);
    FD_SET(*cap, &fds);
    tv.tv_sec = 2;//time out
	tv.tv_usec = 0;
	ret = select(*cap+1, &fds, NULL, NULL, &tv);//判断摄像头是否准备好，tv是定时
    if(-1 == ret){
        printf("select erro! \n");
	}
	else if(0 == ret){
		printf("select timeout! \n");//超时
		return 1;
	}
    

    char filename[50] = {0};
    static int i = 0;
    static unsigned int index = 0;
    
    __bzero(&current_buf,sizeof(current_buf));
    current_buf.index = index;
    current_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    //imgbuf[index].index = index;

    
    index++;
    if(index == COUNT)
        index = 0;

    ret = ioctl(*cap,VIDIOC_DQBUF,&current_buf);
    if(-1 == ret)
    {
        printf("弹出缓冲区失败!\r\n");
        close(*cap);
        return false;
    }
    sprintf(filename,"%s%d%s","./img_out/cap_",i,".bmp");
    i++;

    ptr = fopen(filename,"a+");
    if(NULL == ptr)
    {
        printf("file operation failed!\r\n");
        close(*cap);
        fclose(ptr);
        return false;
    }
    yuyv_to_rgb888(date[current_buf.index]);
    BMP_INIT(&bmph,&bmpb);
    fwrite(&bmph,14,1,ptr);
    fwrite(&bmpb,40,1,ptr);
    fwrite(recbuf,current_buf.bytesused*3,1,ptr);
    fclose(ptr);

    ret = ioctl(*cap,VIDIOC_QBUF,&current_buf);
    if(-1 == ret)
    {
        printf("缓冲区加入图像队列失败!\r\n");
        //que_stat = -1;
        close(*cap);
        return false;
    }
    return true;
}

void BMP_INIT(bmphead *head,bmpbody *body)
{
    body->biBitCount=24;
    body->biClrImportant=0;
    body->biClrUsed=0;
    body->biCompression=0;
    body->biHeight=IMG_HEIGHT;
    body->biPlanes=1;
    body->biSize=40;
    body->biSizeImage=imgbuf[0].bytesused*3;
    body->biWidth=IMG_WIDTH;
    body->biXPelsPerMeter=0;
    body->biYPelsPerMeter=0;

    head->bfOffBits=54;
    head->bfReserved=0;
    head->bfSize= 54 + (body->biSizeImage);
    head->bfType=0x4d42;
}

u_int8_t LCD_SHOW(int *cap,int fb,unsigned int *paddr)
{
    int ret;
    //FILE *ptr;
    struct v4l2_buffer current_buf;
    struct timeval tv;
    fd_set fds;

    
    FD_ZERO(&fds);
    FD_SET(*cap, &fds);
    tv.tv_sec = 2;//time out
	tv.tv_usec = 0;
	ret = select(*cap+1, &fds, NULL, NULL, &tv);//判断摄像头是否准备好，tv是定时
    if(-1 == ret){
        printf("select erro! \n");
	}
	else if(0 == ret){
		printf("select timeout! \n");//超时
		return 1;
	}
    

    char filename[50] = {0};
    //static int i = 0;
    static unsigned int index = 0;
    
    __bzero(&current_buf,sizeof(current_buf));
    current_buf.index = index;
    current_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    //imgbuf[index].index = index;

    
    index++;
    if(index == COUNT)
        index = 0;

    ret = ioctl(*cap,VIDIOC_DQBUF,&current_buf);
    if(-1 == ret)
    {
        printf("弹出缓冲区失败!\r\n");
        close(*cap);
        return false;
    }
    //sprintf(filename,"%s%d%s","./img_out/cap_",i,".bmp");
    //i++;

    //ptr = fopen(filename,"a+");
    /*
    if(NULL == ptr)
    {
        printf("file operation failed!\r\n");
        close(*cap);
        fclose(ptr);
        return false;
    }
    */
    yuyv_to_rgb888(date[current_buf.index]);
    process_get(recbuf);
    draw_bmp(paddr,ret_get);
    /*
    BMP_INIT(&bmph,&bmpb);
    fwrite(&bmph,14,1,ptr);
    fwrite(&bmpb,40,1,ptr);
    fwrite(recbuf,current_buf.bytesused*3,1,ptr);
    fclose(ptr);
    */

    ret = ioctl(*cap,VIDIOC_QBUF,&current_buf);
    if(-1 == ret)
    {
        printf("缓冲区加入图像队列失败!\r\n");
        //que_stat = -1;
        close(*cap);
        return false;
    }
    return true;
}

void draw_point(unsigned int *p, unsigned int color, int pos_x, int pos_y)
{
	//memcpy(p+pos_x+pos_y*800, &color, 4);
	*(p+pos_x+800*pos_y) = color;
}

void draw_bmp(unsigned int *p,unsigned int bmp[])
{
	for(int j = 0; j<480; j++)
	for(int i = 0; i<800; i++)
	{
		draw_point(p,bmp[i+800*j],i,j);
	}
}

void process_get(unsigned char addr[])
{
	int j = 0;
	for(int i = 0;i<800*480;i++)
	{
		ret_get[i] = (unsigned int)(addr[j]*256*256 + addr[j+1]*256 + addr[j+2]);
		j+=3;
	}
}