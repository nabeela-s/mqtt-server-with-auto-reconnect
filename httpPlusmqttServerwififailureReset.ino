#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>          //needed for library
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <ESP8266mDNS.h>
#include <PubSubClient.h>         //https://github.com/knolleary/pubsubclient
MDNSResponder mdns;
ESP8266WebServer server(333);
String webPage = "";

int pgio5_pin = 5;
//define your default values here, if there are different values in config.json, they are overwritten.
//length should be max size + 1 
char mqtt_server[41] = "m11.cloudmqtt.com";
char mqtt_portStr[11] = "14318";
char mqtt_username[21] = "vxhucguu";
char mqtt_password[21] = "Qp2weImuk2MH";
char mqtt_topic1[21] = "GPIO0";
char mqtt_topic2[21] = "GPIO2";
char mqtt_topic3[21] = "GPIO4";
char mqtt_topic4[21] = "GPIO5";
int mqtt_port = atoi(mqtt_portStr);

//default custom static IP
char static_ip[16] = "192.168.8.112";
char static_gw[16] = "192.168.8.1";
char static_sn[16] = "255.255.255.0";

//flag for saving data
#define BUTTON_PIN 0 // GPIO0, button to enter wifi manager
bool shouldSaveConfig = false;    
boolean startConfigPortal = false;
void callback(char* topic, byte* payload, unsigned int length);//callback signature

WiFiManager wifiManager; //initializing things
WiFiClient espClient;
PubSubClient mqttClient(mqtt_server, mqtt_port, callback, espClient);
void Button();

String readStr;
long chipid;


//void callback(const char[] topic, byte* payload, unsigned int length)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
if (strcmp(topic, mqtt_topic1) == 0) {
    if ((char)payload[0] == '1')
    {
      Serial.println("GPIO0 is ON");
      digitalWrite(0, HIGH);  
      mqttClient.publish(topic, "GPIO0 is ON now.");
      
    }
    else if ((char)payload[0] == '0')
    {
      Serial.println("GPIO0 is LOW");
      digitalWrite(0, LOW); 
      mqttClient.publish(topic, "GPIO0 is LOW now.");
    }
  }//gpio0
 if (strcmp(topic, mqtt_topic2)== 0) {
    if ((char)payload[0] == '1')
    {
      Serial.println("GPIO2 is ON");
      digitalWrite(2, HIGH);
      mqttClient.publish(topic, "GPIO2 is ON now.");  
    }
    else if ((char)payload[0] == '0')
    {
      Serial.println("GPIO2 is LOW");
      digitalWrite(2, LOW); 
      mqttClient.publish(topic, "GPIO2 is LOW now.");
    }
  }//end gpio2
  if (strcmp(topic, mqtt_topic3) == 0) {
    if ((char)payload[0] == '1')
    {
      Serial.println("GPIO4 is ON");
      digitalWrite(4, HIGH);  
      mqttClient.publish(topic, "GPIO4 in ON now.");
    }
    else if ((char)payload[0] == '0')
    {
      Serial.println("GPIO4 is LOW");
      digitalWrite(4, LOW); 
      mqttClient.publish(topic, "GPIO4 in LOW now.");
    } 
  }//end gpio4
  if (strcmp(topic, mqtt_topic4) == 0) {
    if ((char)payload[0] == '1')
    {
      Serial.println("GPIO5 is ON");
      digitalWrite(5, HIGH);  
      mqttClient.publish(topic, "GPIO5 in ON now.");
    }
    else if ((char)payload[0] == '0')
    {
      Serial.println("GPIO5 is LOW");
      digitalWrite(5, LOW); 
      mqttClient.publish(topic, "GPIO5 in LOW now.");
    }
  }//end gpio5
}//end callback

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void saveConfigJson() {
  //save the custom parameters to FS
  Serial.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["mqtt_server"] = mqtt_server;
  json["mqtt_port"] = mqtt_portStr;
  json["mqtt_username"] = mqtt_username;
  json["mqtt_password"] = mqtt_password;
  json["mqtt_topic1"] = mqtt_topic1;
  json["mqtt_topic2"] = mqtt_topic2;
  json["mqtt_topic3"] = mqtt_topic3;
  json["mqtt_topic4"] = mqtt_topic4;
  
                                                             
  json["ip"] = WiFi.localIP().toString();
  json["gateway"] = WiFi.gatewayIP().toString();
  json["subnet"] = WiFi.subnetMask().toString();

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  json.printTo(Serial);
  Serial.println();
  json.printTo(configFile);
  configFile.close();
  //end save
}

void setup() {
  
  Serial.begin(115200);
  Serial.println();
  pinMode(BUTTON_PIN, INPUT);
  pinMode(5, OUTPUT); digitalWrite(5, LOW);
  pinMode(2, OUTPUT); digitalWrite(2, LOW);
  pinMode(4, OUTPUT); digitalWrite(4, LOW);
  pinMode(0, OUTPUT); digitalWrite(0, LOW);
 

  //Serial.println("Formatindg FS...");
  //SPIFFS.format(); //clean FS, for testing
  //Serial.print("Reset WiFi settings..."); 
  //wifiManager.resetSettings();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {  
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_portStr, json["mqtt_port"]);
          mqtt_port = atoi(mqtt_portStr);
          strcpy(mqtt_username, json["mqtt_username"]);
          strcpy(mqtt_password, json["mqtt_password"]);
          strcpy(mqtt_topic1, json["mqtt_topic1"]);
          strcpy(mqtt_topic2, json["mqtt_topic2"]);
          strcpy(mqtt_topic3, json["mqtt_topic3"]);
          strcpy(mqtt_topic4, json["mqtt_topic4"]);

          if(json["ip"]) {
            Serial.println("setting custom ip from config");
            strcpy(static_ip, json["ip"]);
            strcpy(static_gw, json["gateway"]);
            strcpy(static_sn, json["subnet"]);
            Serial.println(static_ip);
          } else {
            Serial.println("no custom ip in config");
          }
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
  Serial.println(static_ip);
  Serial.println(mqtt_server);
  
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  if (digitalRead(BUTTON_PIN) == LOW ) {
    startConfigPortal = true;
  }

  WiFi.waitForConnectResult();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Failed to connect Wifi");
    startConfigPortal = true;
  }
  
  if(startConfigPortal){
    Serial.println("Clearing WIFI Credencials...");
    wifiManager.resetSettings();
   // Serial.print("Formatting FS...");
  //SPIFFS.format();
  //Serial.println("Done.");
    
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_portStr, 6);
    WiFiManagerParameter custom_mqtt_username("username", "mqtt username", mqtt_username, 20);
    WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 20);
    WiFiManagerParameter custom_mqtt_topic1("topic1", "mqtt topic1", mqtt_topic1, 20);
    WiFiManagerParameter custom_mqtt_topic2("topic2", "mqtt topic2", mqtt_topic2, 20);
    WiFiManagerParameter custom_mqtt_topic3("topic3", "mqtt topic3", mqtt_topic3, 20);
    WiFiManagerParameter custom_mqtt_topic4("topic4", "mqtt topic4", mqtt_topic4, 20);

    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_username);
    wifiManager.addParameter(&custom_mqtt_password);
    wifiManager.addParameter(&custom_mqtt_topic1);
    wifiManager.addParameter(&custom_mqtt_topic2);
    wifiManager.addParameter(&custom_mqtt_topic3);
    wifiManager.addParameter(&custom_mqtt_topic4);
    
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set static ip
  IPAddress _ip,_gw,_sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);

  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  
  //reset settings - for testing
  //Serial.println("Reset Wifi Settings, For testing.");
  //wifiManager.resetSettings();

  //set minimum quality of signal so it ignores AP's under that quality //defaults to 8%
  wifiManager.setMinimumSignalQuality(30);
  
  if (!wifiManager.autoConnect("EOS", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  //if you get here you have connected to the WiFi  
  Serial.println("connected...yeey :)");

if (shouldSaveConfig) {
      // read the updated parameters
      strcpy(mqtt_server, custom_mqtt_server.getValue());
      strcpy(mqtt_portStr, custom_mqtt_port.getValue());
      mqtt_port = atoi(mqtt_portStr);
      strcpy(mqtt_username, custom_mqtt_username.getValue());
      strcpy(mqtt_password, custom_mqtt_password.getValue());
      strcpy(mqtt_topic1, custom_mqtt_topic1.getValue());
      strcpy(mqtt_topic2, custom_mqtt_topic2.getValue());
      strcpy(mqtt_topic3, custom_mqtt_topic3.getValue());
      strcpy(mqtt_topic4, custom_mqtt_topic4.getValue());
      
      saveConfigJson();
      shouldSaveConfig = false;
    }
}
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  
  Button();
  server.begin();
  mqttClient.setServer(mqtt_server, mqtt_port);
}
  void reconnect() {

  // Loop until we're reconnected
  while (!mqttClient.connected() ) {
    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqtt_server);
    Serial.println("...");
    // Attempt to connect
    char mqtt_clientid[15];
    snprintf (mqtt_clientid, 14, "ESP%u", chipid);

    if (mqttClient.connect(mqtt_clientid, mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected.");
      long rssi = WiFi.RSSI();
      
      // send proper JSON startup message
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
      json["id"] = chipid;
      json["rssi"] = rssi;
      json["message"] = "Home Automation start up";
      char buf[110];
      json.printTo(buf, sizeof(buf));

      Serial.print("Publish message: ");
      Serial.println(buf);

      char topic_buf[50];
      sprintf(topic_buf, mqtt_topic1, chipid);
      mqttClient.publish(topic_buf, buf);
    } 
    
    else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
   }//end while
  }//end setup

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();

 // make sure the MQTT client is connected
  if (!mqttClient.connected()) {
    reconnect();
    
    mqttClient.subscribe(mqtt_topic1);
    mqttClient.subscribe(mqtt_topic2);
    mqttClient.subscribe(mqtt_topic3);
    mqttClient.subscribe(mqtt_topic4);
  } //endif
  mqttClient.loop(); // Need to call this, otherwise mqtt breaks 

}//end loop
void Button()
{
   webPage += "<p>This button controls GPIO5, Pin 20. <p>-" + String(static_ip) + "-<p>Socket #1 <a href=\"socket1On\"><button>ON</button></a>&nbsp;<a href=\"socket1Off\"><button>OFF</button></a></p>";
            // send to someones browser when asked
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });
  server.on("/socket1On", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(pgio5_pin, HIGH);
    delay(1000);
  });
  server.on("/socket1Off", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(pgio5_pin, LOW);
    delay(1000); 
  });
}

