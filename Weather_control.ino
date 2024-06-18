#include <DHT.h>  // Including library for dht

//Including library for WiFi 
#include <WiFi.h>
#include <WiFiClient.h>

#include <FirebaseESP32.h>   // Including library for firebase

const char *ssid =  "";  // wifi ssid 
const char *pass =  ""; // wifi password

//Pins where the dht11 is connected
#define DHTPIN 18 

//Pins connected to soilMoisture sensors
#define SoilMoistureSensorPin 36


//Pins of Relay (Appliances Control)
#define R1 5
#define R2 17
#define R3 16

// Firebase Credentials
#define firebase_host ""
#define firebase_auth ""
#define user "" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

//Variable Declaration
int onlineMaxiRelativeHumidity,onlineMiniRelativeHumidity, onlineMaxiTemperature, onlineMiniTemperature, onlineMaxiSoilMoisture, onlineMiniSoilMoisture; 
int offlineMaxiRelativeHumidity = 70;
int offlineMiniRelativeHumidity = 50;
int offlineMaxiTemperature = 26;
int offlineMiniTemperature = 18;
int offlineMaxiSoilMoisture = 75;
int offlineMiniSoilMoisture = 70;

int x, y;

// Function Declaration
void getSetConditions();
void onlineTemperatureHumidity();
void offlineTemperatureHumidity();
void onlineSoilMoisture();
void offlineSoilMoisture();
void uploadCurrentConditions();

//Assigning the dtht11 pin and also specification of dht type
DHT dht(DHTPIN, DHT11);

WiFiClient client;

void setup() {
 // Setup code
  Serial.begin(115200); //Serial monitor baud rate
  WiFi.begin(ssid, pass); //WiFi connection 

   delay(10); 

   Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  //Assign the api key (required)
  config.api_key = firebase_auth;

  config.database_url = firebase_host;

  //Firebase connection
  Firebase.begin(firebase_host, firebase_auth); 
  //Setting reattempt connection to firebase
  Firebase.reconnectWiFi(true); 

  Firebase.setDoubleDigits(5);
  
  delay(10);

   //Setting up dht11
  dht.begin(); 

  //Setting relay pins as output pins
  pinMode(R1, OUTPUT); //Light relay switch
  pinMode(R2, OUTPUT); //Motor relay switch
  pinMode(R3, OUTPUT); //Water pump relay switch

}

void loop() {
  //Checking if no internet connection 
   if (WiFi.status() != WL_CONNECTED)
  { 
    Serial.println("Not Connected");
    Serial.println();  
    Serial.print("Offline maxi relative humidity:");
    Serial.println(offlineMaxiRelativeHumidity);
    Serial.print("Offline mini relative humidity:");
    Serial.println(offlineMiniRelativeHumidity);
    Serial.print("Offline maxi temperature: ");
    Serial.println(offlineMaxiTemperature);
    Serial.print("Offline mini temperature: ");
    Serial.println(offlineMiniTemperature);
    Serial.print("Onffline maxi soil moisture: ");
    Serial.println(offlineMaxiSoilMoisture);
    Serial.print("Onffline mini soil moisture: ");
    Serial.println(offlineMiniSoilMoisture);
    offlineSoilMoisture();
    offlineTemperatureHumidity();
   
  }

  //ELse if there is internet connection
  else 
  {
    //if (DEBUG_SW) 
    Serial.println(" Connected");
     Serial.println(); 
     getSetConditions();
     onlineSoilMoisture();
     onlineTemperatureHumidity();
  }
} 

//Function to get set conditions from firebase
void getSetConditions()
{
  if (Firebase.ready()) 
  {
    
    //Firebase.setInt(fbdo, main, 5);
    Firebase.setInt(fbdo, user"/currentHumdity", x);
    Firebase.setInt(fbdo, user"/currentTemperature", y);
    delay(200);

    //Fetching the maximum relative humidity for firebase
    Serial.printf("Get maxi humidity--  %s\n", Firebase.getInt(fbdo, user"/maxi_relative_humidity") ? fbdo.to<String>().c_str() : fbdo.errorReason().c_str());
     onlineMaxiRelativeHumidity=fbdo.to<int>();

     //Fetching the minimum relative humidity for firebase
     Serial.printf("Get mini humidity--  %s\n", Firebase.getInt(fbdo, user"/mini_relative_humidity") ? fbdo.to<String>().c_str() : fbdo.errorReason().c_str());
     onlineMiniRelativeHumidity=fbdo.to<int>();

     //Fetching the maximum temeperature for firebase
    Serial.printf("Get maxi temperature--  %s\n", Firebase.getInt(fbdo, user"/maxi_temperature") ? fbdo.to<String>().c_str() : fbdo.errorReason().c_str());
     onlineMaxiTemperature=fbdo.to<int>();

     //Fetching the minimum temperature for firebase
     Serial.printf("Get mini temperature--  %s\n", Firebase.getInt(fbdo, user"/mini_temperature") ? fbdo.to<String>().c_str() : fbdo.errorReason().c_str());
     onlineMiniTemperature=fbdo.to<int>();

     //Fetching the maximum soil moisture for firebase
     Serial.printf("Get maxi soilMoisture--  %s\n", Firebase.getInt(fbdo, user"/maxi_soil_moisture") ? fbdo.to<String>().c_str() : fbdo.errorReason().c_str());
     onlineMaxiSoilMoisture=fbdo.to<int>();

     //Fetching the minimum soil moisture for firebase
     Serial.printf("Get mini soilMoisture--  %s\n", Firebase.getInt(fbdo, user"/mini_soil_moisture") ? fbdo.to<String>().c_str() : fbdo.errorReason().c_str());
      onlineMiniSoilMoisture=fbdo.to<int>();
  
    Serial.println();
    Serial.println("------------------");
    Serial.println();

    //Printing out the set conditions from firebase
    Serial.println();  
    Serial.print("Online maxi relative humidity:");
    Serial.println(onlineMaxiRelativeHumidity);
    Serial.print("Online mini relative humidity:");
    Serial.println(onlineMiniRelativeHumidity);
    Serial.print("Online maxi temperature: ");
    Serial.println(onlineMaxiTemperature);
     Serial.print("Online mini temperature: ");
    Serial.println(onlineMiniTemperature);
    Serial.print("Online maxi soil moisture: ");
    Serial.println(onlineMaxiSoilMoisture);
    Serial.print("Online mini soil moisture: ");
    Serial.println(onlineMiniSoilMoisture);
    delay(2500);
  }  
}


void onlineTemperatureHumidity()
{
   //Variable declaration and assignment to the temperature and humidity condition taken by dht11 sensor
   float humidity = dht.readHumidity();
   float temperature = dht.readTemperature();

   //Cheking the funtionality of dht11
   if (isnan(humidity) || isnan(temperature)) 
   {
     Serial.println("Failed to read from DHT sensor!");
     return;
   }
   else
   {
    Firebase.setString(fbdo, user"/current_humidity", humidity);
    Firebase.setString(fbdo, user"/current_temperature", temperature);
    delay(200);

    //Printing out values from dht11 sensor
    Serial.print("The humidity) = ");
    Serial.print(humidity);
    Serial.println("% RH");
    Serial.print("The temperature) = ");
    Serial.print(temperature); 
    Serial.println("C");

    //Checking if current temperature is greater than online set maximum temperature
    if(temperature > onlineMaxiTemperature) {
      //Switching on fans
      digitalWrite(R2, HIGH); 
    }

    //Checking if current temperature is less than or equal to online set maximum temperature and if its greater than or equals to online set minimum temperature
    else if(temperature <= onlineMaxiTemperature && temperature >= onlineMiniTemperature) {
      //Switching off fans
      digitalWrite(R2, LOW);
    }

    //Checking if temperature is less that set online minimum temperature or humidity is greater than online set maximum temperature 
    else if(temperature < onlineMiniTemperature || humidity > onlineMaxiRelativeHumidity){
      //Switching on light
      digitalWrite(R1, HIGH);
    }

    //Checking if current temperature is less than or equal to online set maximum relative humidity and if its greater than or equals to online set minimum humidity
    else if(humidity <= onlineMaxiRelativeHumidity && humidity >= onlineMiniRelativeHumidity){
      //Switching off light
      digitalWrite(R1, LOW);
    }

    //Checking if humidity is less than online set minimum relative humidity
    else if(humidity < onlineMiniRelativeHumidity){
      //Switching off light
      digitalWrite(R1, LOW);
    }
   }
}

void offlineTemperatureHumidity()
{
   //Variable declaration and assignment to the temperature and humidity condition taken by dht11 sensor
   float humidity = dht.readHumidity();
   float temperature = dht.readTemperature(); 

   //Cheking the funtionality of dht11
   if (isnan(humidity) || isnan(temperature)) 
   {
      //Printing out an error message for dht11 sensor failure
     Serial.println("Failed to read from DHT sensor!");
     return;
   }
   else
   {
    //Printing out values from dht11 sensor
    Serial.print("The humidity) = ");
    Serial.print(humidity);
    Serial.println("% RH");
    Serial.print("The temperature) = ");
    Serial.print(temperature); 
    Serial.println("C");

    //Checking if current temperature is greater than offline set maximum temperature
    if(temperature > offlineMaxiTemperature) {
      //Switching on fans
      digitalWrite(R2, HIGH);
    }

    //Checking if current temperature is less than or equal to offline set maximum temperature and if its greater than or equals to offline set minimum temperature
    else if(temperature <= offlineMaxiTemperature && temperature >= offlineMiniTemperature) {
      //Switching off fans
      digitalWrite(R2, LOW);
    }

    //Checking if temperature is less that set offline minimum temperature or humidity is greater than offline set maximum temperature
    else if(temperature < offlineMiniTemperature || humidity > offlineMaxiRelativeHumidity){
      //Switching on lights
      digitalWrite(R1, HIGH);
    }

    //Checking if current temperature is less than or equal to offline set maximum relative humidity and if its greater than or equals to offline set minimum humidity
    else if(humidity <= offlineMaxiRelativeHumidity && humidity >= offlineMiniRelativeHumidity){
      //Switching off lights
      digitalWrite(R1, LOW);
    }

    //Checking if humidity is less than offline set minimum relative humidity
    else if(humidity < offlineMiniRelativeHumidity){
      //Switching off lights
      digitalWrite(R1, LOW);
    }
   }
}

void onlineSoilMoisture()
{  

    //Variable declaration and assignment of soil moisture in percentage
    int moisture_percentage = ( 100.00 - ( (analogRead(SoilMoistureSensorPin)/1023.00) * 100.00 ) );

    //Printing out the soil moisture
    Serial.print("Soil Moisture(in Percentage) = ");
    Serial.print(moisture_percentage);
    Serial.println("%");
    
    //Cheking if soil moisture is less than set online minimum soil moisture
    if(moisture_percentage < onlineMiniSoilMoisture) {
      //Switching on water pump
      digitalWrite(R3, HIGH);
    }

    //Checking if soil moisture is greater or equals to online set soil minimum moisture and is less than or equals to online set maximum soil moisture
    else if(moisture_percentage >= onlineMiniSoilMoisture && moisture_percentage <= onlineMaxiSoilMoisture){
      //Switching off water pump
      digitalWrite(R3, LOW);
    }

    //Checking if soil moisture is greater than online set maximum soil moisture
    else if(moisture_percentage > onlineMaxiSoilMoisture){
      //Switching off water pump
      digitalWrite(R3, LOW);
    }
}

void offlineSoilMoisture()
{
  
    //Variable declaration and assignment of soil moisture in percentage
    int moisture_percentage = ( 100.00 - ( (analogRead(SoilMoistureSensorPin)/1023.00) * 100.00 ) );

    //Printing out the soil moisture
    Serial.print("Soil Moisture(in Percentage) = ");
    Serial.print(moisture_percentage);
    Serial.println("%");

    //Cheking if soil moisture is less than set offline minimum soil moisture
    if(moisture_percentage < offlineMiniSoilMoisture) {
      //Switching on water pump
      digitalWrite(R3, HIGH);
    }

    //Checking if soil moisture is greater or equals to offline set soil minimum moisture and is less than or equals to offline set maximum soil moisture
    else if(moisture_percentage >= offlineMiniSoilMoisture && moisture_percentage <= offlineMaxiSoilMoisture){
      //Switching off water pump
      digitalWrite(R3, LOW);
    }

    //Checking if soil moisture is greater than offline set maximum soil moisture
    else if(moisture_percentage > offlineMaxiSoilMoisture){
      //Switching off water pump
      digitalWrite(R3, LOW);
    }
    
}

 
