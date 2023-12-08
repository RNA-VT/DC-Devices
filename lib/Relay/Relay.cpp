#include <Relay.h>
#include <WiFiManagerWrapper.h>

// Variable to store the HTTP request
String header;
  
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

void Relay::Serve()
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
              this->HandleRequest(client);
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

  void Relay::HandleRequest(WiFiClient client)
  {
    Serial.println(header);
    String method = header.substring(0, header.indexOf(" "));
    String nextHalf = header.substring(header.indexOf(" ") + 1, header.lastIndexOf(" ") + 2);
    String path = nextHalf.substring(0, nextHalf.indexOf(" "));
    Serial.println("Method: " + method);
    Serial.println("Path: " + path);

    this->wfm->handleAdminEndpoints(method,path);

    if (method == "GET") {
      if (path == "/") {
        this->SendHTTP(client);
      } else if (path == "/output/on") {
        Serial.println("Output on");
        this->SetState(OPEN);
        this->SendHTTP(client);
      } else if (path == "/output/off") {
        Serial.println("Output off");
        this->SetState(CLOSED);
        this->SendHTTP(client);
      } else if (path == "/specification") {
        this->wfm->sendSpecification(client);
      } else {
        client.println("HTTP/1.1 404 NotFound");
        Serial.println("No command.");
        client.println();
        return;
      }
    }
  }



  void Relay::SendHTTP(WiFiClient client)
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
    if (this->GetState() == CLOSED)
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
