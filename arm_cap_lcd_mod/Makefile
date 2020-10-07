PRO_NAME ?= libv4l2_driver.a
OBJ      ?= v4l2_driver.o
CC       ?= gcc
AR		 ?= ar


CC       = arm-linux-gnueabihf-gcc
AR		  = arm-linux-gnueabihf-ar	

$(PRO_NAME):$(OBJ)
	$(AR) -cr $@ $^ 

%.o : %.c
	$(CC) -o $@ -c $< -std=c11

clean:
	rm -rf $(PRO_NAME) *.o demo *.yuyv ./img_out/*.bmp

demo:
	$(CC) -o demo ./demo.c -lv4l2_driver -L./