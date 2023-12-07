#ifndef RELAY_H
#define RELAY_H

#include "SPIFFS.h"

char *OPEN = (char *)"open";
char *CLOSED = (char *)"closed";

class Relay
{
    private:
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
        Relay() {
            this->state = CLOSED;
            this->gpio = 11;
            this->lock = xSemaphoreCreateMutex();
            xSemaphoreGive( ( this->lock ) );
        }

        char *GetState() {
            char *s;
            this->lockState();
            s = this->state;
            this->unlockState();
            return s;
        }   
 
        void SetState(char *s) {
            this->lockState();
            this->state = (char *)s;
            this->unlockState();
        }      

        void Handler(){
            this->lockState();
            if (this->state == OPEN) {
                digitalWrite(this->gpio, HIGH);
            } else {
                digitalWrite(this->gpio, LOW);
            }
            this->unlockState();
        }
};



#endif
