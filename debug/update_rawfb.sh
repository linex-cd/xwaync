#!/bin/bash

buffer_size=10
counter=0

# Create a buffer for image files
for i in $(seq 0 $((buffer_size-1))); do
    touch /tmp/rawfb/output_$i.png
done

while true; do
    latest_image="/tmp/rawfb/output_$((counter % buffer_size)).png"
    if [ -n "$latest_image" ]; then
        ffmpeg -f kmsgrab -i - -vf 'hwdownload,format=bgr0,scale=2160:1440:nv12' -vframes 1 -y "$latest_image"
        #convert "$latest_image" /tmp/rawfb/output.rgb
        
        #cp /tmp/rawfb/output.rgb /tmp/rawfb/output.rgb.tmp
        #mv /tmp/rawfb/output.rgb.tmp /tmp/rawfb/x11vnc_rawfb.rgb

        pngtopnm "$latest_image" > /tmp/rawfb/output.pnm
        cp /tmp/rawfb/output.pnm /tmp/rawfb/output.pnm.tmp
        mv /tmp/rawfb/output.pnm.tmp /tmp/rawfb/x11vnc_rawfb.pnm

        counter=$((counter + 1))
    fi
    #sleep 0.1
done
