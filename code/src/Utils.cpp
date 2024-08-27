#include "Utils.hpp"
#include <Arduino.h>
#include "MD5Builder.h"
#include <WiFi.h>



//温湿度定义
uchar ucharFLAG,uchartemp;
uchar Humi,Humi_small,Temp,Temp_small;
uchar ucharT_data_H,ucharT_data_L,ucharRH_data_H,ucharRH_data_L,ucharcheckdata;
uchar ucharT_data_H_temp,ucharT_data_L_temp,ucharRH_data_H_temp,ucharRH_data_L_temp,ucharcheckdata_temp;
uchar ucharcomdata;


LightUtils::LightUtils(){}

TimeStruct LightUtils::setClock(String SRCCID) {
    struct tm timeInfo; //声明一个结构体
    if (!getLocalTime(&timeInfo))
    { //一定要加这个条件判断，否则内存溢出
        Serial.println("Failed to obtain time");
            struct TimeStruct tretcc;
            tretcc.fileName = "0";
            tretcc.reporttime =  "0";
            tretcc.timeStamp = 0;
        return tretcc;
    }
    String zone = "0";

    String tm_year  = String(timeInfo.tm_year + 1900); //年;

    String tm_mon  = (timeInfo.tm_mon + 1) >9 ? (timeInfo.tm_mon + 1) : (zone + (timeInfo.tm_mon + 1) );
    String tm_mday  = (timeInfo.tm_mday) >9 ? ( timeInfo.tm_mday) : (zone + (timeInfo.tm_mday ) );
    String tm_hour  = (timeInfo.tm_hour) >9 ? (timeInfo.tm_hour) : (zone + (timeInfo.tm_hour ) );
    String tm_min  = (timeInfo.tm_min) >9 ? (timeInfo.tm_min) : (zone + (timeInfo.tm_min ) );
    String tm_sec  = (timeInfo.tm_sec) >9 ? (timeInfo.tm_sec) : (zone + (timeInfo.tm_sec ) );
    
    // sprintf_P(buff1, PSTR("%04d-%02d-%02d %s"), timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday, WDAY_NAMES[timeInfo.tm_wday].c_str());
    String shuju = tm_year;//String(timeInfo.tm_year + 1900); //年

    shuju += tm_mon; //月
    shuju += tm_mday; //日
    shuju += tm_hour; //时
    shuju += tm_min;//分
    shuju += tm_sec;//秒


    Serial.println(shuju.c_str());

    String cs = SRCCID;
            
    String fileName = cs + "_"+ shuju +".jpg";

    String cuss = "";
    cuss+= tm_year; //年
    cuss+="-";
    cuss += tm_mon; //月
    cuss+="-";
    cuss += tm_mday; //日
    cuss+=" ";
    cuss +=tm_hour; //时
    cuss+=":";
    cuss += tm_min;//分
    cuss+=":";
    cuss += tm_sec;//秒

    String reporttime = cuss;
    
    tm st;
  st.tm_year = timeInfo.tm_year;
  st.tm_mon = timeInfo.tm_mon;
  st.tm_mday = timeInfo.tm_mday;
  st.tm_hour = timeInfo.tm_hour;
  st.tm_min = timeInfo.tm_min;
  st.tm_sec = timeInfo.tm_sec;

    uint32_t timeStamp = (uint32_t)mktime(&st);

     struct TimeStruct tret;
     tret.fileName = fileName;
     tret.reporttime = reporttime;
     tret.timeStamp = timeStamp;

    return tret;

}


String LightUtils::urlEncode(String str){
    String new_str = "";
    char c;
    int ic;
    const char* chars = str.c_str();
    char bufHex[10];
    int len = strlen(chars);

    for(int i=0;i<len;i++){
        c = chars[i];
        ic = c;
        // uncomment this if you want to encode spaces with +
        /*if (c==' ') new_str += '+';   
        else */if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') new_str += c;
        else {
            sprintf(bufHex,"%X",c);
            if(ic < 16) 
                new_str += "%0"; 
            else
                new_str += "%";
            new_str += bufHex;
        }
    }
    return new_str;
 }


String LightUtils::md5str(String strs){
   
    MD5Builder md5;
    md5.begin();
    md5.add(strs);
    md5.calculate();
    const char *md5str = md5.toString().c_str();
    // printf("md5str: %s\r\n",md5str);

    return String(md5.toString());

}




static void InputInitial(void)//设置端口为输入
{
  gpio_pad_select_gpio(DHT11_PIN);
  gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);
}
 
static void OutputHigh(void)//输出1
{
  gpio_pad_select_gpio(DHT11_PIN);
  gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(DHT11_PIN, 1);
}
 
static void OutputLow(void)//输出0
{
  gpio_pad_select_gpio(DHT11_PIN);
  gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(DHT11_PIN, 0);
}
 
static uint8 getData()//读取状态
{
	return gpio_get_level(DHT11_PIN);
}
 
//读取一个字节数据
static void COM(void)    // 温湿写入
{
    uchar i;
    for(i=0;i<8;i++)
    {
        ucharFLAG=2;
        //等待IO口变低，变低后，通过延时去判断是0还是1
        while((getData()==0)&&ucharFLAG++) ets_delay_us(10);
        ets_delay_us(35);//延时35us
        uchartemp=0;
 
        //如果这个位是1，35us后，还是1，否则为0
        if(getData()==1) 
          uchartemp=1;
        ucharFLAG=2;
 
        //等待IO口变高，变高后，表示可以读取下一位
        while((getData()==1)&&ucharFLAG++) 
          ets_delay_us(10);
        if(ucharFLAG==1)
          break;
        ucharcomdata<<=1;
        ucharcomdata|=uchartemp;
    }
}
 
void Delay_ms(uint16 ms)
{
	int i=0;
	for(i=0; i<ms; i++){
		ets_delay_us(1000);
	}
}

 
DHT11Struct LightUtils:: DHT11(void)   //温湿传感启动
{
    struct DHT11Struct dtc11;
    OutputLow();
    Delay_ms(19);  //>18MS
    OutputHigh();
    InputInitial(); //输入
    ets_delay_us(30);
    if(!getData())//表示传感器拉低总线
    {
        ucharFLAG=2;
        //等待总线被传感器拉高
        while((!getData())&&ucharFLAG++) 
          ets_delay_us(10);
        ucharFLAG=2;
        //等待总线被传感器拉低
        while((getData())&&ucharFLAG++) 
          ets_delay_us(10);
        COM();//读取第1字节，
        ucharRH_data_H_temp=ucharcomdata;
        COM();//读取第2字节，
        ucharRH_data_L_temp=ucharcomdata;
        COM();//读取第3字节，
        ucharT_data_H_temp=ucharcomdata;
        COM();//读取第4字节，
        ucharT_data_L_temp=ucharcomdata;
        COM();//读取第5字节，
        ucharcheckdata_temp=ucharcomdata;
        OutputHigh();
        //判断校验和是否一致
        uchartemp=(ucharT_data_H_temp+ucharT_data_L_temp+ucharRH_data_H_temp+ucharRH_data_L_temp);
        if(uchartemp==ucharcheckdata_temp)
        {
            //校验和一致，
            ucharRH_data_H=ucharRH_data_H_temp;
            ucharRH_data_L=ucharRH_data_L_temp;
            ucharT_data_H=ucharT_data_H_temp;
            ucharT_data_L=ucharT_data_L_temp;
            ucharcheckdata=ucharcheckdata_temp;
            //保存温度和湿度
            Humi=ucharRH_data_H;
            Humi_small=ucharRH_data_L;
            Temp=ucharT_data_H;
            Temp_small=ucharT_data_L;

            dtc11.Humi = Humi;
            dtc11.Humi_small = Humi_small;
            dtc11.Temp = Temp;
            dtc11.Temp_small = Temp_small;

        }
        else
        {
            Humi=0;
            Temp=0;
            Humi_small = 0;
            Temp_small = 0;
            dtc11.Humi = Humi;
            dtc11.Humi_small = Humi_small;
            dtc11.Temp = Temp;
            dtc11.Temp_small = Temp_small;
        }
    }
    else //没用成功读取，返回0
    {
        Humi=0;
        Temp=0;
        Humi_small = 0;
        Temp_small = 0;
        dtc11.Humi = Humi;
        dtc11.Humi_small = Humi_small;
        dtc11.Temp = Temp;
        dtc11.Temp_small = Temp_small;
    }
 
    OutputHigh(); //输出

    return dtc11;
}



/* Includes ------------------------------------------------------*/
// #include "date_time.h"
#include <time.h>

/* Private define ------------------------------------------------*/
/* Private typedef -----------------------------------------------*/
/* Private constants ---------------------------------------------*/
/* Private variables ---------------------------------------------*/
/* Private function prototypes -----------------------------------*/
/* Public constants ----------------------------------------------*/
/* Public variables ----------------------------------------------*/

/********************************************************************
* name          : DateTime2Stamp
* description   : 根据实际获取时间戳
* Input         : pstDateTime
* Output        : nonoe
* Return        : Stamp
********************************************************************/
uint32_t DateTime2Stamp(PST_DATE_TIME pstDateTime)
{
    struct tm stTime;
    time_t TimeStamp;

    stTime.tm_year = pstDateTime->Year - 1900;
    stTime.tm_mon = pstDateTime->Mon - 1;
    stTime.tm_mday = pstDateTime->Day;
    stTime.tm_hour = pstDateTime->Hour;
    stTime.tm_min = pstDateTime->Min;
    stTime.tm_sec = pstDateTime->Sec;
    TimeStamp = mktime(&stTime);
    return (uint32_t)TimeStamp;
}

/********************************************************************
* name          : TimeStamp2DateTime
* description   : 根据时间戳获取时间
* Input         : Stamp
* Output        : none
* Return        : DateTime
********************************************************************/
ST_DATE_TIME TimeStamp2DateTime(uint32_t Stamp)
{
    ST_DATE_TIME stDateTime;

    time_t TimeStamp = Stamp;
    struct tm* pstTime = localtime(&TimeStamp);
    stDateTime.Year = pstTime->tm_year + 1900;
    stDateTime.Mon = pstTime->tm_mon + 1;
    stDateTime.Day = pstTime->tm_mday;
    stDateTime.Hour = pstTime->tm_hour;
    stDateTime.Min = pstTime->tm_min;
    stDateTime.Sec = pstTime->tm_sec;
    return stDateTime;
}


String  LightUtils::getmac(){
  // uint8_t efuse_mac[6];
  char mac[50];
  // esp_err_t re = ESP_FAIL;
  //     delay(300);
  // while (re != ESP_OK)
  // {
    // re = esp_read_mac(efuse_mac,ESP_MAC_ETH);
    uint8_t macAddr[6]; // 定义macAddr为uint8_t类型的数组，这个数组含有6个元素。
  WiFi.macAddress(macAddr);  //MAC地址会储存在这个macAddr数组里面
   sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X", macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);

  Serial.printf("MAC地址: %02X:%02X:%02X:%02X:%02X:%02X\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

          //  Serial.printf("获取MAC:\n",mac);
           String ccs = "";
          //    sprintf(param, "{\"brightness\":%d}", 32); 
          for (size_t i = 0; i < 6; i++)
          {
            char str[4]; 
            sprintf(str, "%02X", macAddr[i]);
             //Serial.printf(str);
           
            ccs = ccs + (String)str ;

            if (i >= 5)
            {
              
            }else{
               ccs = ccs +":";
            }
            

            /* code */
          }
          
           //  Serial.printf(ccs.c_str());


  return ccs.c_str();
}


void  LightUtils:: setup_wifi(const char *wifiData[][2], int numNetworks)
{

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);

    
    // WiFi.begin(ssid, passwd);
    u_int8_t wifi_failedcount = 0;

    while (WiFi.status() != WL_CONNECTED) {

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
                  if (count >= 10)
                  {
                      Serial.printf("\r\n-- wifi connect fail! --");
                      break;
                  }
                  vTaskDelay(300);
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

        wifi_failedcount ++;
        if (wifi_failedcount >=3)
        {
          digitalWrite(42,HIGH);
          wifi_failedcount = 0;
        }

    }
    digitalWrite(42,LOW);

    wifi_failedcount = 0;
    Serial.print("\nWiFi connected, local IP address:");
    Serial.println(WiFi.localIP());
}







