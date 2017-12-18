# encode_video

This program is based on ffmpeg's `doc/example/encode_video.c` with some changes
including:
1. read a raw video file specifid by the argument instead of making a fake one.
   The width, height and pixel format is coded in source code.
2. codec name is coded in source code, which is `libx265` now. You can change 
   it to other codecs.
3. The process of reading raw video data into frame only supports NV12 and I420.
   You can use conditional macro to switch.
4. The encoded data in packets can be save to separate files by `save_packet` or
   into one file by `save_all_packets_one_file`, use conditional macro to switch.
   
 This programe is based on the new ffmpeg's API, i.e. send_frame and receive_packet
 pair.
