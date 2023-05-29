#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

int main() {
    int fbfd = 0;
    struct fb_var_screeninfo var_info;

    // Open the framebuffer device file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &var_info) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    printf("%dx%d, %dbpp\n", var_info.xres, var_info.yres, var_info.bits_per_pixel);

    // Calculate the size to mmap
    int screensize = var_info.yres * var_info.xres * var_info.bits_per_pixel / 8;

    // Map the device to memory
    char *fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (atoi(fbp) == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }

    // Your business logic here: save the framebuffer data to a file.
    // This will depend on the exact format of your framebuffer.
    // For this example, we assume it's 24-bit RGB.

    FILE *file = fopen("screenshot.rgb", "wb");
    if (file == NULL) {
        perror("Error: cannot open output file");
        exit(5);
    }

    fwrite(fbp, screensize, 1, file);
    fclose(file);

    // Clean up
    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}
