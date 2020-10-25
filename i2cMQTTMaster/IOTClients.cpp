#include "IOTClients.h"

MQTTClient::MQTTClient(const char* clientName,const char* userName, const char* password) {
    this->clientName = clientName;
    this->userName = userName;
    this->password = password;
}

void MQTTClient::setupClient(PubSubClient* pubSubClient, const char* mqttBroker, const int mqttPort) {
    this -> pubSubClient = pubSubClient;
    pubSubClient -> setServer(mqttBroker, mqttPort);
}

void MQTTClient::publishMessage(char* topic, char* payload) {
   if (!pubSubClient->connected()) {
      this -> reconnect();
   }
   pubSubClient->loop();
   Serial.println("publishing");
   pubSubClient->publish(topic, payload);
}

void MQTTClient::reconnect(int connectTimeout) {
  Serial.print("connect...");
   while (!pubSubClient->connected()) {
      Serial.print("Reconnecting...");
      if (!pubSubClient->connect(clientName, userName, password)) {
         Serial.print("failed, return state=");
         Serial.print(pubSubClient->state());
         Serial.print(" retrying in ");Serial.print(connectTimeout);Serial.println(" milli seconds");
         delay(connectTimeout);
      }
   }
}

void setupWifiConnection(const char* ssid,const char* password) {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");
   
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Waiting for connection...");
    delay(1000);
  }

  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}
