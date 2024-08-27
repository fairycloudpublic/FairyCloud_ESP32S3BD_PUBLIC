#include <Arduino.h>
#include <stdio.h>

// -----------------------------------------------begin------------------------------------------

//1.appkey+secretkey+url地址：直接问管理员获取
String appkey  ="XXX";
String secretkey  ="XXX";
String getsrccid_url = "XXX";

//2. 替换为自己常用的wifi名和密码，支持多个
const char *wifiData[][2] = {
    {"Gunter", "{qwerty123}"}, 
    {"CMCC-AKtZ", "0102030405"},
    {"Fairy", "{qwerty123}"},
    {"XXX", "XXX"},
    // 继续添加需要的 Wi-Fi 名称和密码
};

//3.软件版本号：三位数字，第一位是大版本、第二位是小版本，第三位是微调
String version  = "0.0.3";

// -----------------------------------------------end------------------------------------------



//MQTT订阅和发布服务定义--自动获取
String mqtt_pub_topicsss = "";
String mqtt_sub_topicsss = "" ;
String combinecontroll_url = "";
String photourl = ""; 


//设备的编号，系统自行获取
String SRCCID  = "";
String mqtt_mqttClientId  = "";
String mqtt_username  = "";
String mqtt_passwd  = "";
String mqtt_mqttHostUrl  = "";
uint16_t mqtt_port = 0;

String udp_host = ""; //UDT 地址
uint16_t udp_port = 0;  //UDT 端口

//硬件的MAC地址，自行获取
String MAC  = "";


uint8_t old_temp =  0;//上一次有效的温度
uint8_t old_humi =  0;//上一次有效的湿度
uint8_t old_airc =  0;//上一次有效的湿度
String deviceStatus = "";
String old_reporttime = "0000/00/00 00:00:00";

String old_rc1 = "  ";
String old_rc2 = "  ";
String old_rc3 = "  ";
String old_rc4 = "  ";

uint8_t alarmLevel =  0;//告警级别  0没有 1低级 2中级 3最高级


uint8_t ac_temp =  25;//默认的温度
uint8_t ac_fan = 5;//风力 5auto默认  3low 2 med 1high

//功能引脚定义
const int gasSensor =1; //因为连上WIFE后许多引脚读不了模拟值，但是34、35、36、39可以读取模拟值

const uint16_t kIrLed = 0;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).


// #define TFT_MOSI 48 // In some display driver board, it might be written as "SDA" and so on.
// #define TFT_SCLK 47
// #define TFT_CS   39  // Chip select control pin
// #define TFT_DC   40  // Data Command control pin
// #define TFT_RST  38  // Reset pin (could connect to Arduino RESET pin)
// #define TFT_BL   -1  // LED back-light



// 四个继电器引脚
#define rc1 12
#define rc2 21
#define rc3 41
#define rc4 45

// 蜂鸣器
#define alarm 42 

String ac = "close";


//设备发送数据到平台，数据格式定义，JSON格式的转义字符
#define REPORT_COMMON_TEMPLATE "{\"did\":\"\",\"datatype\":\"dictionary\",\"version\":\"\",\"cmdtype\":\"\",\"cid\":\"\",\"reporttime\":\"\"}"

#define REPORT_PHOTO_TEMPLATE "{\"did\":\"\",\"datatype\":\"dictionary\",\"version\":\"\",\"cmdtype\":\"cmd_takephoto\",\"cid\":\"\",\"filetype\":\"jpg\",\"filename\":\"SRC00000000000010_20230303142557\",\"reporttime\":\"\"}"
#define REPORT_DATA_TEMPLATE "{\"did\":\"\",\"cmdtype\":\"cmd_status\",\"datatype\":\"dictionary\",\"version\":\"\",\"photoname\":\"SRC00000000000010_20230303142557.jpg\",\"photourl\":\"\",\"ac\":\"\",\"alarm\":\"\",\"deviceid\":\"\",\"cid\":\"\",\"reporttime\":\"\"}"
#define REPORT_CONTROLLACK_TEMPLATE "{\"status\":\"success\",\"cmdtype\":\"cmd_controllack\",\"did\":\"\",\"reporttime\":\"\"}"

static String httpResponseString; //接收服务器返回信息


//默认的字符串定义
String fileName =  "20230312103012.jpg";
String lastphotofileName =  "20230312103012.jpg";
String reporttime = "2000-01-01 14:57:45";
boolean autoTakePhotoFlag = false;//拍照flag



