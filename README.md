# FairyCloud_ESP32S3BD_PUBLIC
ESP32S3BD物联网开发板-硬件代码；欢迎交流：QQ群：630017549 ，个人微信：fairycloud2035，QQ：1055417926 


## 代码说明
### 代码目录
FairyCloud_ESP32S3BD_PUBLIC/code/


### 配置文件说明

#### 1.appkey、secretkey、url地址
说明：通信使用的秘钥，用于连接到物联网平台，直接问管理员获取即可；

FairyCloud_ESP32S3BD_PUBLIC/code/src/config.hpp

String appkey  ="XXX";
String secretkey  ="XXX";
String getsrccid_url = "XXX";


#### 2.常用的WiFi账号和密码
说明：设备连接的WiFi名称和密码，支持多个WiFi账号设置，哪个能连上就连接哪个

const char *wifiData[][2] = {
    {"Gunter", "{qwerty123}"}, 
    {"CMCC-AKtZ", "0102030405"},
    {"Fairy", "{qwerty123}"},
    {"XXX", "XXX"},
    // 继续添加需要的 Wi-Fi 名称和密码
};


#### 3.version
说明：软件版本号，自行设置/修改：三位数字，第一位是大版本、第二位是小版本，第三位是微调

String version  = "0.0.3";


#### 4.烧写代码
说明：完成以上3步配置，烧录代码目录所有.lua尾缀的代码，即可正常运行；

FairyCloud_ESP32S3BD_PUBLIC/code/

## 示例教程

### 实物演示
[【ESP32实现自动化执行任务，支持小程序一键控制】](https://b23.tv/4Z2b86y)


### 说明文档
[【外部】精灵物联网各项目汇总](https://gv9jqt8gpcb.feishu.cn/docx/DAJGdExvZoZBA3xuAogc53ohnxg?from=from_copylink)

### 实物图片
![image](https://github.com/fairycloudpublic/FairyCloud_ESP32S3BD_PUBLIC/blob/main/photo1.png)

![image](https://github.com/fairycloudpublic/FairyCloud_ESP32S3BD_PUBLIC/blob/main/photo2.png)

![image](https://github.com/fairycloudpublic/FairyCloud_ESP32S3BD_PUBLIC/blob/main/photo3.png)

![image](https://github.com/fairycloudpublic/FairyCloud_ESP32S3BD_PUBLIC/blob/main/photo4.png)

![image](https://github.com/fairycloudpublic/FairyCloud_ESP32S3BD_PUBLIC/blob/main/photo5.png)

![image](https://github.com/fairycloudpublic/FairyCloud_ESP32S3BD_PUBLIC/blob/main/photo6.png)


## 版权说明
仅限用于学习和研究目的，请勿用于非法用途，概不负责，一切后果由用户自行承担！未经版权所有权人书面许可，不能自行用于商业用途。如需作商业用途，请与原作者联系。

### 许可协议
许可协议 AGPL3.0协议

### 软著证书
![image](https://github.com/fairycloudpublic/FairyCloud_ESP32S3BD_PUBLIC/blob/main/%E7%B2%BE%E7%81%B5%E7%89%A9%E8%81%94%E7%BD%91%E5%B9%B3%E5%8F%B0%E7%89%88%E6%9D%83.png)
