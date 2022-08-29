#include <Arduino.h>
#include "painlessMesh.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

#define DHTPIN 4      // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);

String readDHTTemperature()
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else
  {
    Serial.println(t);
    return String(t);
  }
}

// User stub
void sendMessage(); // Prototype so PlatformIO doesn't complain

Task taskSendMessage(TASK_SECOND * 0.01, TASK_FOREVER, &sendMessage);

void sendMessage()
{
  String msg = readDHTTemperature();
  msg += mesh.getNodeId();
  mesh.sendBroadcast(msg);
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  // Serial.println(readDHTTemperature());
  // if(mesh.isConnected(from))
  //{
  // mesh.sendSingle(from, readDHTTemperature());
  //}
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup()
{
  Serial.begin(115200);
  sensors.begin();

  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop()
{
  // it will run the user scheduler as well
  mesh.update();
}