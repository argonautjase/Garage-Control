#include <ESP8266WiFi.h> // Wifi
#include <PubSubClient.h> // MQTT


// Define the pins 
#define RELAY 5 // Relay connected to GPIO5
#define SENSOROPEN 4 // Pin D2 - Sensor to detect Open GPIO4
#define SENSORCLOSED 12 // Pin D6 - Seensor to detect Closed GPIO12

//Network setup
#define HOSTNAME "garage-door"
const char* ssid     = "YOURSSIDHERE";
const char* password = "YOURWIFIPASSWORDHERE";
WiFiClient espClient;
PubSubClient client(espClient);

//MQTT setup
const char* mqtt_server = "YOURMQTTSERVERIPHERE";
const PROGMEM char *eventTopic = "RL/esp8266-1/garagedoor1/activate";
const PROGMEM char *stateTopic = "RL/esp8266-1/garagedoor1/status";

long lastMsg = 0;
char mssg[50];
int value = 0;

int SENSOROPENSTATE = 0;         // current state of the button
int LASTSENSOROPENSTATE = 0;     // previous state of the button
int SENSORCLOSEDSTATE = 0;         // current state of the button
int LASTSENSORCLOSEDSTATE = 0;     // previous state of the button
const char* door_state = "closed";


void setup() {

//configure pins
pinMode(RELAY,OUTPUT);    // Configure Output for Relay
digitalWrite(RELAY, LOW); // Set inital state of Relay to Low / OFF
pinMode(SENSOROPEN,INPUT_PULLUP); // Configure Input for Open Sensor
pinMode(SENSORCLOSED,INPUT_PULLUP); // Configure Input for Closed Sensor

//Configure Serail port for monitor
Serial.begin(115200); // Set the BaudRate of the serial port for the monitor tool for debug

// Call wifi configuration
setup_wifi();

// configure MQTT
client.setServer(mqtt_server, 1883);
client.setCallback(callback);

}


void setup_wifi() {   //WIFI connect
  delay(10);

//  Serial.println(" Connecting to ");
//  Serial.println("ssid");

  //Set Hostname
  String hostname(HOSTNAME);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
//    Serial.print(".");
  }

//   Serial.println("");
//   Serial.println("WiFi connected");
//   Serial.println("IP address: ");
//   Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length){
//  Serial.print("Message arrived on topic: ");
//  Serial.print(topic);
//  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++){
 //    Serial.print((char)message[i]);
     messageTemp += (char)message[i];
  }

//  Serial.println();

  if(String(topic) == eventTopic){
//    Serial.print("Activating Door ");
    if(messageTemp == "on"){
//      Serial.println("moving door");
      digitalWrite(RELAY, HIGH); // turn the Relay on (HIGH is the voltage level)
      delay(1000); // wait for a second , (1000milliseconds = 1 sec)
      digitalWrite(RELAY, LOW); // turn the Relay on (HIGH is the voltage level)
    
  }
  else if (messageTemp =="status"){
//    Serial.println("checking status");
    checkstate();
  }
  }
}

void checkstate() {
      // if the state has changed, increment the counter
    if ((SENSORCLOSEDSTATE == HIGH )&& (SENSOROPENSTATE == LOW)) {
      // if the current state is HIGH then the button went from on to off:
//      Serial.println("OPEN");
      door_state = "Open";
      client.publish(stateTopic, door_state);
    } else {
     if ((SENSORCLOSEDSTATE == LOW )&& (SENSOROPENSTATE == HIGH)) {
      // if the current state is HIGH then the button went from on to off:
//      Serial.println("CLOSED");
      door_state = "Closed";
      client.publish(stateTopic, door_state);
    }else {
     if ((SENSORCLOSEDSTATE == HIGH )&& (SENSOROPENSTATE == HIGH)) {
      // if the current state is HIGH then the button went from on to off:
//      Serial.println("Door Moving...........");
      door_state = "Door Moving....";
      client.publish(stateTopic, door_state);
     } 
   
    // Delay a little bit to avoid bouncing
    delay(1000);
    }
  }

 }
 

void reconnect(){
  // Loop until we are connected
  while (!client.connected()){
//    Serial.print("Attempting MQTT connection...");
    //Attempt to connect
    if (client.connect("HOSTNAME")){
//     Serial.print("connected");
     //Subscribe
      client.subscribe(eventTopic);
    } else {
//      Serial.print("failed, rc=");
//      Serial.print(client.state());
//      Serial.println(" try again in 5 seconds");
      // wait 5 seconds before retry
      delay(5000);
    }
    }
  }
 
void activaterelay(){
 digitalWrite(RELAY, HIGH); // turn the Relay on (HIGH is the voltage level)
 delay(1000); // wait for a second , (1000milliseconds = 1 sec)
 digitalWrite(RELAY, LOW); // turn the Relay on (HIGH is the voltage level)
}

void sensorstate(){
// read the pushbutton input pin:
  SENSOROPENSTATE = digitalRead(SENSOROPEN);
  SENSORCLOSEDSTATE = digitalRead(SENSORCLOSED);
  
  // compare the Open Sensor state to its previous state
  if ((SENSORCLOSEDSTATE != LASTSENSORCLOSEDSTATE) or (SENSOROPENSTATE != LASTSENSOROPENSTATE)){
    // if the state has changed, increment the counter
    if ((SENSORCLOSEDSTATE == HIGH )&& (SENSOROPENSTATE == LOW)) {
      // if the current state is HIGH then the button went from on to off:
//      Serial.println("OPEN");
      door_state = "Open";
      client.publish(stateTopic, door_state);
    } else {
     if ((SENSORCLOSEDSTATE == LOW )&& (SENSOROPENSTATE == HIGH)) {
      // if the current state is HIGH then the button went from on to off:
//      Serial.println("CLOSED");
      door_state = "Closed";
      client.publish(stateTopic, door_state);
    }else {
     if ((SENSORCLOSEDSTATE == HIGH )&& (SENSOROPENSTATE == HIGH)) {
      // if the current state is HIGH then the button went from on to off:
//      Serial.println("Door Moving...........");
      door_state = "Door Moving....";
      client.publish(stateTopic, door_state);
     } 
   
    // Delay a little bit to avoid bouncing
    delay(1000);
    }
  }

 }
  
  // save the current state as the last state, for next time through the loop
  LASTSENSORCLOSEDSTATE = SENSORCLOSEDSTATE;
  LASTSENSOROPENSTATE = SENSOROPENSTATE;
}



void loop() {

if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
sensorstate();

 }
}
