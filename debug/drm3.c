#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <time.h>

#define FILE_NAME "drmfb.ppm"


int get_screen_info(struct fb_var_screeninfo *screeninfo)
{
	
	int fbfd = 0;
    struct fb_var_screeninfo var_info;

    // Open the framebuffer device file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        return -1;
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &var_info) == -1) {
        perror("Error reading variable information");
        return -3;
    }

    printf("%dx%d, %dbpp\n", var_info.xres, var_info.yres, var_info.bits_per_pixel);

    // Calculate the size to mmap
    int screensize = var_info.yres * var_info.xres * var_info.bits_per_pixel / 8;
	
	*screeninfo = var_info;
	
	return screensize;
	
}

int savetofile(const char *filename, void* data, int size, int width, int height)
{
	/*
	
	像素数据是从 DRM framebuffer 中读取的。DRM framebuffer 存储了屏幕上每个像素的颜色信息。在这种情况下，每个像素的颜色信息由四个字节表示：一个字节的红色（R），一个字节的绿色（G），一个字节的蓝色（B），和一个字节的透明度（A），这就是所谓的 ARGB 格式。这种格式在计算机图形中是很常见的，因为它可以用一个 32 位的整数来表示一个像素的颜色。

	在 framebuffer 中，像素数据是以行优先的顺序存储的。也就是说，第一个像素是左上角的像素，接着是第一行的其余像素，然后是第二行的像素，以此类推，直到右下角的像素。因此，我们可以通过一个双重循环来遍历所有的像素。

	对于每个像素，我们首先计算它在 framebuffer 中的位置（即偏移量）。然后，我们读取这个位置的四个字节，并将它们转换为一个 32 位的整数。由于我们的系统是小端序的，所以最低有效字节（LSB）是红色分量，接着是绿色分量，然后是蓝色分量，最后是透明度分量。这四个字节的顺序是 0xAARRGGBB。

	在我们的代码中，我们只关心 RGB 三个分量。所以，我们将这个 32 位的整数分解成三个 8 位的整数，然后忽略透明度分量。我们将这三个整数以 PPM 格式写入到文件中，即每个分量占一个字节，红色分量在前，绿色分量在中，蓝色分量在后。
	*/
	
	 // 将数据写入文件
    int file_fd = open(filename, O_WRONLY | O_CREAT, 0666);
    if (file_fd < 0) {
        perror("open file failed");
        
        return 0;
    }

	
	//PNM header
	char header[200] = {0};
	sprintf(header, "P6\n%d %d\n255\n", width, height);

    write(file_fd, header, strlen(header));
    write(file_fd, data, size);
	

    // 清理
    close(file_fd);
	
	return 1;
}

/*
int raw2pnm(const char* raw, char* pnmbuf, int width, int height)
{
	int pitch = 4;
	long offset = 0;
    for (int y = 0; y < height; ++y) {
        uint32_t *row = (uint32_t *)(((char *)map) + y * pitch);
        for (int x = 0; x < width; ++x) {
            uint32_t pixel = row[x];
            uint8_t colors[3] = {pixel >> 16, pixel >> 8, pixel};
            //fwrite(colors, sizeof(colors), 1, out);
			memcpy(&pnmbuf[offset++], colors, sizeof(colors) );
			pnmbuf[offset++] = colors;
        }
    }
	
}
*/

int main() {
    // 打开 DRM 设备
    int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        // Handle error
        return 1;
    }

    // 获取当前的 CRTC（显示管道）
    drmModeRes* res = drmModeGetResources(fd);
    if (!res) {
        // Handle error
        return 1;
    }
    drmModeCrtc* crtc = drmModeGetCrtc(fd, res->crtcs[0]);  // 获取第一个 CRTC

    // 获取帧缓冲区
    drmModeFB* fb = drmModeGetFB(fd, crtc->buffer_id);

    printf("fb handle = %d\n", fb->handle);

    // 创建一个用于 DMA-BUF 的文件描述符
    int dma_buf_fd = 0;
    struct drm_prime_handle prime_handle = {
        .handle = fb->handle,
        .fd = -1,
        .flags = 0
    };
    if (ioctl(fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime_handle) < 0) {
        perror("ioctl failed");
        // Handle error
        return 1;
    }
    printf("fd = %d, dma_buf_fd  = %d \n", prime_handle.fd, dma_buf_fd);
    // dma_buf_fd 现在可以用于访问帧缓冲区的数据
    dma_buf_fd = prime_handle.fd;
    // ...
	
	 // 获取 DMA-BUF 的大小
	off_t size = lseek(dma_buf_fd, 0, SEEK_END);
	lseek(dma_buf_fd, 0, SEEK_SET);
    printf("dma_buf_fd %d size=%d\n", dma_buf_fd, size);

    // 将 DMA-BUF 映射到内存
    void* map = mmap(NULL, size, PROT_READ, MAP_SHARED, dma_buf_fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

	//计时器
	clock_t start, end;
    double cpu_time_used;

    start = clock();
    savetofile(FILE_NAME, map, size, fb->width, fb->height);
	
	end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("savetofile 1 took %f seconds to execute\n", cpu_time_used);
	
	
	//
	sleep(10);
	
	start = clock();
    savetofile("2.ppm", map, size, fb->width, fb->height);
	
	end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("savetofile 2 took %f seconds to execute\n", cpu_time_used);
	
    // 清理

    munmap(map, size);


    // 清理
    close(dma_buf_fd);
    drmModeFreeFB(fb);
    drmModeFreeCrtc(crtc);
    drmModeFreeResources(res);
    close(fd);
    printf("success\n");
    return 0;
}
