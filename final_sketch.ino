#include "esp_camera.h"
#include "Arduino.h"
#include "WiFi.h"
#include "soc/soc.h"           //Disable brownout problems
#include "soc/rtc_cntl_reg.h"  //Disable brownout problems

//Replace with your network credentials
const char* ssid="****";
const char* password="****";

//Pin definitions for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

//PDLC Control Pins-Adjust these based on your actual ESP32 pin connections
//Note: Some pins are used by the camera and cannot be used for PDLC control
#define NUM_ZONES 6  //The number of zones used
const int pdlcPins[NUM_ZONES]={12,13,14,15,2,4}; //Example GPIO pins

//Brightness threshold for auto-dimming (0-255)
const int BRIGHTNESS_THRESHOLD=120;

//Store latest brightness values for web display
int zoneBrightness[NUM_ZONES]={0};
bool zoneStatus[NUM_ZONES]={false};

//Initialize Camera
bool startCamera(){
  camera_config_t config;
  config.ledc_channel=LEDC_CHANNEL_0;
  config.ledc_timer=LEDC_TIMER_0;
  config.pin_d0=Y2_GPIO_NUM;
  config.pin_d1=Y3_GPIO_NUM;
  config.pin_d2=Y4_GPIO_NUM;
  config.pin_d3=Y5_GPIO_NUM;
  config.pin_d4=Y6_GPIO_NUM;
  config.pin_d5=Y7_GPIO_NUM;
  config.pin_d6=Y8_GPIO_NUM;
  config.pin_d7=Y9_GPIO_NUM;
  config.pin_xclk=XCLK_GPIO_NUM;
  config.pin_pclk=PCLK_GPIO_NUM;
  config.pin_vsync=VSYNC_GPIO_NUM;
  config.pin_href=HREF_GPIO_NUM;
  config.pin_sscb_sda=SIOD_GPIO_NUM;
  config.pin_sscb_scl=SIOC_GPIO_NUM;
  config.pin_pwdn=PWDN_GPIO_NUM;
  config.pin_reset=RESET_GPIO_NUM;
  config.xclk_freq_hz=20000000;
  config.pixel_format=PIXFORMAT_GRAYSCALE; //Using grayscale format for easier brightness analysis
  
  //Initial settings-adjust based on available memory
  if (psramFound()){
    config.frame_size=FRAMESIZE_QVGA;  //320x240
    config.fb_count=2;
  }
  else{
    config.frame_size=FRAMESIZE_QVGA;  //320x240
    config.fb_count=1;
  }
  
  esp_err_t err=esp_camera_init(&config);
  if (err!=ESP_OK){
    Serial.printf("Camera init failed with error 0x%x", err);
    return false;
  }
  
  return true;
}

//Analyze brightness in zones and control PDLC from frame buffer
void analyzeBrightness(camera_fb_t *fb){
  if (!fb){
    Serial.println("Invalid frame buffer for analysis");
    return;
  }
  
  int width=fb->width;
  int height=fb->height;
  int zoneWidth=width/NUM_ZONES;
  
  //Reset zone sums
  unsigned long zoneSums[NUM_ZONES]={0};
  unsigned long zonePixels[NUM_ZONES]={0};
  
  //Process each pixel in the grayscale image
  for (int y=0;y<height;y++){
    for (int x=0;x<width;x++){
      int zone=x/zoneWidth;
      if (zone>=NUM_ZONES) zone=NUM_ZONES-1;
      
      int index=y*width+x;
      uint8_t brightness=fb->buf[index];
      
      zoneSums[zone]+=brightness;
      zonePixels[zone]++;
    }
  }
  
  //Calculate average brightness for each zone and control PDLC
  for (int i=0;i<NUM_ZONES;i++){
    int avgBrightness=zoneSums[i]/zonePixels[i];
    zoneBrightness[i]=avgBrightness; 
    
    //Apply dimming logic
    if (avgBrightness>BRIGHTNESS_THRESHOLD){
      digitalWrite(pdlcPins[i],LOW); //Dim zone
      zoneStatus[i]=true;
    }
    else{
      digitalWrite(pdlcPins[i],HIGH);  //Keep zone transparent
      zoneStatus[i]=false;
    }
    
    Serial.printf("Zone %d: Brightness %d,Status: %s\n", 
                  i+1,avgBrightness, 
                  (avgBrightness>BRIGHTNESS_THRESHOLD) ? "Dimmed":"Transparent");
  }
}

//Analyze brightness with new capture
void updateBrightnessAnalysis(){
  camera_fb_t *fb=esp_camera_fb_get();
  if (!fb){
    Serial.println("Camera capture failed in analysis");
    return;
  }
  
  analyzeBrightness(fb);
  esp_camera_fb_return(fb);
}

void setup(){
  //Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG,0);

  Serial.begin(115200);
  Serial.println("ESP32-CAM Smart Windshield System Starting");
  
  //Initialize PDLC control pins
  for (int i=0;i<NUM_ZONES;i++){
    pinMode(pdlcPins[i],OUTPUT);
    digitalWrite(pdlcPins[i],HIGH); //Initialize all zones as transparent
  }

  //Initialize camera
  if (!startCamera()){
    Serial.println("Failed to initialize camera. System cannot continue.");
    //Infinite loop to prevent further execution
    while (1){
      delay(1000);
    }
  }

  //Connect to WiFi
  WiFi.begin(ssid,password);
  while (WiFi.status()!=WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop(){
  // Continuously analyze brightness and control PDLC zones
  updateBrightnessAnalysis();
  
  // Small delay to prevent overloading the CPU
  delay(500);
}
