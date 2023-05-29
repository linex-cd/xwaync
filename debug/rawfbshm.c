#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


// 2160*1440*4的pnm文件
// 共享内存的大小
//#define SHM_SIZE  9331217
#define SHM_SIZE  9331220

key_t key = 1991;  // 指定的shmid
char *filename = "/tmp/rawfb/x11vnc_rawfb.pnm";  // 要读取的文件名
 
int main() {
    int shmid;
    char *shm;
    FILE *file;
   
	// 尝试获取已存在的共享内存段
    shmid = shmget(key, SHM_SIZE, 0);
    if (shmid == -1) {
        // 如果共享内存段不存在，则创建它
        shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
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


    // 持续读取文件内容到共享内存
    while (1) {
		 // 打开要读取的文件
		file = fopen(filename, "rb");
		if (file == NULL) {
			perror("fopen");
			exit(1);
		}

        // 清空共享内存
        //memset(shm, 0, SHM_SIZE);

        // 读取文件内容到共享内存
        fread(shm, sizeof(char), SHM_SIZE, file);

        // 打印共享内存内容
        printf("Shared Memory:id=%d,size=%d, file=%s\n", shmid, SHM_SIZE, filename);

		 // 关闭文件
		fclose(file);	
        // 等待文件更新
        //sleep(1);
    }

   

    // 分离共享内存
    shmdt(shm);

    // 删除共享内存段
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
