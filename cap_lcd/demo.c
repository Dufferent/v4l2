#include "./v4l2_driver.h"
#include "time.h"

int main(int argc,char** argv)
{
    int cap_fd;
    int ret;
    //int frame_ct = 20;
    int fb = open("/dev/ubuntu_lcd", O_RDWR);
    if(-1 == fb)
    {
        perror("open fb error!");
        exit(1);
    }

    unsigned int *paddr = (unsigned int*)mmap(NULL,800*480*4,PROT_WRITE|PROT_READ,MAP_SHARED,fb,0);

    ret = Cap_Init("/dev/video0",&cap_fd);
    
    if(ret)
    {
        //ret = Acquire_date(&cap_fd);
        capture_start(&cap_fd);
        //Acquire_date(&cap_fd);
        while(1)
        {
            //BMP_OUT(&cap_fd);
            LCD_SHOW(&cap_fd,fb,paddr);
            usleep(30*1000);
        }
        capture_stop(&cap_fd);
        exit_cap(&cap_fd);
    }
    return 0;
}