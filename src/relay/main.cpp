#include <Arduino.h>
#include <WiFiManagerWrapper.h>
#include <WiFi.h>
#include <Relay.h>
#include <esp_task_wdt.h>

TaskHandle_t WebServerTaskHandle;
TaskHandle_t IOTaskHandle;
WiFiManagerWrapper wfm = WiFiManagerWrapper();
Relay relay = Relay();


void WebServerTask(void * parameter){
  Serial.println("Begin Webserver Task Thread");
  wfm.add_param((char*)"device_name",(char*)"Device Name",(char*)"device-01", 60);
  wfm.setup_wifi_manager();

  for (;;){
    relay.Serve();
  }
}

void IOTask(void * parameter) {
  Serial.println("Begin IO Task Thread");
  for (;;){
    esp_task_wdt_reset();
    Serial.println(wfm.getParamValue((char*)"device_name"));
    digitalWrite(LED_BUILTIN, HIGH);
    relay.IOHandler();
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay( 3 / portTICK_PERIOD_MS ); 
  }

}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(11, OUTPUT);

  // xTaskCreatePinnedToCore(
  //     WebServerTask,
  //     "WebServer", 
  //     10000,  /* Stack size in words */
  //     NULL,  /* Task input parameter */
  //     10,  /* Priority of the task */
  //     &WebServerTaskHandle,
  //     0); 
  // delay(100);

  // xTaskCreatePinnedToCore(
  //   IOTask,
  //   "IO", 
  //   10000,  /* Stack size in words */
  //   NULL,  /* Task input parameter */
  //   10,  /* Priority of the task */
  //   &IOTaskHandle,
  //   1); 
  // delay(100);

}

void loop() {
}

