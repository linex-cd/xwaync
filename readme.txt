# xwaync
用于x11vnc的rawfb shm模式，可以兼容x11和wayland环境。

本程序使用服务安装后，可以避免x11vnc启动时请求sudo。


## 安装依赖
```
sudo apt install libdrm-dev
```

## 编译
```
gcc xwaync.c -o xwaync -ldrm -I/usr/include/drm
```

## 安装部署

```
# 必须安装udev规则，否则只有sudo/root才能接收键鼠消息
bash uinput.sh

# 安装服务，后台同步framebuffer，并开机启动。
bash installxwaync.sh
```

## 卸载服务
```
bash removexwaync.sh
```


## 制作安装包
默认arm64架构，可以在package/info.txt修改
```
sudo bash makedeb.sh
```