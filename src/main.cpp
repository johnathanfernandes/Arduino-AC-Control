// Arduino AC controller

#include <Arduino.h>     // Arduino library
#include <Stepper.h>     // Stepper motor library
#include <ESP8266WiFi.h> // NodeMCU WiFi thing library

#define sensorPin A0 // Temperature sensor input

char *ssid = "REDACTED";         // Wi-Fi SSID
char *password = "REDACTED";     // Wi-Fi password
int stepsPerRevolution = 90 * 3; // Motor control
int value = LOW;                 // AC state variable
float temp_celsius = 0;          // Measured room temperature

// Initialize stepper motor
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);

// Initialize Wi-Fi server
WiFiServer server(80);

void setup()
{
  myStepper.setSpeed(5); // Set motor to operate at 60 rpm

  Serial.begin(9600);

  WiFi.begin(ssid, password);
  server.begin();
  Serial.print("Connection URL: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP()); // Enter in browser to access website
  Serial.println("/");
}

void loop()
{
  temp_celsius = (analogRead(sensorPin) * 330.0) / 1023.0; // Specific to this particular sensor

  // Check for readings from temperature sensor
  if (isnan(temp_celsius))
  {
    return;
  }

  // Check if wifi server is available
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available())
  {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Match the request
  if (request.indexOf("/AC=ON") != -1 || temp_celsius > 24)
  {
    myStepper.step(stepsPerRevolution); // clockwise - decrease temp (turn on)
    value = HIGH;
  }
  if (request.indexOf("/AC=OFF") != -1 || temp_celsius < 17)
  {
    myStepper.step(-stepsPerRevolution); // counterclockwise - increase  temp (turn off)
    value = LOW;
  }

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.print("AC is now: ");
  if (value == HIGH)
  {
    client.print("On");
  }
  else
  {
    client.print("Off");
  }
  
  client.println("<br><br>");
  client.println("Click <a href=\"/AC=ON\">here</a> turn the AC ON<br>");
  client.println("Click <a href=\"/AC=OFF\">here turn the AC OFF<br>");
  client.println("</html>");
}

// Stepper motor reference: https://www.tutorialspoint.com/arduino/arduino_stepper_motor.htm#
// LM35 temperature sensor reference: https://www.makerguides.com/lm35-arduino-tutorial/
// Web server reference: https://www.instructables.com/Arduino-UNO-ESP8266-WiFi-Module/