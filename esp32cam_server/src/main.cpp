/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
#include <OV2640.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Network Settings
// Wifi passwd and SSID :
// 1. add wifi_settings.h file inside the include dir or uncomment the defines and comment the include
// 2. add wifi passwd &  SSID
//#define NET_SSID "****"
//#define NET_PWD "****" 
#include "wifi_settings.h"
#define NET_port 80
IPAddress NET_staticIP(192, 168, 1, 231); //ESP static ip
IPAddress NET_gateway(192, 168, 1, 1);   //IP Address of Router (Gateway)
IPAddress NET_subnet(255, 255, 255, 0);  //Subnet mask
IPAddress NET_dns(8, 8, 8, 8);  //DNS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Headers
const char HEADER[] = "HTTP/1.1 200 OK\r\n" \
                      "Access-Control-Allow-Origin: *\r\n" \
                      "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";
const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";
const int hdrLen = strlen(HEADER);
const int bdrLen = strlen(BOUNDARY);
const int cntLen = strlen(CTNTTYPE);
const char JHEADER[] = "HTTP/1.1 200 OK\r\n" \
                       "Content-disposition: inline; filename=capture.jpg\r\n" \
                       "Content-type: image/jpeg\r\n\r\n";
const int jhdLen = strlen(JHEADER);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Serial debugging
// enabled/disabled
#define DEBUG_SERIAL true //false for no serial
#define print(text) if(DEBUG_SERIAL) Serial.println(text);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
OV2640 cam;
WebServer server(NET_port);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handle_jpg_stream(void)
{
  char buf[32];
  int s;

  WiFiClient client = server.client();

  client.write(HEADER, hdrLen);
  client.write(BOUNDARY, bdrLen);

  while (true)
  {
    if (!client.connected()) break;
    cam.run();
    s = cam.getSize();
    client.write(CTNTTYPE, cntLen);
    sprintf( buf, "%d\r\n\r\n", s );
    client.write(buf, strlen(buf));
    client.write((char *)cam.getfb(), s);
    client.write(BOUNDARY, bdrLen);
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handle_jpg(void)
{
  WiFiClient client = server.client();

  if (!client.connected()) return;
  cam.run();
  client.write(JHEADER, jhdLen);
  client.write((char *)cam.getfb(), cam.getSize());
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handle_status(void){ // returns the current sensors settings and other status
  server.send(200, "text/plain", "Status");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool change_resolution(int value){
  print("Changing settings....");
  
  sensor_t * s = esp_camera_sensor_get();
  
  if(s->pixformat == PIXFORMAT_JPEG && (value>=0 && value<=10))  s->set_framesize(s, (framesize_t)value);
  
  return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool change_settings(String id, int value){
  print("Changing settings....");
  
  sensor_t * s = esp_camera_sensor_get();

  if(id == "quality" && (value >= 0 && value <= 63))        s->set_quality(s, value);

  if(id == "contrast" && (value >= -2 && value <= 2))       s->set_contrast(s, value);
  if(id == "brightness" && (value >= -2 && value <= 2))     s->set_brightness(s, value);  
  if(id == "saturation" && (value >= -2 && value <= 2))     s->set_saturation(s, value);

  if(id == "awb" && (value == 0 || value == 1))             s->set_whitebal(s, value);    
  if(id == "awb_gain" && (value == 0 || value == 1))        s->set_awb_gain(s, value);
  if(id == "wb_mode" && (value >= 0 && value <= 4))         s->set_wb_mode(s, value);
  
  if(id == "aec2" && (value == 0 || value == 1))            s->set_aec2(s, value);
  if(id == "ae_level" && (value >= -2 && value <= 2))       s->set_ae_level(s, value);
  if(id == "aec_value" && (value >= 0 && value <= 1200))    s->set_aec_value(s, value);

  if(id == "agc" && (value == 0 || value == 1))             s->set_gain_ctrl(s, value);
  if(id == "agc_gain" && (value >= 0 && value <= 30))       s->set_agc_gain(s, value);
  if(id == "gainceiling" && (value >= 0 && value <= 6))     s->set_gainceiling(s, (gainceiling_t)value);

  if(id == "bpc" && (value == 0 || value == 1))             s->set_bpc(s, value);
  if(id == "wpc" && (value == 0 || value == 1))             s->set_wpc(s, value);
  if(id == "raw_gma" && (value == 0 || value == 1))         s->set_raw_gma(s, value);
    
  if(id == "denoise" && (value == 0 || value == 1))         s->set_denoise(s, value);     
  if(id == "lenc" && (value == 0 || value == 1))            s->set_lenc(s, value);

  if(id == "colorbar" && (value == 1 && value == 0))        s->set_colorbar(s, value);
  if(id == "special_effect" && (value >= 0 && value <= 6))  s->set_special_effect(s, value);
  
  if(id == "hmirror" && (value == 0 || value == 1))         s->set_hmirror(s, value);
  if(id == "vflip" && (value == 0 || value == 1))           s->set_vflip(s, value);
  
  else return false;
  return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handle_settings(void){ //handles changes of camera settings by query strings
  int numOfArgs = server.args();
  bool QueryStatus = true;
  
  for (int i = 0; i < numOfArgs; i++) {
    String parameter_name = server.argName(i);
    String parameter_value = server.arg(i);
    print(parameter_name + " : ")
    print(parameter_value.toInt());
    print(parameter_value);
    if(parameter_name == "resolution") if(!change_resolution(parameter_value.toInt())){ QueryStatus = false;  break;} 
    if(!change_settings(parameter_name,parameter_value.toInt()) ) { QueryStatus = false;  break;} 
  } 
  
  if(QueryStatus) server.send(200, "text/plain", "OK");       //Response to the HTTP request
  else server.send(200, "text/plain", "NOPE");       //Response to the HTTP request
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handleNotFound()
{
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.send(200, "text / plain", message);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{

  Serial.begin(115200);
  //while (!Serial);            //wait for serial connection.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Frame parameters
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  cam.init(config);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  WiFi.hostname(NET_deviceName);      // DHCP Hostname
//  WiFi.config(NET_ staticIP, NET_subnet, NET_gateway, NET_dns);
  WiFi.mode(WIFI_STA);
  WiFi.begin(NET_SSID, NET_PWD);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    print(F("."));
  }
  
  IPAddress ip = WiFi.localIP();
  Serial.println(F("WiFi connected"));
  Serial.println("");
  Serial.println(ip);
  server.on("/stream", HTTP_GET, handle_jpg_stream);
  server.on("/image", HTTP_GET, handle_jpg);
  server.on("/settings", HTTP_GET, handle_settings);
  server.on("/status", HTTP_GET, handle_status);
  server.onNotFound(handleNotFound);
  server.begin();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  server.handleClient();
}
