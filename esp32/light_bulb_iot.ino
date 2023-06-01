#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "DIGI_ROUTER";
const char* password = "rfcomm_wifi_wlan";

String serverName = "iot-light-bulb.rf.gd";

unsigned long lastTime = 0;
const unsigned long timerDelay = 10000; // miliseconds

const int MOS_gate_pin = 16;  /* GPIO16 */

int dutyCycle;
/*PWM Properties */
const int PWMFreq = 5000; /* 5 KHz */
const int PWMChannel = 0;
const int PWMResolution = 10;

void setup() 
{
  //PWM setup
  ledcSetup(PWMChannel, PWMFreq, PWMResolution);
  ledcAttachPin(MOS_gate_pin, PWMChannel);
  ledcWrite(PWMChannel, 0);

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
  }
 
}

void loop() 
{
  //Ask the server for updates every 10 seconds
  if ((millis() - lastTime) > timerDelay) 
  {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED)
    {
      HTTPClient http;

      String serverPath = "http://" + serverName + "/iot/light_bulb.php";
      
      http.begin(serverPath.c_str());

      //For some free web servers
      http.setUserAgent("Googlebot/2.1 (+http://www.google.com/bot.html)");
      
      // Send the HTTP GET request
      const int httpResponseCode = http.GET();
      
      //The server response is OK
      if (httpResponseCode == 200) 
      {
        String payload = http.getString();

        const int pwm_val = payload.toInt();
        ledcWrite(PWMChannel, pwm_val);
      }
      
      // Free resources
      http.end();
    }
    else 
    {
      WiFi.disconnect();
      WiFi.reconnect();
    }

    lastTime = millis();
  }
}