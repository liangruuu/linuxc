#include <unistd.h>  
#include <stdio.h>  
#include <fcntl.h>  
#include <linux/fb.h>  
#include <sys/mman.h>  
#include <stdlib.h>  
#include <sys/ioctl.h>
#include <error.h>


int main ()   
{  
     int fp=0;  
     struct fb_var_screeninfo vinfo;  
     struct fb_fix_screeninfo finfo; 
     long screensize = 0;
     char *fbp =0;
     int x=0,y=0;
     long location =0;

 
     fp = open ("/dev/fb0",O_RDWR);  

     if (fp < 0)
     {  
      perror("fopen:");  
      exit(1);  
     }  

     if (ioctl(fp,FBIOGET_FSCREENINFO,&finfo))
     {  
      perror("fopen:");  
      exit(2);  
     }  

     if (ioctl(fp,FBIOGET_VSCREENINFO,&vinfo))
     {  
      perror("fopen:");  
      exit(3);  
     }  

     printf("The mem is :%d\n",finfo.smem_len);  
     printf("The line_length is :%d\n",finfo.line_length);  
     printf("The xres is :%d\n",vinfo.xres);  
     printf("The yres is :%d\n",vinfo.yres);  
     printf("bits_per_pixel is :%d\n",vinfo.bits_per_pixel);  

     screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
     fbp =(char*)mmap(0, screensize, PROT_READ|PROT_WRITE, MAP_SHARED,fp,0);

     if(fbp == MAP_FAILED)
     {   
        perror("mmap:");
        exit(4);
     }
     x = 100;
     y = 100;

     location = x *(vinfo.bits_per_pixel/8) + y * finfo.line_length;
     *(fbp+location) = 100;
     *(fbp+location+1) = 15;
     *(fbp+location+2) = 200;
     *(fbp+location+3) = 0;
     munmap(fbp, screensize);
     
     close(fp);  
} 
