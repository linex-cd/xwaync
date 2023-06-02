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


#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdlib.h>



key_t key = 1991;  // 指定的shmid


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
    //drmModeCrtc* crtc = drmModeGetCrtc(fd, res->crtcs[0]);  // 获取第一个 CRTC
	drmModeCrtc* crtc = NULL;
	printf("found res->count_crtcs=%d\n", res->count_crtcs);
    for (int i = 0; i < res->count_crtcs; i++) {
		printf("current crtc id=%d\n", i);
        crtc = drmModeGetCrtc(fd, res->crtcs[i]);
        if (crtc->mode_valid && crtc->buffer_id != 0) {
            // 找到了正在使用的 CRTC
			
			
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
				puts("ioctl failed");
				// Handle error
				//continue;
			}
			
			else{
				puts("ioctl ok");
				break;
				
			}
			
            
        }
        drmModeFreeCrtc(crtc);
        crtc = NULL;
    }
	
    if (!crtc) {
        // 未找到正在使用的 CRTC
        // Handle error
        return 1;
    }
	
	puts("loading framebuffer");
	
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
    printf("dma_buf_fd %d size=%ld\n", dma_buf_fd, size);

    // 将 DMA-BUF 映射到内存
    void* map = mmap(NULL, size, PROT_READ, MAP_SHARED, dma_buf_fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

	
	//计时器
	clock_t start, end;
	double cpu_time_used;
	

	
	//循环保存到共享空间
	int shmid;
    char *shm;

   
	// 尝试获取已存在的共享内存段
    shmid = shmget(key, size, 0);
    if (shmid == -1) {
        // 如果共享内存段不存在，则创建它
        shmid = shmget(key, size, IPC_CREAT | 0666);
        if (shmid == -1) {
            perror("shmget");
            exit(1);
        }
    }
	
 
    // 附加共享内存
    shm = shmat(shmid, NULL, 0);
    if (shm == (char *)-1) {
        perror("shmat");
        exit(1);
    }

	
	
	
	while(1)
	{

		start = clock();
		
		//同步内存
		memcpy(shm, map, size);
	
		end = clock();

		cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
		printf("SYNC to SHared Memory took %f seconds to execute\n", cpu_time_used);
		
		struct timespec req = {0};
		req.tv_sec = 0; // 0 second
		req.tv_nsec = 100000000L; // 100 million nanoseconds = 0.1 seconds
		nanosleep(&req, NULL);
		
	}
	
	 // 分离共享内存
    shmdt(shm);

    // 删除共享内存段
    shmctl(shmid, IPC_RMID, NULL);


	
    // 清理
    munmap(map, size);


    // 清理
    close(dma_buf_fd);
    drmModeFreeFB(fb);
    drmModeFreeCrtc(crtc);
    drmModeFreeResources(res);
    close(fd);
    printf("success done\n");
    return 0;
}
