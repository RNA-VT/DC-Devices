#ifndef WIFI_MANAGER_WRAPPER_H
#define WIFI_MANAGER_WRAPPER_H

#include <FS.h> //this needs to be first, or it all crashes and burns...
#include "SPIFFS.h"
#include <map>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include "./wifiparam.h"
#include "../Relay/Relay.h"

WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Mutex lock to allow access to output variable from either core
SemaphoreHandle_t outputLock;
void lockOutput(){
    xSemaphoreTake(outputLock, portMAX_DELAY);
}
void unlockOutput(){
    xSemaphoreGive(outputLock);
}

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

class WiFiManagerWrapper
{
private:
  uint32_t _ESP_ID;
  char *sensorLabel;
  char *pin;
  WiFiManager wifiManager;
  std::vector<WifiParam> params;
  std::map<String, String> paramValues;

public:
  char *dbBucket;
  Relay *relay;

  WiFiManagerWrapper()
  {
    this->_ESP_ID = ESP.getEfuseMac();
    Serial.println("This is where we're at:");
    Serial.println("ESP ID:");
    Serial.println(String(this->_ESP_ID));
    this->sensorLabel = (char*)"sensor-1";
    this->dbBucket = (char*)"default";
    outputLock = xSemaphoreCreateMutex();
    xSemaphoreGive( ( outputLock) );
  }

  // @id is used for HTTP queries and must not contain spaces nor other special characters
  void add_param(char *id, char *placeholder, char *defaultValue, int length)
  {
    WifiParam param = WifiParam(id, placeholder, defaultValue, length);
    param.createParam();
    params.push_back(param);
    this->paramValues[id] = defaultValue;
  }

  char *getParamValue(char *id)
  {
    if (this->paramValues.find(id) == this->paramValues.end()) {
      Serial.println("Param value not found " + String(id));
      return (char*)"";
    }

    return (char*)this->paramValues[id].c_str();
  }

  void apply_wfm_options()
  {
    //set config save notify callback
    this->wifiManager.setSaveConfigCallback(saveConfigCallback);

    // set custom ip for portal
    //this->wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    for (WifiParam param : this->params)
    {
      this->wifiManager.addParameter(param.param);
    }

    // Uncomment and run it once, if you want to erase all the stored information
    // this->wifiManager.resetSettings();

    //set minimum quality of signal so it ignores AP's under that quality
    //defaults to 8%
    //this->wifiManagerwifiManager.setMinimumSignalQuality();

    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    this->wifiManager.setTimeout(1200);

    // fetches ssid and pass from eeprom and tries to connect
    // if it does not connect it starts an access point with the specified name
    // here  "AutoConnectAP"
    // and goes into a blocking loop awaiting configuration
    // this->wifiManager.autoConnect("AutoConnectAP");
    // or use this for auto generated name ESP + ChipID
    this->wifiManager.autoConnect();
  }

  void load_from_config()
  {
    Serial.println("mounting FS...");
    if (SPIFFS.begin())
    {
      Serial.println("mounted file system");
      if (SPIFFS.exists("/config.json"))
      {
        //file exists, reading and loading
        Serial.println("reading config file");
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile)
        {
          Serial.println("opened config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);

          configFile.readBytes(buf.get(), size);
          DynamicJsonDocument doc(1024);
          deserializeJson(doc, buf.get());

          JsonObject obj = doc.as<JsonObject>();

          Serial.println(obj.size());
          for (WifiParam param : this->params)
          {
            Serial.println("Check if saved value exists for " + String(param.id));
            if (obj.containsKey(param.id))
            {
              Serial.println("Found saved value for " + String(param.id) + " ==> " + obj[param.id].as<String>());
              this->paramValues.insert_or_assign(param.id, obj[param.id].as<String>());
            }
          }
        }
      }
    }
    else
    {
      Serial.println("failed to mount FS");
    }
  }

  void setup_wifi_manager()
  {
    Serial.println("Start setup.");

    //clean FS, for testing
    // SPIFFS.format();

    //read configuration from FS json
    this->load_from_config();
    //end read

    this->apply_wfm_options();

    // if you get here you have connected to the WiFi
    Serial.println("Connected.");

    //save the custom parameters to FS
    if (shouldSaveConfig)
    {
      this->save_config();
    }

    server.begin();
    Serial.println("End setup.");
  }

  void save_config()
  {
    Serial.println("saving config");
    DynamicJsonDocument doc(1024);

    for (WifiParam param : this->params)
    {
      doc[param.id] = param.getValue();
    }

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
      Serial.println("failed to open config file for writing");
    }

    Serial.println("file open prepare to write");

    serializeJsonPretty(doc, Serial);
    serializeJsonPretty(doc, configFile);
    Serial.write('\n');

    Serial.println("file write complete");

    configFile.close();

    Serial.println("config save complete.");
  }

  void do_loop()
  {
    WiFiClient client = server.available(); // Listen for incoming clients

    if (client)
    {                                // If a new client connects,
      Serial.println("New Client."); // print a message out in the serial port
      String currentLine = "";       // make a String to hold incoming data from the client
      while (client.connected())
      { // loop while the client's connected
        if (client.available())
        {                         // if there's bytes to read from the client,
          char c = client.read(); // read a byte, then
          Serial.write(c);        // print it out the serial monitor
          header += c;
          if (c == '\n')
          { // if the byte is a newline character
            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0)
            {
              handleRequest(client);
              // Break out of the while loop
              break;
            }
            else
            { // if you got a newline, then clear currentLine
              currentLine = "";
            }
          }
          else if (c != '\r')
          {                   // if you got anything else but a carriage return character,
            currentLine += c; // add it to the end of the currentLine
          }
        }
      }
      // Clear the header variable
      header = "";
      // Close the connection
      client.stop();
      Serial.println("Client disconnected.");
      Serial.println("");
    }
  }

  void handleRequest(WiFiClient client)
  {
    Serial.println(header);
    String method = header.substring(0, header.indexOf(" "));
    String nextHalf = header.substring(header.indexOf(" ") + 1, header.lastIndexOf(" ") + 2);
    String path = nextHalf.substring(0, nextHalf.indexOf(" "));
    Serial.println("Method: " + method);
    Serial.println("Path: " + path);

    if(method == "GET") {
      if (path == "/") {
        sendHTTP(client);
      } else if (path == "/admin/reset") {
        Serial.println("Resetting.");
        this->wifiManager.resetSettings();
        WiFi.disconnect(true);
        vTaskDelay(2000);
        ESP.restart();
      } else if (path == "/output/on") {
        Serial.println("Output on");
        this->relay->SetState(OPEN);
        sendHTTP(client);
      } else if (path == "/output/off") {
        Serial.println("Output off");
        this->relay->SetState(CLOSED);
        sendHTTP(client);
      } else if (path == "/specification") {
        sendSpecification(client);
      } else {
        client.println("HTTP/1.1 404 NotFound");
        Serial.println("No command.");
        client.println();
        return;
      }
    }
  }

  void sendSpecification(WiFiClient client)
  {
    Serial.println("Send specification.");

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:application/json");
    client.println("Connection: close");
    client.println();

    DynamicJsonDocument doc(1024);

    doc["id"] = String(this->_ESP_ID);
    doc["label"] = String(this->sensorLabel);

    for ( auto paramValueIterator = this->paramValues.begin(); paramValueIterator != this->paramValues.end(); ++paramValueIterator  )
    {
      doc["params"][paramValueIterator->first] = paramValueIterator->second;
    }

    serializeJsonPretty(doc, client);
    client.println();
  }

  void sendHTTP(WiFiClient client)
  {
    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
    // and a content-type so the client knows what's coming, then a blank line:
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();

    // Display the HTML web page
    client.println("<!DOCTYPE html><html>");
    client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<link rel=\"icon\" href=\"data:,\">");
    // CSS to style the on/off buttons
    // Feel free to change the background-color and font-size attributes to fit your preferences
    client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
    client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
    client.println(".button2 {background-color: #77878A;}</style></head>");

    // Web Page Heading
    client.println("<body><h1>Relay Control</h1>");

    // Display current state through ON/OFF buttons for the defined GPIO
    // If the outputState is off, it displays the ON button
    if (this->relay->GetState() == CLOSED)
    {
      client.println("<p><a href=\"/output/on\"><button class=\"button\">ON</button></a></p>");
    }
    else
    {
      client.println("<p><a href=\"/output/off\"><button class=\"button button2\">OFF</button></a></p>");
    }
    client.println("</body></html>");

    // The HTTP response ends with another blank line
    client.println();
  }

};

#endif
