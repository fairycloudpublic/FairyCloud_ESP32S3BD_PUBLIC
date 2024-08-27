#include <Arduino.h>
#include <WiFi.h>
#include "PubSubClient.h"
#include <HTTPUpdate.h>
#include "esp_http_client.h"
#include "ArduinoJson.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"


#include "config.hpp"
#include "Utils.hpp"
LightUtils utils;



#include <TFT_eSPI.h> // ST7735驱动芯片的图形和字体库
#include <SPI.h>
 
TFT_eSPI tft = TFT_eSPI(); // 调用库，引脚在User_Setup.h中定义
 
void ftf_display(void);
void alarmAction(void);

JsonArray combineAutoArr ;
 StaticJsonDocument<1024*10> docauto;
 boolean combineAutoArrFlag = false;


//  监听需求总数
 int8_t demand_count = 0;
boolean getCombinemonitorFlag = false;


void combinecontrollFunc(JsonArray cmdcombine);
void combinemonitorFunc(JsonArray cmdcombine);
void getCombineAutoByCidHardware(String cid);
// 红外 自定义发射 
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Haier.h>

IRHaierACYRW02 haier_ac(kIrLed);

#include <string.h>
#define uint unsigned int

TaskHandle_t TASK_HandleOne = NULL;
TaskHandle_t TASK_HandleTwo = NULL;


//温湿度定义
 DHT11Struct dht11;

// delay for half a second
int delayval = 1000; 

// 重启尝试次数
int restartCount = 0; 


int showType = 0;


void videoUploadBegin(void);
void videoUploadStop(void);
void desetup_camera(void);
void setup_camera(void);
float getVoltage(int pin);
void send_fotamsg(String fotastatus ,String toastmsg ,String loading ,u8_t loadingmaxtime ,String progress);

//摄像头
#include "esp_camera.h"
#include "UdpClient.hpp"

#ifndef OVER_LOAD_CAM_CONF
// #define CAMERA_MODEL_AI_THINKER
#define FRAMESIZE FRAMESIZE_VGA
#define HE_ZHOU_S3
#endif

#include "cam_pins.h"


/*
   发送图片的UDP
  */
const uint16_t localUdpPort = 2333;
LightUDP streamSender;
 boolean videoUploadFlag = false;
 boolean videoUploadInitFlag = false;
 boolean cameraFlag = false;//视频开关状态


boolean esp_restartFlag = false;//是否重启设备

boolean autoUploadFlag = false;//是否自动上传视频的flag
int tim1_Photo_count = 0;//十分钟拍一张图片 10s一次，默认60次后恢复0

int fota_status = 0;//fota的状态
boolean fotaFlag = false;
String updateurl= "";
String updateversion= "";
int totalProgress = -1;


// 定时器初始化
hw_timer_t *tim1 = NULL;
int tim1_IRQ_count = 0;

//全局的TCP连接，在发送UDP的时候，可以检测TCP获取的ID是否变化，以防服务端挂了后客户端不知道
WiFiClient client;

PubSubClient mqttclient(client);

// #include "esp_task_wdt.h"
// #define TWDT_TIMEOUT_S  30


//-----------网络时间获取-----------//
#define NTP1 "ntp1.aliyun.com"
#define NTP2 "ntp2.aliyun.com"
#define NTP3 "ntp3.aliyun.com"

//时区设置函数，东八区（UTC/GMT+8:00）写成8*3600
const long gmtOffset_sec = 8 * 3600;    
const int daylightOffset_sec = 0;   //夏令时填写3600，否则填0




void wifiConnect(const char *wifiData[][2], int numNetworks)
{
    WiFi.disconnect(true);

    for (int i = 0; i < numNetworks; ++i)
    {
        const char *ssid = wifiData[i][0];
        const char *password = wifiData[i][1];

        Serial.print("Connecting to ");
        Serial.println(ssid);

        WiFi.begin(ssid, password);
        uint8_t count = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
            Serial.print(".");
            count++;
            if (count >= 6)
            {
                Serial.printf("\r\n-- wifi connect fail! --");
                break;
            }
            vTaskDelay(500);
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.printf("\r\n-- wifi connect success! --\r\n");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            Serial.println("Free Heap: " + String(ESP.getFreeHeap()));
            return; // 如果连接成功，退出函数
        }
    }



    
}

void getCombineAutoByCidHardware(String cid)
{

    /* code */
 
    HTTPClient http;

    String nonce = utils.md5str(reporttime +'0'+ random(1000));

   TimeStruct stc =  utils.setClock(SRCCID);

    int64_t signt = stc.timeStamp ;

    String did = utils.md5str(reporttime +'0'+ random(1000));

    String str5 = "api/getCombineAutoByCidHardware_appkey=" + appkey + "_cid=" + cid + "_did=" + did +"_nonce="+nonce+ "_signt="+(signt *1000) +"_"+ secretkey;

    String str6 = utils.urlEncode(str5);

    String sign =  utils.md5str(str6);
    
    String url = combinecontroll_url  + "?cid=" + cid + "&did="+did+ "&nonce="+nonce+ "&signt="+(signt *1000)+ "&appkey="+ appkey + "&sign="+sign;
    Serial.println(url);
    // Serial.print(url);

    http.begin(url); //访问服务器地址

    // Serial.print("[HTTP] GET...n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        // Serial.printf("[HTTP] GET... code: %dn", httpCode);

        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println(payload);

               
                deserializeJson(docauto, payload);

                boolean status = docauto["status"];
                if (status)
                {
                    combineAutoArr =  docauto["data"]["list"].as<JsonArray>();;

                    demand_count = combineAutoArr.size();
                    deviceStatus = "get combineauto ok";
                    
                    // 获取成功的flag
                    combineAutoArrFlag = true;
                    //自动拉取的flag
                    getCombinemonitorFlag = false;
                
                }
                   
        }else{
          Serial.printf("获取CID失败延时5s");
         
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %sn", http.errorToString(httpCode).c_str());
    }
    http.end();

}



void getDeviceByMac(String mac){

 uint8_t wifi_failedcount = 0;

  while (SRCCID == "")
  {
    /* code */
 
    HTTPClient http;

    String nonce = utils.md5str(reporttime +'0'+ random(1000));

   TimeStruct stc =  utils.setClock(SRCCID);

    int64_t signt = stc.timeStamp ;

    String did = utils.md5str(reporttime +'0'+ random(1000));

    String str5 = "api/getDeviceByMac_appkey=" + appkey+"_did="+ did+ "_mac=" + mac +"_nonce="+nonce+ "_signt="+(signt *1000)+"_version="+ version +"_"+ secretkey;

    String str6 = utils.urlEncode(str5);

    String sign =  utils.md5str(str6);
    
    String url = getsrccid_url  + "?mac="+mac+ "&version="+version+ "&did="+did+ "&nonce="+nonce+ "&signt="+(signt *1000)+ "&appkey="+ appkey + "&sign="+sign;
    Serial.println(url);
    // Serial.print(url);

    http.begin(url); //访问服务器地址

    // Serial.print("[HTTP] GET...n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        // Serial.printf("[HTTP] GET... code: %dn", httpCode);

        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println(payload);

                StaticJsonDocument<1024> doc;
                deserializeJson(doc, payload);

                boolean status = doc["status"];
                if (status)
                {
                    const char* cid =  doc["data"]["cid"];

                    String newcid = cid;
                    SRCCID = newcid.c_str();
                    Serial.printf(newcid.c_str());


                    const char* clientmqttClientId =  doc["data"]["mqttClientId"];
                    String newmqttClientId = clientmqttClientId;
                    mqtt_mqttClientId = newmqttClientId.c_str();

                    const char* clientusername =  doc["data"]["username"];
                    String newusername = clientusername;
                    mqtt_username = newusername.c_str();
                    
                    const char* clientpasswd =  doc["data"]["passwd"];
                    String newpasswd = clientpasswd;
                    mqtt_passwd = newpasswd.c_str();

                    const char* clientmqttHostUrl =  doc["data"]["mqttHostUrl"];
                    String newmqttHostUrl = clientmqttHostUrl;
                    mqtt_mqttHostUrl = newmqttHostUrl.c_str();
                    
                    mqtt_port =  doc["data"]["port"];

                    const char* clientudphost =  doc["data"]["udphost"];
                    String newudphost = clientudphost;
                    udp_host = newudphost.c_str();
                    
                    udp_port =  doc["data"]["udpport"];

                    const char* clientphotourl =  doc["data"]["photourl"];
                    String newphotourl = clientphotourl;
                    photourl = newphotourl.c_str();
                    
                    const char* clientmqtt_pub_topic =  doc["data"]["mqtt_pub_topic"];
                    String mqtt_pub_topic = clientmqtt_pub_topic;
                    mqtt_pub_topicsss = mqtt_pub_topic.c_str();
                    

                    const char* clientmqtt_sub_topic =  doc["data"]["mqtt_sub_topic"];
                    String mqtt_sub_topic = clientmqtt_sub_topic;
                    mqtt_sub_topicsss = mqtt_sub_topic.c_str();
                    
                    const char* clientcombinecontrollurl =  doc["data"]["combinecontrollurl"];
                    String combinecontrollurl = clientcombinecontrollurl;
                    combinecontroll_url = combinecontrollurl.c_str();
                    
                  
                }
                   
        }else{
          Serial.printf("获取CID失败延时5s");
         
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %sn", http.errorToString(httpCode).c_str());
    }
    http.end();
    // 延时3S继续获取
     if (SRCCID =="")
     {
       delay(3000);

        wifi_failedcount ++;
        if (wifi_failedcount >=5)
        {
          digitalWrite(alarm,HIGH);
          wifi_failedcount = 0;
        }

     }
     

  }


    wifi_failedcount = 0;
   digitalWrite(alarm,LOW);

}




//当升级开始时，打印日志
void update_started() {
  totalProgress= -1;
  send_fotamsg("fota_started","升级开始","show",5,"--");
  Serial.println("CALLBACK:  HTTP update process started");
}

//当升级结束时，打印日志
void update_finished() {
  totalProgress= -1;
    send_fotamsg("fota_finished","升级完成","hiden",0,"--");
  Serial.println("CALLBACK:  HTTP update process finished");
}

//当升级中，打印日志
void update_progress(int cur, int total) {
  float aa = (float)cur;
  float bb = (float)total;
    int grs = (int)(aa/bb *100);
    String progress = (String) grs ;

  if (  totalProgress != grs)
  {
    /* code */
        send_fotamsg("fota_progress","升级中","show",5,progress);

  }
      totalProgress = grs;

  
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...%s...%d... \n", cur, total,progress,grs);
}

//当升级失败时，打印日志
void update_error(int err) {
  totalProgress= -1;
  send_fotamsg("fota_error","FOTA升级失败","hiden",0,"--");
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void send_fotamsg(String fotastatus ,String toastmsg ,String loading ,u8_t loadingmaxtime ,String progress){

    StaticJsonDocument<1024> doc;
        deserializeJson(doc, REPORT_COMMON_TEMPLATE);
       // setClock();
        doc["cid"] = SRCCID;


        doc["datatype"] = "dictionary";
        doc["version"] = version;
        doc["cmdtype"] = "cmd_fotaack";
        doc["loadingmaxtime"] = loadingmaxtime;
        doc["toastmsg"] = toastmsg;
        doc["loading"] = loading ;
        doc["progress"] = progress;
        doc["fotastatus"] = fotastatus;
        doc["reporttime"] = reporttime;

        
                
    String nonce = utils.md5str(reporttime +'0'+ random(1000));
    doc["nonce"]= nonce;

      TimeStruct stc =  utils.setClock(SRCCID);

    int64_t signt = stc.timeStamp ;

       doc["signt"] = (signt *1000)  ;
    String cidstr = SRCCID;
    
    String did = utils.md5str(reporttime +'0'+ random(1000));
        doc["did"] =  did;




        char  buffer[1024];
        size_t n = serializeJson(doc, buffer);
        Serial.println(buffer);


    //   const char *mqtt_pub_topic = mqtt_pub_topicsss.c_str();

        if (mqttclient.publish( mqtt_pub_topicsss.c_str(), buffer,n)) {
            // Serial.printf("auto topic fota [%s] ok\n", mqtt_pub_topic);
            
        } else {
            Serial.printf("auto topic fota [%s] fail\n",  mqtt_pub_topicsss.c_str());
        }

}

void updateDevice(String upUrl,String updateversion){

  String ver = (String)version;
  String newver = (String)updateversion;
      Serial.println(ver); 
      Serial.println(newver); 

  if (ver == newver)
  {
      Serial.println("version equal "); 

       //FOTA开始的通知 mqtt 小程序提示
        send_fotamsg("fota_checked","已是最新版本","hiden",0,"--");

      return ;
  } else if ( upUrl == "")
  {

       //FOTA开始的通知 mqtt 小程序提示
        send_fotamsg("fota_error","FOTA升级地址错误","hiden",0,"--");

      Serial.println("url error"); 
      return ;
  }




  //FOTA开始的通知 mqtt 小程序提示
      
String toast = "";


  Serial.println("start update");    
  WiFiClient UpdateClient = client;
  
  //如果是旧版esp32 SDK，需要删除下面四行，旧版不支持，不然会报错
  httpUpdate.onStart(update_started);//当升级开始时
  httpUpdate.onEnd(update_finished);//当升级结束时
  httpUpdate.onProgress(update_progress);//当升级中
  httpUpdate.onError(update_error);//当升级失败时
  
  t_httpUpdate_return ret = httpUpdate.update(UpdateClient, upUrl);
  switch(ret) {
    case HTTP_UPDATE_FAILED:      //当升级失败
    toast = "FOTA升级失败";
        Serial.println("[update] Update failed.");
        break;
    case HTTP_UPDATE_NO_UPDATES:  //当无升级
     toast = "无FOTA升级";
        Serial.println("[update] Update no Update.");
        break;
    case HTTP_UPDATE_OK:         //当升级成功
      toast = "FOTA升级成功";
        Serial.println("[update] Update ok.");
        break;
  }

  //FOTA开始的通知 mqtt 小程序提示
      
}

 
/********http请求处理函数*********/
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    if (evt->event_id == HTTP_EVENT_ON_DATA)
    {
        httpResponseString.concat((char *)evt->data);
    }
    return ESP_OK;

}

/********推送图片*********/
static esp_err_t take_send_photo() {
  

  Serial.println("take_send_photo...");
  camera_fb_t* fb = NULL;
  esp_err_t res = ESP_OK;
  fb = esp_camera_fb_get();
  esp_camera_fb_return(fb);//重复利用一个缓存？？？
  fb = esp_camera_fb_get();
  esp_camera_fb_return(fb);//重复利用一个缓存？？？
  fb = esp_camera_fb_get();
  esp_camera_fb_return(fb);//重复利用一个缓存？？？
  fb = esp_camera_fb_get();
  esp_camera_fb_return(fb);//重复利用一个缓存？？？

  delay(50);
  fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Camera capture failed...");
    return ESP_FAIL;
  }


  httpResponseString = "";
  esp_http_client_handle_t http_client;
  esp_http_client_config_t config_client = { 0 };
  
  // String urls = (post_url + fileName).c_str();

    String nonce = utils.md5str(reporttime +'0'+ random(1000));

    int64_t signt= esp_timer_get_time(); //获取本地时间戳
    // printf("time cnt:%lld\r\n",signt); //打印时间戳
    
    String did = utils.md5str(reporttime +'0'+ random(1000));


    String str5 = "file/uploadImg_appkey=" + appkey+"_did="+ did+ "_fileName=" + fileName +"_nonce="+nonce+ "_signt="+signt+"_version="+ version +"_"+ secretkey;

    String str6 =  utils.urlEncode(str5);

    String sign =  utils.md5str(str6);

    String urls = photourl+ "?fileName="+fileName+ "&version="+version+ "&did="+did+ "&nonce="+nonce+ "&signt="+signt+ "&appkey="+ appkey + "&sign="+sign;
    Serial.print("post_url:"+photourl);



const char * url = urls.c_str();

   Serial.println(url);
  config_client.url = url;
  config_client.event_handler = _http_event_handler;
  config_client.method = HTTP_METHOD_POST;
  http_client = esp_http_client_init(&config_client);
  esp_http_client_set_post_field(http_client, (const char*)fb->buf, fb->len);  //设置http发送的内容和长度
  esp_http_client_set_header(http_client, "Content-Type", "image/jpg");        //设置http头部字段
//   esp_http_client_set_header(http_client, "Authorization", uid.c_str());               //设置http头部字段
//   esp_http_client_set_header(http_client, "Authtopic", topic);                 //设置http头部字段
//   esp_http_client_set_header(http_client, "wechatmsg", wechatMsg);             //设置http头部字段
//   esp_http_client_set_header(http_client, "wecommsg", wecomMsg);               //设置http头部字段
//   esp_http_client_set_header(http_client, "picpath", urlPath);                 //设置http头部字段
  esp_err_t err = esp_http_client_perform(http_client);                        //发送http请求
  if (err == ESP_OK) {
    Serial.println(httpResponseString);  //打印获取的URL
    //json数据解析
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, httpResponseString);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
    }
    boolean status = doc["status"];
    if (status)
    {
        // mqtt_username
        // SRCCID..'.' ..fileName

        // String sedStr = SRCCID+'.'+fileName;
        String ss = SRCCID;
        ss = ss +'.'+ fileName;
        Serial.print(ss.c_str());

        StaticJsonDocument<1024> doc;
        deserializeJson(doc, REPORT_PHOTO_TEMPLATE);
       // setClock();
        doc["cid"] = SRCCID;
        doc["filename"] = fileName;

        doc["datatype"] = "dictionary";
        doc["version"] = version;
        doc["reporttime"] = reporttime;

                
        TimeStruct stc =  utils.setClock(SRCCID);

    int64_t signt = stc.timeStamp ;

       doc["signt"] = (signt *1000)  ;
    String cidstr = SRCCID;
    
    String did = utils.md5str(reporttime +'0'+ random(1000));
        doc["did"] =  did;





        
        lastphotofileName = fileName;
        
        char  buffer[1024];
        size_t n = serializeJson(doc, buffer);
        Serial.println(buffer);

// const char *mqtt_pub_topic = mqtt_pub_topicsss.c_str();

        if (mqttclient.publish( mqtt_pub_topicsss.c_str(), buffer,n)) {
            Serial.printf("auto topic photo [%s] ok\n",  mqtt_pub_topicsss.c_str());
            
        } else {
            Serial.printf("auto topic photo [%s] fail\n",  mqtt_pub_topicsss.c_str());
        }

    }
    


    // Serial.println(doc);  //打印获取的URL
  }

  Serial.println("Taking picture END");
  esp_camera_fb_return(fb);
  esp_http_client_cleanup(http_client);


  return res;
}



// 获取通道ID和设置流媒体
void getChanneAndSetStreamSender(void) {

    streamSender.begin(WiFi.localIP(), localUdpPort);
    streamSender.setServer(udp_host.c_str(), udp_port);

}

// led alarm初始化
void cmd_GPIO_Init(){
    //设置io口为输出模式

    pinMode(alarm,OUTPUT);
    digitalWrite(alarm, LOW);

    pinMode(rc1,OUTPUT);
    digitalWrite(rc1, LOW);

        pinMode(rc2,OUTPUT);
    digitalWrite(rc2, LOW);

        pinMode(rc3,OUTPUT);
    digitalWrite(rc3, LOW);

        pinMode(rc4,OUTPUT);
    digitalWrite(rc4, LOW);
}




// 10s自动上传报文
void autoDataStatus(){
    unsigned char rc1_status = digitalRead(rc1) ;
    unsigned char rc2_status = digitalRead(rc2) ;
    unsigned char rc3_status = digitalRead(rc3) ;
    unsigned char rc4_status = digitalRead(rc4) ;

    unsigned char alarm_status = digitalRead(alarm) ;


    StaticJsonDocument<1024> doc;
    deserializeJson(doc, REPORT_DATA_TEMPLATE);

    TimeStruct stc =  utils.setClock(SRCCID);
    reporttime = stc.reporttime;
    fileName = stc.fileName;

    if (old_temp!=0 && old_humi !=0)
    {
          doc["temperature"] = old_temp;
          doc["humidity"] = old_humi;
    }
    
    // doc["temperature"] = 1.0*((int)(dht11.Temp*10))/10 + 1.0*dht11.Temp_small/10;//(String)Temp +"."+ (String)Temp_small ;
    // doc["humidity"] = 1.0*((int)(dht11.Humi*10))/10 + 1.0*dht11.Humi_small/10;//(String)Humi +"."+ (String)Humi_small ;
    doc["reporttime"] = reporttime;
    doc["alarm"] = alarm_status ? "open" : "close";
    doc["camera"] = cameraFlag ? "open" : "close";
    doc["ac"] = ac;
    doc["ac_temp"] = ac_temp;
    doc["ac_fan"] = ac_fan;
    
    doc["rc1"] = rc1_status ? "open" : "close";
    doc["rc2"] = rc2_status ? "open" : "close";
    doc["rc3"] = rc3_status ? "open" : "close";
    doc["rc4"] = rc4_status ? "open" : "close";
    

    doc["version"] = version;
    doc["demand"] = demand_count;

    

    doc["cid"] = SRCCID;
    doc["photoname"] = lastphotofileName;

    doc["airquality"] = old_airc;

    String cidstr = SRCCID;
    
    String did = utils.md5str(reporttime +'0'+ random(1000));
    doc["did"] =   did;



     char  buffer[1024];
     size_t n = serializeJson(doc, buffer);
  
    Serial.println(buffer);

    const char *mqtt_pub_topic = mqtt_pub_topicsss.c_str();
    if (mqttclient.publish( mqtt_pub_topicsss.c_str(),buffer,n)) {
        Serial.printf("auto topic [%s] ok\n",  mqtt_pub_topicsss.c_str());
                restartCount = 0;

    } else {
        Serial.printf("auto topic [%s] fail\n",  mqtt_pub_topicsss.c_str());
        restartCount++;

        //三次错误，就直接重启设备
        if (restartCount >=3)
        {
          alarmLevel = 3;
          restartCount = 0;
          Serial.printf("restarxxx3");
          // esp_restartFlag = true;//继续联网别重启
        }
    }

}

//中断服务函数
void tim1Interrupt()
{
    // Serial.println("void tim1Interrupt()");

    if (videoUploadFlag)
    {
        tim1_IRQ_count++;
    }else{
        
         tim1_IRQ_count = 0;
         tim1_Photo_count++;
    }
    
   

    timerAlarmEnabled(tim1);

    autoUploadFlag = true;


}

//定时器初始化
void timerInit(){

  tim1 = timerBegin(0, 80, true);
  timerAttachInterrupt(tim1, tim1Interrupt, true);
  timerAlarmWrite(tim1, 10000000ul, true);  //10s 10000000ul
  timerAlarmEnable(tim1);
  
}

// 0级别
void alarm_close(){
    digitalWrite(alarm,LOW);
}

// 1级别
void alarm_low(){

    digitalWrite(alarm,HIGH);
    delay(10);
    digitalWrite(alarm,LOW);
    delay(200);

}
// 2级别
void alarm_medium(){

    digitalWrite(alarm,HIGH);
    delay(100);
    digitalWrite(alarm,LOW);
    delay(100);

}

// 3级别
void alarm_height(){
    digitalWrite(alarm,HIGH);
}

// 告警执行
void alarmAction(){

  switch (alarmLevel)
  {
  case 3:
     alarm_height();
    break;
  case 2://wifi断开
     alarm_medium();
    break;
  case 1://4路继电器打开
     alarm_low();
    break;
  case 0://4路继电器打开
     alarm_close();
    break;
  default:
    break;
  }

}
//远控停止控制上传video
void videoUploadStop(){
    videoUploadFlag = false;
    desetup_camera();
    streamSender.stop();
}

//远控下发控制上传video
void videoUploadBegin(){
    videoUploadFlag = true;
    videoUploadInitFlag= true;

    // setup_camera();
    // getChanneAndSetStreamSender();


}


//自动化执行  10*1024内存 20条指令
void combinemonitorFunc(JsonArray cmdcombine){


    String demand_and= "demand_and";
    String demand_or= "demand_or";

    String compare_valuetype_String = "String";
    String compare_valuetype_Number = "Number";

    // 外设的值
    String compare_key_rc1 = "rc1";
    String compare_key_rc2 = "rc2";
    String compare_key_rc3 = "rc3";
    String compare_key_rc4 = "rc4";

    // 字段传感器的采集的值
    String compare_key_temperature = "temperature";
    String compare_key_humidity = "humidity";
    String compare_key_airquality = "airquality";

    // 运算符
    String compare_type_dy   = ">";
    String compare_type_dydy = ">=";
    String compare_type_hdy  = "=";
    String compare_type_xydy = "<=";
    String compare_type_xy   = "<";
    String compare_type_bdy  = "!=";

    // JsonArray array = doc.as<JsonArray>();
    for(auto dict : cmdcombine) {
        
        String autotype = dict["autotype"];
        
        JsonArray demandArr =  dict["demand"];
        JsonArray paramsArr =  dict["params"];
        
        // 数据运算比较
        if (autotype == demand_and)//与计算
        {
            // 是否执行的flag
            int8_t combineFlag = 0;

            for(JsonObject dict1 : demandArr) {
                String compare_key = dict1["compare_key"];
                String compare_type = dict1["compare_type"];
                String compare_valuetype = dict1["compare_valuetype"];
                auto compare_value = dict1["compare_value"];
            
                String compare_value_string = "";
                int    compare_value_number = 0;


                // 温度 
                if (compare_key == compare_key_temperature)
                {
                    compare_value_number = old_temp;

                } else if (compare_key == compare_key_humidity)
                {
                    compare_value_number = old_humi;

                } else if (compare_key == compare_key_airquality)
                {
                    compare_value_number = old_airc;

                } else if (compare_key == compare_key_rc1)
                {

                    compare_value_string = (old_rc1 == "I" ? "open":"close");

                } else if (compare_key == compare_key_rc2)
                {
                    compare_value_string = (old_rc2 == "I" ? "open":"close");

                } else if (compare_key == compare_key_rc3)
                {
                    compare_value_string = (old_rc3 == "I" ? "open":"close");

                } else if (compare_key == compare_key_rc4)
                {
                    compare_value_string = (old_rc4 == "I" ? "open":"close");

                }else
                {

                   Serial.printf("compare_key err:%s" , compare_key);

                }


                // 计算数据比较
                if (compare_type == compare_type_dy)
                {
                    if (compare_valuetype == compare_valuetype_String )
                    {
                        if (compare_value_number > compare_value)
                        {
                            combineFlag++;
                        }

                    }else if(compare_valuetype == compare_valuetype_Number )
                    {
                        if (compare_value_string > compare_value)
                        {
                            combineFlag++;
                        }
                    }else{
                        Serial.println("ERROR 3");
                    }


                }else if (compare_type ==  compare_type_dydy)
                {
                    if (compare_valuetype == compare_valuetype_String )
                    {
                        if ( compare_value_string >= compare_value)
                        {
  
                            combineFlag++;
                        }else{

                        }

                    }else if(compare_valuetype == compare_valuetype_Number )
                    {
                        if (compare_value_number >= compare_value)
                        {
                            combineFlag++;
                        }else{


                        }
                    }else{
                        Serial.println("ERROR 3");
                    }

                }else if (compare_type ==  compare_type_hdy)
                {
                    if (compare_valuetype == compare_valuetype_String )
                    {
                        if (compare_value_string == compare_value)
                        {
                            combineFlag++;
                        }

                    }else if(compare_valuetype == compare_valuetype_Number )
                    {
                        if ( compare_value_number == compare_value)
                        {
                            combineFlag++;
                        }
                    }else{
                        Serial.println("ERROR 3");
                    }

                }else if (compare_type ==  compare_type_xydy)
                {
                    if (compare_valuetype == compare_valuetype_String )
                    {
                        if (compare_value_string <= compare_value)
                        {
                            combineFlag++;
                        }

                    }else if(compare_valuetype == compare_valuetype_Number )
                    {
                        if (compare_value_number <= compare_value)
                        {
                            combineFlag++;
                        }
                    }else{
                        Serial.println("ERROR 3");
                    }

                }else if (compare_type ==  compare_type_xy)
                {
                    if (compare_valuetype == compare_valuetype_String )
                    {
                        if (compare_value_string < compare_value)
                        {
                            combineFlag++;
                        }

                    }else if(compare_valuetype == compare_valuetype_Number )
                    {
                        if (compare_value_number < compare_value)
                        {
                            combineFlag++;
                        }
                    }else{
                        Serial.println("ERROR 3");
                    }

                }else if (compare_type ==  compare_type_bdy)
                {
                    if (compare_valuetype == compare_valuetype_String )
                    {
                        if (compare_value_string != compare_value)
                        {
                            combineFlag++;
                        }

                    }else if(compare_valuetype == compare_valuetype_Number )
                    {
                        if (compare_value_number != compare_value)
                        {
                            combineFlag++;
                        }
                    }else{
                        Serial.println("ERROR 3");
                    }

                }else{


                   Serial.printf("compare_type55 err:%s" , compare_type);
            Serial.println(" ");

                }

            }

            // 最后是否执行的判断，是根据与全部满足执行
            if (combineFlag == demandArr.size())
            {
                // 触发执行
                combinecontrollFunc(paramsArr);               
            }else{
                 
            }





        }else if (autotype == demand_or)//或计算
        {
            // 是否执行的flag
            boolean combineFlag = false;

            for(JsonObject dict1 : demandArr) {
                String compare_key = dict1["compare_key"];
                String compare_type = dict1["compare_type"];
                String compare_valuetype = dict1["compare_valuetype"];
                auto compare_value = dict1["compare_value"];
            

                String compare_value_string = "";
                int    compare_value_number = 0;


                // 温度 
                if (compare_key == compare_key_temperature)
                {
                    compare_value_number = old_temp;

                } else if (compare_key == compare_key_humidity)
                {
                    compare_value_number = old_humi;

                } else if (compare_key == compare_key_airquality)
                {
                    compare_value_number = old_airc;

                } else if (compare_key == compare_key_rc1)
                {

                    compare_value_string = (old_rc1 == "I" ? "open":"close");

                } else if (compare_key == compare_key_rc2)
                {
                    compare_value_string = (old_rc2 == "I" ? "open":"close");

                } else if (compare_key == compare_key_rc3)
                {
                    compare_value_string = (old_rc3 == "I" ? "open":"close");

                } else if (compare_key == compare_key_rc4)
                {
                    compare_value_string = (old_rc4 == "I" ? "open":"close");

                }else
                {
                    Serial.println("ERROR 2");


                }


        // Serial.println(compare_type);

                // 计算数据比较
                if (compare_type == compare_type_dy)
                {
                    if (compare_valuetype == compare_valuetype_String )
                    {
                        if (compare_value_string > compare_value)
                        {
                            combineFlag = true;
                        }

                    }else if(compare_valuetype == compare_valuetype_Number )
                    {
                        if (compare_value_number > compare_value)
                        {
                            combineFlag = true;
                        }
                    }else{
                        Serial.println("ERROR 3");
                    }


                }else if (compare_type == compare_type_dydy)
                {
                    if (compare_valuetype == compare_valuetype_String )
                    {
                        if ( compare_value_string >= compare_value)
                        {
                            combineFlag = true;
                        }

                    }else if(compare_valuetype == compare_valuetype_Number )
                    {
                        if (compare_value_number >= compare_value)
                        {
                            combineFlag = true;
                        }
                    }else{
                        Serial.println("ERROR 3");
                    }

                }
                else if (compare_type == compare_type_hdy)
                {
                    if (compare_valuetype == compare_valuetype_String )
                    {
                        if (compare_value_string == compare_value)
                        {
                            combineFlag = true;
                        }

                    }else if(compare_valuetype == compare_valuetype_Number )
                    {
                        if (compare_value_number == compare_value)
                        {
                            combineFlag = true;
                        }
                    }else{
                        Serial.println("ERROR 3");
                    }

                }else if (compare_type == compare_type_xydy)
                {
                    if (compare_valuetype == compare_valuetype_String )
                    {
                        if (compare_value_string <= compare_value)
                        {
                            combineFlag = true;
                        }

                    }else if(compare_valuetype == compare_valuetype_Number )
                    {
                        if ( compare_value_number <= compare_value)
                        {
                            combineFlag = true;
                        }
                    }else{
                        Serial.println("ERROR 3");
                    }

                }else if (compare_type == compare_type_xy)
                {
                    if (compare_valuetype == compare_valuetype_String )
                    {
                        if (compare_value_string < compare_value)
                        {
                            combineFlag = true;
                        }

                    }else if(compare_valuetype == compare_valuetype_Number )
                    {
                        if (compare_value_number < compare_value)
                        {
                            combineFlag = true;
                        }
                    }else{
                        Serial.println("ERROR 3");
                    }

                }else if (compare_type == compare_type_bdy)
                {
                    if (compare_valuetype == compare_valuetype_String )
                    {
                        if (compare_value_string != compare_value)
                        {
                            combineFlag = true;
                        }

                    }else if(compare_valuetype == compare_valuetype_Number )
                    {
                        if (compare_value_number != compare_value)
                        {
                            combineFlag = true;
                        }
                    }else{
                        Serial.println("ERROR 3");
                    }

                }else{

                    Serial.println("ERROR 2");

                }


                // 最后是否执行的一个flag
                if (combineFlag)
                {
                    // 触发执行
                    combinecontrollFunc(paramsArr);
                    // 跳出当前这一个执行，进行下一个判断
                    break;
                }



            }



        }else{
                   Serial.printf("autotype err:%s" , autotype);
            Serial.println("-");

        }



    }



} 

//一键执行  2*1024内存 20条指令
void combinecontrollFunc(JsonArray cmdcombine){

    String exe_controll= "exe_controll";
    String exe_delay= "exe_delay";

     Serial.println("combinecontroll begin");
     Serial.println(cmdcombine.size());
     Serial.println("combinecontroll end");

    // JsonArray array = doc.as<JsonArray>();
    for(JsonObject dict : cmdcombine) {
        String cmdtype =  dict["cmdtype"];
        Serial.println("cmdtype:"+cmdtype);

        if (cmdtype == exe_controll)
        {
            auto  cmddata = dict["cmddata"];
            String sensorname = cmddata["sensorname"];
            String sensorcmd= cmddata["sensorcmd"];
            Serial.println(sensorname);
            Serial.println(sensorcmd);

            if (sensorname == "rc1"){
                if(sensorcmd == "open")
                {
                    digitalWrite(rc1,HIGH);
                }else if(sensorcmd == "close")
                {
                    digitalWrite(rc1,LOW);
                }else{}

            }else if (sensorname == "rc2"){
                  if(sensorcmd == "open")
                    {
                        digitalWrite(rc2,HIGH);

                    }else if(sensorcmd == "close")
                    {
                        digitalWrite(rc2,LOW);

                    }else{
                    }

            }else if (sensorname == "rc3"){
                  if(sensorcmd == "open")
                    {
                        digitalWrite(rc3,HIGH);

                    }else if(sensorcmd == "close")
                    {
                        digitalWrite(rc3,LOW);

                    }else{
                    }

            }else if (sensorname == "rc4"){
                  if(sensorcmd == "open")
                    {
                        digitalWrite(rc4,HIGH);

                    }else if(sensorcmd == "close")
                    {
                        digitalWrite(rc4,LOW);

                    }else{
                    }

            }else if (sensorname == "alarm"){
                  if(sensorcmd == "open")
                    {
                        digitalWrite(alarm,HIGH);

                    }else if(sensorcmd == "close")
                    {
                        digitalWrite(alarm,LOW);

                    }else{
                    }

            }else if (sensorname == "exe_alarm1"){
                  if(sensorcmd == "open")
                    {
                        digitalWrite(alarm,HIGH);
                        delay(100);
                        digitalWrite(alarm,LOW);


                    }else if(sensorcmd == "close")
                    {
                        digitalWrite(alarm,LOW);

                    }else{
                    }

            }else if (sensorname == "exe_alarm2"){
                  if(sensorcmd == "open")
                    {
                        digitalWrite(alarm,HIGH);
                        delay(100);
                        digitalWrite(alarm,LOW);
                        delay(100);
                      digitalWrite(alarm,HIGH);
                        delay(100);
                        digitalWrite(alarm,LOW);

                    }else if(sensorcmd == "close")
                    {
                        digitalWrite(alarm,LOW);

                    }else{
                    }

            }else{
                 Serial.println("unknown sensorname");

            }


        }else if (cmdtype == exe_delay)
        {
            auto  cmddata = dict["cmddata"];
            int timevalue = cmddata["timevalue"];
            Serial.printf("timevalue:%d" ,timevalue);
            delay(timevalue);

        }
        
    }

} 

void callback(char* topic, byte* payload, unsigned int length)
{
    Serial.println(topic);    

    //json数据解析
    StaticJsonDocument<1024*2> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
    }
    
    String did= doc["did"];

    // 远控类型
    String cmdType = doc["cmdtype"];



    String cmd_combinecontroll= "cmd_combinecontroll";
    String cmd_combinemonitor= "cmd_combinemonitor";
    String cmd_controll= "cmd_controll";
    String cmd_status= "cmd_status";
    String cmd_statusack= "cmd_statusack";

    String cidstr = SRCCID ;
        
    String signt = doc["signt"]; 
    

    if (cmdType == cmd_controll)
    {
        // 远控命令
        Serial.println("cmd_controll");
        auto  cmddata = doc["cmddata"];
        String sensorname = cmddata["sensorname"];
        String sensorcmd= cmddata["sensorcmd"];

        if (sensorname == "takephoto")
        {
            Serial.println("takephoto");
            if (sensorcmd == "open")
            {
                Serial.println("takephoto");
                 autoTakePhotoFlag = true;
                
            }else{
                Serial.println(sensorcmd);
            }
        }else if (sensorname == "camera")
        {
            Serial.println("camera");
             if(sensorcmd == "open")
            {
                Serial.println("open");
                cameraFlag = true;
                autoUploadFlag = true;
                videoUploadBegin();

            }else if(sensorcmd == "close")
            {
                Serial.println("close");
                cameraFlag = false;
                autoUploadFlag = true;
                 videoUploadStop();

            }else{
                Serial.println(sensorcmd);
            }
        }else if (sensorname == "rc1"){
             Serial.println(sensorname);
              if(sensorcmd == "open")
                {
                    Serial.println("open");
                    digitalWrite(rc1,HIGH);
                      autoUploadFlag = true;

                }else if(sensorcmd == "close")
                {
                    Serial.println("close");
                    digitalWrite(rc1,LOW);
                      autoUploadFlag = true;

                }else{
                    Serial.println(sensorcmd);
                }

        }else if (sensorname == "rc2"){
             Serial.println(sensorname);
              if(sensorcmd == "open")
                {
                    Serial.println("open");
                    digitalWrite(rc2,HIGH);
                      autoUploadFlag = true;

                }else if(sensorcmd == "close")
                {
                    Serial.println("close");
                    digitalWrite(rc2,LOW);
                      autoUploadFlag = true;

                }else{
                    Serial.println(sensorcmd);
                }

        }else if (sensorname == "rc3"){
             Serial.println(sensorname);
              if(sensorcmd == "open")
                {
                    Serial.println("open");
                    digitalWrite(rc3,HIGH);
                      autoUploadFlag = true;

                }else if(sensorcmd == "close")
                {
                    Serial.println("close");
                    digitalWrite(rc3,LOW);
                      autoUploadFlag = true;

                }else{
                    Serial.println(sensorcmd);
                }

        }else if (sensorname == "rc4"){
             Serial.println(sensorname);
              if(sensorcmd == "open")
                {
                    Serial.println("open");
                    digitalWrite(rc4,HIGH);
                      autoUploadFlag = true;

                }else if(sensorcmd == "close")
                {
                    Serial.println("close");
                    digitalWrite(rc4,LOW);
                      autoUploadFlag = true;

                }else{
                    Serial.println(sensorcmd);
                }

        }else if (sensorname == "alarm"){
             Serial.println(sensorname);
              if(sensorcmd == "open")
                {
                    Serial.println("open");
                    alarmLevel = 3;
                    // digitalWrite(alarm,HIGH);
                      autoUploadFlag = true;

                }else if(sensorcmd == "close")
                {
                    Serial.println("close");
                    alarmLevel = 0;
                    // digitalWrite(alarm,LOW);
                      autoUploadFlag = true;

                }else{
                    Serial.println(sensorcmd);
                }

        }else if (sensorname == "ac"){//控制空调
             Serial.println(sensorname);
              if(sensorcmd == "open")
                {
                    Serial.println("open");

                    uint8_t ac_mode = 1;//制热制冷-1制冷 4制热
                    
                    String ac_action = "";//下发的命令
                    String ac_action_cmd = "";//调高

                    // 空调的扩展指令 Temp温度 Mode加热/制冷--默认制冷 Fan风力--默认5auto
                    auto  extdata = cmddata["extdata"];

                    if (extdata.containsKey("mode")) {
                        ac_mode = extdata["mode"];//制热制冷-1制冷 4制热
                    } else {
                      Serial.println("mode does not exist");
                    }

                    if (extdata.containsKey("fan")) {
                        ac_fan = extdata["fan"];//风力 5auto默认  3low 2 med 1high
                    }  else {
                      Serial.println("fan does not exist");
                    }

                    if (extdata.containsKey("temp")) {
                        ac_temp =  extdata["temp"];//温度
                    }  else {
                      Serial.println("temp does not exist");
                    }
                    
                    //扩展指令类型 action 用于判断指令具体作用是 调整还是打开 
                    if (extdata.containsKey("action")) {
                        String ac_actioncc =  extdata["action"];
                        ac_action = (String)ac_actioncc;
                    }  else {
                      Serial.println("action does not exist");
                    }

                    if (extdata.containsKey("action_cmd")) {
                        String ac_action_cmdcc =  extdata["action_cmd"];
                        ac_action_cmd = (String)ac_action_cmdcc;

                    }  else {
                      Serial.println("action_cmd does not exist");
                    }

                    Serial.println("ac_mode:"+ ac_mode);
                    Serial.println("ac_fan:"+ ac_fan);
                    Serial.println("ac_temp:"+ ac_temp);
                    Serial.println("ac_action:"+ ac_action);
                    Serial.println("ac_action_cmd:"+ ac_action_cmd);
                    

                    //打开和修改都公用的指令

                        // 参数方式
                        //打开空调-默认
                        haier_ac.on();
                        //设置模式  (1) V9014557 Remote in "A" setting. (Default)
                        haier_ac.setModel(V9014557_A);
                        // setPower
                        haier_ac.setPower(1);
                        //设置按钮 5是电源
                        haier_ac.setButton(5);
                        //制热制冷-1制冷 4制热
                        haier_ac.setMode(ac_mode);
                        //设置温度
                        haier_ac.setTemp(ac_temp);
                        // 设置风扇 风力 5auto默认  3low 2 med 1high
                        haier_ac.setFan(ac_fan);

                        //设置强劲模式(涡轮增压) 若传入的是on则Quiet = false;
                        haier_ac.setTurbo(false);
                        //设置静音 若传入的是on则 Turbo = false;
                        haier_ac.setQuiet(false);

                        //垂直摆动模式 Set the Vertical Swing mode of the A/C.
                        haier_ac.setSwingV(2);//Middle 2
                        // 水平摆动模式 Set the Horizontal Swing mode of the A/C.
                        haier_ac.setSwingH(0);//Middle 0
                        // Set the Sleep setting of the A/C.
                        haier_ac.setSleep(false);
                        // Set the Health (filter) setting of the A/C.
                        haier_ac.setHealth(true);

                        //设置定时开启/关闭空调 Set the Timer operating mode.  0关闭
                        haier_ac.setTimerMode(0);
                        //设置开启时间 Set the number of minutes of the On Timer setting.
                        /// @param[in] mins Nr. of Minutes for the Timer. `0` means disable the timer.
                        haier_ac.setOnTimer(0);
                        //设置关闭时间 Set the number of minutes of the Off Timer setting.
                        // @param[in] mins Nr. of Minutes for the Timer. `0` means disable the timer.
                        haier_ac.setOffTimer(0);

                        //设置锁定 Set the Lock setting of the A/C.
                        haier_ac.setLock(false); 


                    //进行调整空调文档数据  根据扩展指令中的action字段，判断是：后续调整、第一次打开
                    if (ac_action == "change")
                    {
                        Serial.println("change okxxxxxxxxxxxxx");

                        // 如果有temp fan直接使用此值，如果没有temp则直接进行加/减1
                        if (extdata.containsKey("temp")) {
                            Serial.println("temp is exist 不需要操作 ");
                        }  else if (extdata.containsKey("fan")) {
                            Serial.println("fan is exist 不需要操作 ");
                        }  else{
                            //本地记时间，进行加减操作
                            Serial.println("temp does not exist");
                            if (ac_action_cmd == "temp_height")
                            {
                            Serial.println("height okxxxxxxxxxxxxx");

                              haier_ac.setButton(0); //0 是新增  1是减少
                                // ac_temp =  haier_ac.getTemp();  
                                //设置温度
                                ac_temp = ac_temp + 1;
                                haier_ac.setTemp(ac_temp);                          
                            }else if (ac_action_cmd == "temp_low") {

                              Serial.println("low okxxxxxxxxxxxxx");
                              haier_ac.setButton(1); //0 是新增  1是减少
                                // ac_temp =  haier_ac.getTemp();  
                                //设置温度
                                ac_temp = ac_temp - 1;
                                haier_ac.setTemp(ac_temp); 
                            }else if (ac_action_cmd == "fan_height") {  //风力三个档位 3最小 2居中 1最大

                              Serial.println("low okxxxxxxxxxxxxx");
                              if (ac_fan == 5 )//自动模式
                              {
                                ac_fan = 3;
                              }else if (ac_fan == 3)
                              {
                                ac_fan = 2;
                              }else if (ac_fan == 2)
                              {
                                ac_fan = 1;
                              }else  
                              {
                                ac_fan = 1;//最大
                              }
                              
                              haier_ac.setFan(ac_fan); //3最小 2居中 1最大

                            }else if (ac_action_cmd == "fan_low") {

                              Serial.println("low okxxxxxxxxxxxxx");
                               if (ac_fan == 1 )
                              {
                                ac_fan = 2;
                              }else if (ac_fan == 2)
                              {
                                ac_fan = 3;
                              }else if (ac_fan == 3)
                              {
                                ac_fan = 5;
                              }else  
                              {
                                ac_fan = 5;//自动模式
                              }
                              
                              haier_ac.setFan(ac_fan); //3最小 2居中 1最大

                            }
                            
                        }
                        Serial.println("send okxxxxxxxxxxxxx");

                        //最后 发送红外
                        haier_ac.send();

                    }else if (ac_action == "open"){
                      //默认的打开空调
                        Serial.println("ac_action open okxxxxxxxxxxxxx");

                      

                        // 发射指令
                        Serial.println("a power_on capture from IRrecvDumpV2");
                        //指令方式
                        // irsend.sendRaw(power_on, 229, 38);  // Send a raw data capture at 38kHz.

                        //设置温度
                        haier_ac.setTemp(ac_temp);
                        haier_ac.setFan(ac_fan);
                       
                        //最后 发送红外
                        haier_ac.send();

                    }else{
                        Serial.println("ac_action err!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                    }



                    // 修改空调flag状态
                    ac = "open";
                    autoUploadFlag = true;

                }else if(sensorcmd == "close")
                {
                    Serial.println("close");
                    Serial.println("a power_on capture from IRrecvDumpV2");
                    // irsend.sendRaw(power_off, 229, 38);  // Send a raw data capture at 38kHz.
                    haier_ac.off();
                    haier_ac.send();
                    ac = "close";
                      autoUploadFlag = true;

                }else{
                    Serial.println(sensorcmd);
                }

        }else if (sensorname == "restart"){
            Serial.println(sensorname);

            if(sensorcmd == "open")
            {
                Serial.println("open");
                autoUploadFlag = true;
                esp_restartFlag = true;

            }else{
                Serial.println(sensorcmd);
            }


        }else if (sensorname == "status"){
            Serial.println(sensorname);

            if(sensorcmd == "open")
            {
                Serial.println("open");
                //  -- 基础数据查询
                autoUploadFlag = true;

            }else{
                Serial.println(sensorcmd);
            }


        }else if (sensorname == "fota"){
            Serial.println(sensorname);

            if(sensorcmd == "open") 
            {
              updateversion = "";
              updateurl = "";


              Serial.println("open");
              //  -- 远程更新FOTA
              auto  extdata = cmddata["extdata"];
              String updateurlCC= extdata["updateurl"];
              updateurl = updateurlCC;
              String updateversions =  extdata["updateversion"];
              Serial.println(updateurl +":"+ updateversions);
              updateversion = updateversions;

               fotaFlag = true;
               fota_status = 1;

            }else{
                Serial.println(sensorcmd);
            }

        }else if (sensorname == "combinemonitor"){
            Serial.println(sensorname);

            if(sensorcmd == "open")
            {
                Serial.println("open");
                getCombinemonitorFlag = true;

            }else{
                Serial.println(sensorcmd);
            }


        }else{
             Serial.println(sensorname);
        }
        
        
    }else if (cmdType == cmd_status){
         //  -- 基础数据查询
        Serial.println("cmd_status");
        autoUploadFlag = true;

    }else if (cmdType == cmd_combinecontroll){
         //  -- 一键下发
        Serial.println("cmd_combinecontroll");
        auto params = doc["cmdcombine"]["params"];//数组
        
        JsonArray cmdcombine = params.as<JsonArray>();

        autoUploadFlag = true;
        combinecontrollFunc(cmdcombine); //2*1024内存 20条指令

    }else if (cmdType == cmd_combinemonitor){
         //  -- 自动化执行
        Serial.println("cmd_combinemonitor");
        getCombinemonitorFlag = true;
        autoUploadFlag = true;


        // combineAutoArrFlag = false;
        // DeserializationError error = deserializeJson(docauto, orinalpayload, length);
        // if (error) {
        //     Serial.print(F("deserializeJson() failed: "));
        //     Serial.println(error.c_str());
        // }
        // JsonArray cmdcombine = docauto["cmdcombine"].as<JsonArray>();
        // combineAutoArr = cmdcombine;//数组
        // autoUploadFlag = true;
        // combineAutoArrFlag = true;


    }else if (cmdType == cmd_statusack){
        //远控的ACK返回
        Serial.println("cmd_statusack");
        String reporttime = doc["reporttime"];
        old_reporttime = reporttime;

    }else{
        //其他类型
        Serial.println("ERROR 00");
        Serial.println(cmdType.c_str());
        Serial.println("ERROR 01");

    }

    // 返回远控的ACK消息
    if (cmdType != cmd_statusack && cmdType != cmd_status)
    {
       StaticJsonDocument<1024> docack;
        deserializeJson(docack, REPORT_CONTROLLACK_TEMPLATE);
           TimeStruct stc =  utils.setClock(SRCCID);
          reporttime = stc.reporttime;
          fileName = stc.fileName;

        docack["did"] =  did;
        docack["datatype"] =  "dictionary";
        docack["cid"] = SRCCID;

        docack["reporttime"] = reporttime;
        docack["version"] = version;


        
       
        int64_t timer1_cnt=esp_timer_get_time(); //获取本地时间戳
        // printf("time cnt:%lld\r\n",timer1_cnt); //打印时间戳

        docack["signt"] = timer1_cnt;
        String cidstr = SRCCID;
       
        
        char  buffer[1024];
        size_t n = serializeJson(docack, buffer);
        // Serial.println(buffer);

        // const char *mqtt_pub_topic = mqtt_pub_topicsss.c_str();
        if (mqttclient.publish( mqtt_pub_topicsss.c_str(), buffer,n)) {
            Serial.printf("auto topic ack [%s] ok\n",  mqtt_pub_topicsss.c_str());
            
        } else {
            Serial.printf("auto topic ack [%s] fail\n",  mqtt_pub_topicsss.c_str());
        }

    }

       

}


//摄像头卸载退出
void desetup_camera(){
    // camera deinit 
    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK) {
        log_i("Camera deinit failed with error 0x%x", err);
        return;
    }else{
        log_i("esp_camera_deinit ok");
    }

}

//摄像头初始化
void setup_camera(){
    
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 10;
    config.fb_count = 1;

    if (psramFound()) {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 3;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
        // 使用直接内存申请
        config.frame_size = FRAMESIZE_SVGA;//FRAMESIZE_QVGA 
        config.fb_location = CAMERA_FB_IN_DRAM;
    }


    // camera init 
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        log_i("Camera init failed with error 0x%x", err);
        return;
    }
    log_i("get sensor ");
    sensor_t *s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE);

}

// void wdt_init(){
//     rtc_wdt_protect_off();     //看门狗写保护关闭 关闭后可以喂狗
//     //rtc_wdt_protect_on();    //看门狗写保护打开 打开后不能喂狗
//     //rtc_wdt_disable();       //禁用看门狗
//     rtc_wdt_enable();          //启用看门狗
//     rtc_wdt_set_time(RTC_WDT_STAGE0, 30000); // 设置看门狗超时 8000ms.则reset重启
// }

#include <esp32-hal.h>
#include <lwip/apps/sntp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief 设置时区和NTP服务器和并校对时间
 * @param [in] posix_tz 表示posix规范的时间戳。例如"UTC-8"表示东八区（北京时间）
 * @param [in] server_cnt NTP服务器的数量，超过三个的会被抛弃
 * @param [in] ... NTP服务器，C风格字符串指针类型，数量应与server_cnt对应
 * @attention 函数不会保证WiFi连接正常
 * @example set_time("UTC-8", 3, "time2.cloud.tencent.com", "ntp1.aliyun.com", "ntp.ntsc.ac.cn");
 * @example set_time("UTC+6", 2, "time2.cloud.tencent.com", "192.168.0.2");
 */
void set_time(const char *const posix_tz, const int server_cnt,const char* server1, const char* server2, const char* server3)
{
	sntp_setoperatingmode(SNTP_OPMODE_POLL);

    sntp_setservername(0, (char*)server1);
    sntp_setservername(1, (char*)server2);
    sntp_setservername(2, (char*)server3);

	sntp_init();
	setenv("TZ", posix_tz, 1);
	tzset();
	while (time(NULL) < 1580000000) // 阻塞，直到时间同步
		yield();
	sntp_stop();
}



void tft_setup(){
    tft.init();
    tft.setRotation(1);

}


void ftf_display()
{
 
    // 用灰色填充屏幕，以便我们可以看到带有和不带有背景颜色定义的打印效果
    tft.fillScreen(TFT_BLACK);
 
    // 将“光标”设置在显示器的左上角(0,0)，并选择字体2
    // （在使用'tft.println'打印时，光标将自动移动到下一行
    // 或者如果有足够的空间，光标将停留在同一行上）
    tft.setCursor(0, 0, 2);
 
    // 将字体颜色设置为白色，背景为黑色，将文本大小乘数设置为1
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    // 现在我们可以使用“print”类在屏幕上绘制文本
    tft.println(old_reporttime);

    // 设备状态
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);
    tft.print("status:  ");
    tft.println(deviceStatus);
    //  tft.println("");

    // 将字体颜色设置为黄色，没有背景，设置为字体7
    // tft.setTextColor(TFT_WHITE, TFT_BLACK);
    // tft.setTextFont(2);
    // tft.println("Temp: 36℃    Humi: 78%");
 
    // 将字体颜色设置为红色，背景为黑色，设置为字体4
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
   
    // 温度
    tft.setTextFont(4);
    tft.print("T:");
    tft.print(old_temp);
    tft.print("'C");

  // 湿度
    tft.print("  H:");
    tft.print(old_humi);
     tft.println("%");

//  空气质量

    tft.print("A:");
    tft.print(old_airc);
     tft.println("ppm");

    // tft.println("Alarm: close");
     tft.print("RC1:");
     tft.print(old_rc1);

     tft.print("  RC2:");
     tft.println(old_rc2);

     tft.print("RC3:");
     tft.print(old_rc3);
     tft.print("  RC4:");
     tft.print(old_rc4);
    // 测试一些打印格式化函数
    // float fnumber = 123.45;
    // 将字体颜色设置为蓝色，没有背景，设置为字体2
    // tft.setTextColor(TFT_WHITE, TFT_BLACK);
    // tft.setTextFont(2);
    // tft.print("Float = ");
    // tft.println(fnumber); // 打印浮点数
    // tft.print("Binary = ");
    // tft.println((int)fnumber, BIN); // 以二进制形式打印为整数值
    // tft.print("Hexadecimal = ");
    // tft.println((int)fnumber, HEX); // 以十六进制形式打印为整数值
 
   
}

void  TaskDisplay(void *param){

  uint32_t blink_delay = 900;
// for死循环
  while (1)
  {
    
    delay(blink_delay);
    unsigned char rc1_status = digitalRead(rc1) ;
    unsigned char rc2_status = digitalRead(rc2) ;
    unsigned char rc3_status = digitalRead(rc3) ;
    unsigned char rc4_status = digitalRead(rc4) ;
    unsigned char alarm_status = digitalRead(alarm) ;
  
    uint c_temp = 1.0*((int)(dht11.Temp*10))/10 + 1.0*dht11.Temp_small/10;
    uint c_humi = 1.0*((int)(dht11.Humi*10))/10 + 1.0*dht11.Humi_small/10;

    uint  c_airc =   1.0*((int)(getVoltage(gasSensor)*10))/10;
   
   
    // 空气质量值变化，马上上报
    if ( c_airc != old_airc )
    {
      autoUploadFlag = true;
    }

    old_airc = c_airc;

     // 温湿度值变化，马上上报
      if (  old_temp != c_temp  || old_humi != c_humi)
      {
        autoUploadFlag = true;
      }
      
      old_temp = c_temp;
      old_humi = c_humi;



    // 设备有打开任何外设时，设备进行告警播报1S一次
    // if (rc1_status || rc2_status || rc3_status || rc4_status)
    // {
    //     alarmLevel = 1; //告警1

    // }else{
    //     alarmLevel = 0; //告警消除
    // }
    
    
    old_rc1 = rc1_status ? "I" : "O";
    old_rc2 = rc2_status ? "I" : "O";
    old_rc3 = rc3_status ? "I" : "O";
    old_rc4 = rc4_status ? "I" : "O";


   if (getCombinemonitorFlag)
   {
        getCombineAutoByCidHardware(SRCCID);

   }
   

    // 监控实时数据
   if (combineAutoArrFlag)
   {
    // Serial.println("--------------------------begin---------------------------");
        if (demand_count>0)
        {
            deviceStatus = "demand [ " + (String)demand_count + " ]";
        }
        

        combinemonitorFunc(combineAutoArr);
    // Serial.println("--------------------------end---------------------------");

   }

   

    ftf_display();
    alarmAction();
  }
  



}
void setup() {
  // 屏幕初始化

   tft_setup();

  xTaskCreate(
    TaskDisplay       // 这个任务运行的函数
    ,  "Task Display" //  给人看的名字
    ,  8*1024        // 任务栈的大小，用于存储任务运行时的上下文信息。简单来说，就是最多存这么多信息
    ,  NULL // 任务参数。要么没有填NULL；要么必须为无类型指针
    ,  0  // 优先级
    ,  NULL // 任务的句柄，用于管理和控制任务，NULL相当于0，意味着此处不需要任务句柄
    );

  deviceStatus = "init Serial";


    // 红外发射初始化V2
    // 必须一开始就初始化，不然红外灯发热厉害
    // haier_ac.begin();


    Serial.begin(115200);
    Serial.setDebugOutput(true);
    while (!Serial) {
        /* code */
    }
  deviceStatus = "init GPIO";


    cmd_GPIO_Init();


  deviceStatus = "connect wifi";

    
    // utils.setup_wifi(ssid, passwd);
    int numNetworks = sizeof(wifiData) / sizeof(wifiData[0]);
    utils.setup_wifi(wifiData, numNetworks);

    configTime(8 * 3600, 0, NTP1, NTP2, NTP3);
    
    // set_time("UTC-8", 3, "time2.cloud.tencent.com", "ntp1.aliyun.com", "ntp.ntsc.ac.cn");

    String mac = utils.getmac();

    deviceStatus = "get device info";

    getDeviceByMac(mac);
    getCombineAutoByCidHardware(SRCCID);


    deviceStatus = "mqtt connecting";


    // connect mqtt server
    mqttclient.setServer(mqtt_mqttHostUrl.c_str(), mqtt_port);
    mqttclient.setCallback(callback);
    mqttclient.setKeepAlive(60);
    while (!mqttclient.connect(mqtt_mqttClientId.c_str(), mqtt_username.c_str(), mqtt_passwd.c_str())) {
        Serial.println("mqtt connect fail, reconnect");
          deviceStatus = "mqtt connect err";

            int numNetworks = sizeof(wifiData) / sizeof(wifiData[0]);
            utils.setup_wifi(wifiData, numNetworks);

    }

    Serial.println("mqtt connected!");
    
    deviceStatus = "mqtt connected!";

    // sub topic

    boolean ret = mqttclient.subscribe(mqtt_sub_topicsss.c_str());
    if (ret != true) {
        Serial.printf("mqtt subscribe topic [%s] fail\n", mqtt_sub_topicsss.c_str());
    }
    Serial.printf("mqtt subscribe topic [%s] ok\n", mqtt_sub_topicsss.c_str());


    // wdt_init();
    // Serial.printf("wdt_init\n");
    



    //定时器
    timerInit();

}




void reconnect() {
      deviceStatus = "reconnect mqtt";

  while (!mqttclient.connected()) {

      alarmLevel = 3; //告警
      // alarmAction();

// 重新打开wifi链接、
    Serial.println("reset wifi ...");
    
    int numNetworks = sizeof(wifiData) / sizeof(wifiData[0]);
    utils.setup_wifi(wifiData, numNetworks);

    
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttclient.connect(mqtt_mqttClientId.c_str(), mqtt_username.c_str(), mqtt_passwd.c_str())) {
      Serial.println("connected");

      alarmLevel = 0; //告警消除
      
      // 连接成功时订阅主题
    deviceStatus = "connect mqtt ok";

    // const char *mqtt_sub_topic =  mqtt_sub_topicsss.c_str();
    mqttclient.subscribe(mqtt_sub_topicsss.c_str());

    autoUploadFlag = 1;

      
    } else {

      Serial.print("failed, rc=");
      Serial.print(mqttclient.state());
      Serial.println(" try again in 3 seconds");
      // Wait 5 seconds before retrying
      delay(3000);
    }
  }
}




/*MQ135空气质量检测传感器模块（有害物体 氨气 硫化物检测)
 程序之一
 */

//   float voltage;  voltage = getVoltage(gasSensor);
// 15往上报警
 float getVoltage(int pin){

  return (analogRead(pin) * 0.004882814); 
 }




void loop() {

    Serial.println("do loop");

    camera_fb_t *fb = NULL;//拍照缓存
    size_t len;//拍照的数据字节长度

    while (true)
    {

        if (!mqttclient.connected()) {
          //断网重来开始
            Serial.println("do loop 断网重连开始");
            reconnect();
        }
        // client loop
        mqttclient.loop();

        
        // 是否FOTA
        if (fotaFlag)
        {
          deviceStatus = "FOTA";

          fotaFlag = false;
          updateDevice(updateurl,updateversion);
           
        }

        // 是否上报设备状态 10s中断上报
        if (autoUploadFlag)
        {
            dht11 = utils.DHT11(); //读取温湿度

            // printf("Temp=%d.%d℃--Humi=%d.%d%%RH \r\n", Temp,Temp_small,Humi,Humi_small);

            autoUploadFlag = false;
            autoDataStatus();
        }

        if (autoTakePhotoFlag)//远程拍照
        { 
            autoTakePhotoFlag = false;
                TimeStruct stc =  utils.setClock(SRCCID);
            reporttime = stc.reporttime;
            fileName = stc.fileName;
            setup_camera();
            
            take_send_photo();
            desetup_camera();
            
        }
        //过一分钟，自动拍照
        if (tim1_Photo_count > 60 )
        {
            tim1_Photo_count = 0;
            autoTakePhotoFlag = true;
        }
        
        
        // 是否上报视频流
        if (videoUploadFlag)
        {
            if (videoUploadInitFlag)//没有初始化的时候，进行初始化，仅一次
            {
              videoUploadInitFlag = false;
              setup_camera();
              getChanneAndSetStreamSender();
              /* code */
            }
            
            // log_i("send image");
            fb = esp_camera_fb_get();
            if (!fb) {
                Serial.println("Camera capture failed");
                break;
            }
            
            len = fb->len;//拍照字节长度赋值给len
            // fb->buf[len] = channel_index;//赋值给buf的最后一位，是通道ID

            size_t srccitlent = strlen(SRCCID.c_str());
            // Serial.println(srccitlent);

            for (size_t i = 0; i < srccitlent; i++)
            {
              /* code */
               fb->buf[len+i] =SRCCID[i];
            }
            
            streamSender.send(fb->buf, len + srccitlent); //发送数据，带上通道ID，总长度+1
            esp_camera_fb_return(fb);//重复利用一个缓存？？？
            delay(500);//受限带宽1MBSP，慢点发图片给服务端

        }

        //视频流超过5分钟，自动断开   10一次，6对应一分钟
        if (tim1_IRQ_count > 6  & videoUploadFlag )
        {
            cameraFlag = false;
            videoUploadFlag = false;
            // desetup_camera();//卸载摄像头
            Serial.println("tim1_IRQ_count 超过1分钟自动断开");
            tim1_IRQ_count  =0;
            videoUploadStop();
        }

        if (esp_restartFlag )
        {
            esp_restartFlag = false;
            mqttclient.disconnect();
            delay(200);
            esp_restart();
        }

                        

   
 
    }

}
