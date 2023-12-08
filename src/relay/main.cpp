#include <Arduino.h>
#include <WiFiManagerWrapper.h>
#include <WiFi.h>
#include <Relay.h>

Relay relay = Relay();
TaskHandle_t WebServerTaskHandle;
TaskHandle_t IOTaskHandle;
WiFiManagerWrapper wfm = WiFiManagerWrapper();

void WebServerTask(void * parameter){
  Serial.println("Begin Webserver Task Thread");
  wfm.relay = &relay;
  wfm.add_param((char*)"device_name",(char*)"Device Name",(char*)"device-01", 60);
  wfm.setup_wifi_manager();

  for (;;){
    wfm.do_loop();
  }
}

void IOTask(void * parameter) {
  Serial.println("Begin IO Task Thread");
  for (;;){
    Serial.println(wfm.getParamValue((char*)"device_name"));
    digitalWrite(LED_BUILTIN, HIGH);
    relay.Handler();
    digitalWrite(LED_BUILTIN, LOW);
  }

}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(11, OUTPUT);

  xTaskCreatePinnedToCore(
      WebServerTask,
      "WebServer", 
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &WebServerTaskHandle,
      0); 
  delay(100);

  xTaskCreatePinnedToCore(
    IOTask,
    "IO", 
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &IOTaskHandle,
    1); 
  delay(100);

}

void loop() {
}

