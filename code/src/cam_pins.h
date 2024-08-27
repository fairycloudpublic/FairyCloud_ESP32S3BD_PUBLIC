//
// Created by lustre on 2023/2/2.
//

#ifndef CAM_PINS_H
#define CAM_PINS_H

#ifdef CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#elif defined(HE_ZHOU_S3)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 9
#define SIOD_GPIO_NUM 14
#define SIOC_GPIO_NUM 13
// 这里Y几并非真正的原理图上引脚的标号 而是 适配 D0 ~ D7 固定一套 Y标号的写法 以便于兼容其它esp32摄像头的定义
#define Y9_GPIO_NUM 18 //D7
#define Y8_GPIO_NUM 17 //D6
#define Y7_GPIO_NUM 16 //D5
#define Y6_GPIO_NUM 15 //D4
#define Y5_GPIO_NUM 6 //D3
#define Y4_GPIO_NUM 4 //D2
#define Y3_GPIO_NUM 5 //D1
#define Y2_GPIO_NUM 7 //D0
#define VSYNC_GPIO_NUM 11
#define HREF_GPIO_NUM 10
#define PCLK_GPIO_NUM 8
#endif

#endif // CAM_PINS_H
