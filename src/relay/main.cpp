#include <Arduino.h>
#include <WiFiManagerWrapper.h>
#include <ESP8266WiFi.h>

WiFiManagerWrapper wfm = WiFiManagerWrapper();

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  wifi_set_sleep_type(NONE_SLEEP_T);

  wfm.add_param("device_name", "Device Name", "device-01", 60);

  wfm.setup_wifi_manager();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello World!");

  wfm.do_loop();

  Serial.println(wfm.getParamValue("device_name"));

  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
