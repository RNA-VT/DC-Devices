#ifndef RELAY_H
#define RELAY_H

#include "SPIFFS.h"
#include <WiFiClient.h>
#include <WiFiManagerWrapper.h>

class Relay
{
    private:
        WiFiServer server;
        char *state;
        int gpio;
        SemaphoreHandle_t lock;
        void lockState(){
            xSemaphoreTake(this->lock, portMAX_DELAY);
        }
        
        void unlockState(){
            xSemaphoreGive(this->lock);
        }
    public:
        WiFiManagerWrapper *wfm;
        static char *OPEN;
        static char *CLOSED;
        Relay();
        char *GetState();
        void HandleRequest(WiFiClient client);
        void IOHandler();
        void SendHTTP(WiFiClient client);
        void SendSpecification(WiFiClient client);
        void Serve();
        void SetState(char *s);
        void Setup();
};



#endif
