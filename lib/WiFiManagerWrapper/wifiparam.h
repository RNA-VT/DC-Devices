#include <string>
#include <WiFiManager.h>

class WifiParam
{

public:
  char *id;
  char *placeholder;
  char *defaultValue;
  char *value;
  int length;
  WiFiManagerParameter *param;

  WifiParam(char *id, char *placeholder, char *defaultValue, int length)
  {
    this->id = id;
    this->placeholder = placeholder;
    this->defaultValue = defaultValue;
    this->length = length;
  }

  void createParam()
  {
    this->param = new WiFiManagerParameter(this->id, this->placeholder, this->defaultValue, this->length);
  }

  void saveValue()
  {
    strcpy(this->value, this->param->getValue());
    Serial.println("Saved value.");
    Serial.println(this->value);
    Serial.println(this->param->getValue());
  }

  const char * getValue()
  {
    return this->param->getValue();
  }
};
