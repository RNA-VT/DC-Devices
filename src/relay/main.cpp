#include <Arduino.h>
#include <WiFiManagerWrapper.h>
#include <WiFi.h>

WiFiManagerWrapper wfm = WiFiManagerWrapper();
TaskHandle_t WebServerTaskHandle;
TaskHandle_t IOTaskHandle;

void WebServerTask(void * parameter){
  Serial.println("Begin Webserver Task Thread");
  wfm.add_param((char*)"device_name",(char*)"Device Name",(char*)"device-01", 60);
  wfm.setup_wifi_manager();

  for (;;){
    wfm.do_loop();
    Serial.println(wfm.getParamValue((char*)"device_name"));
  }
}

void IOTask(void * parameter) {
  Serial.println("Begin IO Task Thread");
  for (;;){
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(1000);
  }

}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED_BUILTIN, OUTPUT);

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

