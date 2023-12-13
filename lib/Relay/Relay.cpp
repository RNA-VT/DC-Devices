#include <Relay.h>

char *Relay::CLOSED = (char *)"closed";
char *Relay::OPEN  = (char *)"open";

Relay::Relay() {
    this->state = CLOSED;
    this->gpio = 11;
    this->lock = xSemaphoreCreateMutex();
    xSemaphoreGive( ( this->lock ) );
}

void Relay::Setup(){
    server = WiFiServer(80);
    Serial.println("Start setup.");
    this->wfm->setup_wifi_manager(); 
    server.begin();
    Serial.println("End setup.");
}

char *Relay::GetState() {
    char *s;
    this->lockState();
    s = this->state;
    this->unlockState();
    return s;
}   

void Relay::SetState(char *s) {
    this->lockState();
    this->state = (char *)s;
    this->unlockState();
}      

void Relay::IOHandler(){
    this->lockState();
    if (this->state == OPEN) {
        digitalWrite(this->gpio, HIGH);
    } else {
        digitalWrite(this->gpio, LOW);
    }
    this->unlockState();
}
