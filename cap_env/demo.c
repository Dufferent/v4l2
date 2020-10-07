#include "./v4l2_driver.h"

int main(int argc,char** argv)
{
    int cap_fd;
    int ret;
    int frame_ct = 20;
    ret = Cap_Init(argv[1],&cap_fd);
    if(ret)
    {
        //ret = Acquire_date(&cap_fd);
        capture_start(&cap_fd);
        Acquire_date(&cap_fd);
        while(frame_ct--)
        {
           
            BMP_OUT(&cap_fd);
        }
        capture_stop(&cap_fd);
        exit_cap(&cap_fd);
    }
    return 0;
}
