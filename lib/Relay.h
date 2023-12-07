#ifndef RELAY_H
#define RELAY_H

#include "SPIFFS.h"

class Relay
{
    private:
        char *state;
        SemaphoreHandle_t lock;
        void lockOutput(){
            xSemaphoreTake(this->lock, portMAX_DELAY);
        }
        
        void unlockOutput(){
            xSemaphoreGive(this->lock);
        }
    public:
        Relay() {
            this->state = "off";
            this->lock = xSemaphoreCreateMutex();
            xSemaphoreGive( ( this->lock ) );
        }

        char *GetState() {
            char *s;
            this->lockOutput();
            s = this->state;
            this->unlockOutput();
            return s;
        }   
 
        void SetState(char *s) {
            this->lockOutput();
            this->state = s;
            this->unlockOutput();
        }      

        void Handler(){
            
        }
};



#endif