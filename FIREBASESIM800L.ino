#define TINY_GSM_MODEM_SIM800
#include <DHT.h>
#include <HTTPClient.h>
#include <TinyGsmClient.h>


#define SerialMon Serial
#define SerialAT Serial1

#define TINY_GSM_RX_BUFFER 1024
//#endif
#define TINY_GSM_DEBUG SerialMon
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26

const char apn[]  = "internet.tn";// CHANGES: Add your apn from the service provider of your simcard
const char gprsUser[] = "";
const char gprsPass[] = "";

#define DHTPIN 4  // DHT22 data pin connected to GPIO 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

#define FIREBASE_HOST "station-d-irrigation-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "ZPolzI******************"

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

TinyGsmClient client(modem);
 


void setup() {
  Serial.begin(115200);

  // Connect to APN
  SerialMon.begin(115200);
  delay(10);

  
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  SerialMon.println("Wait...");

  // Set GSM module baud rate
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  SerialMon.println("Initializing modem...");
  modem.init();
  SerialMon.print(F("Connecting to "));
   SerialMon.print(apn);
   if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      SerialMon.println(" fail");
      delay(10000);
      return;
    }
   SerialMon.println(" success");

   if (modem.isGprsConnected()) {
      SerialMon.println("GPRS connected");
  }

  //SerialMon.print("Connecting to ");
  //SerialMon.println(server);
  //if (!client.connect(server, port)) {
    //SerialMon.println(" fail");
   // delay(10000);
   // return;
  //}
  //SerialMon.println(" success");
  
  
//}
  
  // Initialize DHT sensor
  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements
  delay(2000);

  // Read temperature and humidity from the DHT sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if reading was successful
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.println(humidity);
  Serial.println(temperature);
  

  // Send data to Firebase
  String path = "/DHT22";
  String url = String("https://") + FIREBASE_HOST + path + ".json?auth=" + FIREBASE_AUTH;
  String data = "{\"humidity\": " + String(humidity) + ", \"temperature\": " + String(temperature) + "}";

  // Make an HTTP PUT request to Firebase
  HTTPClient http;
  http.begin(url, "DUMMY_FINGERPRINT");
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.PUT(data);
  delay(1000) ;
  http.end();

  // Check if the request was successful
  if (httpResponseCode > 0) {
    Serial.println("Data sent to Firebase successfully!");
  } else {
    Serial.println("Failed to send data to Firebase!");
    Serial.println("HTTP Response code: " + String(httpResponseCode));
  }
}
