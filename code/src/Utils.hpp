#include <Arduino.h>
#include <stdint.h>

#define uchar unsigned char
#define uint8 unsigned char
#define uint16 unsigned short

#define DHT11_PIN   GPIO_NUM_2 //定义DHT11的引脚



typedef struct TimeStruct{
    String fileName;
    String reporttime;
    uint32_t timeStamp;
}TimeStruct;

typedef struct DHT11Struct{
     unsigned char Humi;
     unsigned char Humi_small;
     unsigned char Temp;
     unsigned char Temp_small;

}DHT11Struct;

class LightUtils {
   private:
    // int udp_server;
    // uint16_t server_port;
    
   public:
    LightUtils();
    TimeStruct setClock(String SRCCID);
    String urlEncode(String str);
    String md5str(String strs);
    DHT11Struct DHT11(void);
    String getmac(void);
    void setup_wifi(const char *wifiData[][2], int numNetworks);


    void colorWipe(uint32_t c, uint8_t wait);
    void startShow(int i);
    void rainbow(uint8_t wait);
    void rainbowCycle(uint8_t wait);
    void theaterChase(uint32_t c, uint8_t wait);
    void theaterChaseRainbow(uint8_t wait);
    uint32_t Wheel(byte WheelPos);




};


/* Private define ------------------------------------------------*/
/* Private typedef -----------------------------------------------*/
typedef struct
{
    uint16_t Year;
    uint16_t Mon;
    uint16_t Day;
    uint16_t Hour;
    uint16_t Min;
    uint16_t Sec;
}ST_DATE_TIME,*PST_DATE_TIME;

/* Private constants ---------------------------------------------*/
/* Private variables ---------------------------------------------*/
/* Private function prototypes -----------------------------------*/
/* Public constants ----------------------------------------------*/
/* Public variables ----------------------------------------------*/
uint32_t DateTime2Stamp(PST_DATE_TIME pstDateTime);
ST_DATE_TIME TimeStamp2DateTime(uint32_t Stamp);

