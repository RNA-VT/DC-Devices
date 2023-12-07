#ifndef RELAY_H
#define RELAY_H

#include "SPIFFS.h"

char *OPEN = (char *)"open";
char *CLOSED = (char *)"closed";

class Relay
{
    private:
        char *state;
        int pin;
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
            this->pin = 5;
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
                digitalWrite(this->pin, HIGH);
            } else {
                digitalWrite(this->pin, LOW);
            }
            this->unlockState();
        }
};



#endif