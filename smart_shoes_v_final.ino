#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

String apiKey = "PZUUGLJTUUEADEJ9";

const char *ssid = "ssid";
const char *pass = "password";
const char* server = "api.thingspeak.com";

WiFiClient client;

float x;
float y;
float z;

float min_avg = 0;
float max_avg = 0;
int sampling = 0;

int steps = 0;
boolean flag = 0;

float threshold = 14;

float x_avg = 0;
float y_avg = 0;
float z_avg = 0;

float ave = 0;

/* Assign a unique ID to this sensor */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);


void setup(void)
{
  
  Serial.begin(9600);

  Serial.println("Connecting to..");
  Serial.println(ssid);

  WiFi.begin(ssid,pass);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WIFI CONNECTED");


 
  Serial.println("Accelerometer Test"); Serial.println("");

  /* Initialise the sensor */
  if (!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1);
  }

  /* Set the range of accelerometer*/
  accel.setRange(ADXL345_RANGE_16_G);


  calibrate();
  Serial.print(steps);

  

  delay(500);
}

void loop(void)
{
  
  /* Get a new sensor event */
  sensors_event_t event;
  accel.getEvent(&event);



  x = event.acceleration.x;
  y = event.acceleration.y;
  z = event.acceleration.z;
  ave = sqrt((x - x_avg) * (x - x_avg) + (y - y_avg) * (y - y_avg) + (z - z_avg) * (z - z_avg));
//  ave = sqrt(x*x + y*y +z*z);

  if(sampling == 0)
  {
    max_avg = ave;
    min_avg = ave;
  }

  else if(max_avg < ave)
  {
    max_avg = ave;
  }

  else if(min_avg > ave)
  {
    min_avg = ave;
  }

  sampling++;

  if(sampling >= 50)
  {
    threshold = (max_avg + min_avg) / 2 + 2;
    sampling = 0;
  }
  
  if (ave > threshold && flag == 0)
  {
    steps++;
    Serial.print("Step: ");
    Serial.println(steps);
 
    flag = 1;
  }

  else if (ave > threshold && flag == 1)
  {
    Serial.print("Step: ");
    Serial.println(steps);
  }

  else if (ave < threshold && flag == 1)
  {
    flag = 0;
    Serial.print("Step: ");
    Serial.println(steps);
  }

 if(client.connect(server,80)){

     String postStr = apiKey;
         postStr +="&field1=";
         postStr += String(steps);
         postStr += "\r\n\r\n";

         client.print("POST /update HTTP/1.1\n");
         client.print("Host: api.thingspeak.com\n");
         client.print("Connection: close\n");
         client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
         client.print("Content-Type: application/x-www-form-urlencoded\n");
         client.print("Content-Length: ");
         client.print(postStr.length());
         client.print("\n\n");
         client.print(postStr);
  
 }

  delay(50);
}

void calibrate()
{
  Serial.println("Starting calibration!!!");
  int sumx = 0;
  int sumy = 0;
  int sumz = 0;

  for (int i = 0; i < 100; i++)
  {
    sensors_event_t event;
    accel.getEvent(&event);

    sumx = sumx + event.acceleration.x;
    delay(10);

    sumy = sumy + event.acceleration.y;
    delay(10);

    sumz = sumz + event.acceleration.z;
    delay(10);
  }

  x_avg = sumx / 100;
  y_avg = sumy / 100;
  z_avg = sumz / 100;

  Serial.println(x_avg);
  Serial.println(y_avg);
  Serial.println(z_avg);

  Serial.println("End of calibration!!!");

  delay(1000);
}
