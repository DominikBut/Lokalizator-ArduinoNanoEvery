//Dominik But 
//System lokalizacji pojazdów bazujący na platformie Arduino i technologii Laravel
//Lokalizator pojazdu - kod programu mikrokontrolera Arduino Nano Every
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String apn = "internet"; // Dane logowanie do sieci
String url = "https://example.pl/api/data";  // URL serwera
SoftwareSerial SIM800(3, 2); // RX, TX
SoftwareSerial GPSNeo(8, 9); // RX, TX
TinyGPSPlus gps;
String SIMID = "";

void gsm_send_serial(String command) {
  Serial.println("S: " + command);
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("S: ");
  delay(100);
  SIM800.println(command);
  String response = "";
  String data ="";
  long wtimer = millis();
  while (wtimer + 6000 > millis()) {
    while (SIM800.available()) {
      char c = SIM800.read();
      Serial.print(c);
      data.concat(c);
      
    }
  }
  display.setCursor(0, 20);
  display.println(data);
  display.display();
}
boolean getResponse(String expected_answer, String expected_2, unsigned int timeout = 5000){
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Check: ");
  boolean flag = false;
  String response = "";
  unsigned long previous;
  //*************************************************************
  for(previous=millis(); (millis() - previous) < timeout;){
    while(SIM800.available()){
      response = SIM800.readString();
      if(response.indexOf(expected_answer) > 0 or response.indexOf(expected_2) > 0){
        flag = true;
        goto OUTSIDE;
      }
    }
  }
  //*************************************************************
  OUTSIDE:
  if(response != ""){
    Serial.println(response);
    display.setCursor(0, 20);
    display.println(response);
    display.display();
    }
  return flag;
}
String sim_get_id(String command) {
  String data ="";
  Serial.println("Get: " + command);
  delay(100);
  SIM800.println(command);
  long wtimer = millis();
  while (wtimer + 5000 > millis()) {
    while (SIM800.available()) {
      char c = SIM800.read();
      Serial.print(c);
      if(isdigit(c) && data.length() <9)
      {
        data.concat(c);
      }
    }
  }
  Serial.println();
  return data;
}
String sim_get_battery(String command) {
  String data ="";
  Serial.println("Get: " + command);
  delay(100);
  SIM800.println(command);
  long wtimer = millis(); //mozna zwiekszyc
  while (wtimer + 5000 > millis()) {
    while (SIM800.available()) {
      char c = SIM800.read();
      Serial.print(c);
      data.concat(c);
    }
  }
  data = data.substring(data.indexOf(',')+1,data.lastIndexOf(','));
  Serial.println();
  return data;
}
String sim_get_signal(String command) {
  String data ="";
  Serial.println("Get: " + command);
  delay(100);
  SIM800.println(command);
  long wtimer = millis();
  while (wtimer + 5000 > millis()) {
    while (SIM800.available()) {
      char c = SIM800.read();
      Serial.print(c);
      data.concat(c);
    }
  }
  data = data.substring(data.indexOf(':')+2,data.lastIndexOf(','));
  Serial.println();
  return data;
}
void gsm_http_post(String postdata) {
  Serial.println(" --- Wysylanie HTTP POST --- ");
  gsm_send_serial("AT+CIPSHUT"); // wylaczanie polaczenia sieciowego
  delay(2000);
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("Przygotowywanie POST");
  display.display();
  gsm_send_serial("AT+SAPBR=0,1");
  gsm_send_serial("AT+SAPBR=3,1,\"Contype\",\"GPRS\""); // ustawianie profilu polaczenia sieciowego
  gsm_send_serial("AT+SAPBR=3,1,\"APN\",\"" + apn + "\""); // ustawiania danych logowania do sieci
  gsm_send_serial("AT+SAPBR=1,1"); // wlaczenie profilu polaczenia sieciowego
  delay(5000);
  gsm_send_serial("AT+SAPBR=2,1"); // sprawdzenie poprawnosci polaczenia sieciowego
  gsm_send_serial("AT+HTTPINIT"); // tworzenie obiektu HTTP
  gsm_send_serial("AT+HTTPSSL=1");
  gsm_send_serial("AT+HTTPPARA=CID,1");
  gsm_send_serial("AT+HTTPPARA=URL," + url); // ustawianie adresu celu przesylania danych
  gsm_send_serial("AT+HTTPPARA=CONTENT,application/json"); // ustawianie typu danych przesylania
  gsm_send_serial("AT+HTTPDATA=256,10000");
  gsm_send_serial(postdata); // ladowanie danych do obiektu
  delay(1000);
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("POST wysylanie");
  display.display();
  gsm_send_serial("AT+HTTPACTION=1"); //ustawienie typu zadania na POST
  delay(10000);
  gsm_send_serial("AT+HTTPREAD"); // odczytywanie odpowiedzi od API
  delay(5000);
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("POST zakonczono");
  display.display();
  gsm_send_serial("AT+HTTPTERM"); // usuwanie obiektu HTTP
  gsm_send_serial("AT+SAPBR=0,1");
  Serial.println(" --- HTTP POST ukonczony --- ");
  
 
}
void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.println("Startowanie...");
  display.display(); 
  Serial.println("Startowanie...");
  delay(10000);
  GPSNeo.begin(9600);
  SIM800.begin(9600);
  Serial.println(" --- Test polaczenia GSM --- ");
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("Test GSM...");
  display.display(); 
  while(true)
  {
    SIM800.println("AT+CREG?");
    if(getResponse("0,1","0,5",6000) == true)
    {
      Serial.println("==> Siec GSM aktywna");
      display.clearDisplay();
      display.setCursor(0, 10);
      display.println("Siec aktywna");
      display.display(); 
      break;
    }
    else
    {
      Serial.println("...");
      display.clearDisplay();
      display.setCursor(0, 20);
      display.println("Szukanie sieci...");
      display.display(); 
    }
    delay(2000);
  }
  Serial.println(" --- Polaczenie GSM aktywne --- ");
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("Konfigurowanie modulow");
  display.display();
  gsm_send_serial("AT+IPR=9600"); // zmiana predkosci UART
  gsm_send_serial("AT+CMGF=1");
  gsm_send_serial("AT+CNMI=1,2,0,0,0");
  gsm_send_serial("AT+GSMBUSY=1");
  //gsm_send_serial("AT&W"); // zapis w pamieci EPROM
  delay(1000);
  SIMID = sim_get_id("AT+CCID");
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("ID: ");
  display.setCursor(0, 20);
  display.println(SIMID);
  display.display();
  //gsm_send_serial("AT+CMGDA=\"DEL ALL\""); // usuwanie SMSow
  delay(5000);
  
}

void loop() {
  boolean newData = false;
  String latitude="", longitude="";
  Serial.println("Oczekiwanie na GPS...");
  Serial.println("");
  GPSNeo.listen();
  //GPS
  Serial.println(" --- Pozyskiwanie danych GPS --- ");
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Oczekwianie na GPS...");
  display.display();
  long wtimer = millis();
  while (wtimer + 6000 > millis()) {
    while (GPSNeo.available() > 0)
    {
      if (gps.encode(GPSNeo.read()))
      {
        if (gps.location.isValid())
        {
          latitude = String(gps.location.lat(), 6);
          longitude = String(gps.location.lng(), 6);
          Serial.println("Latitude: "+ latitude +" | Longitude: "+ longitude);
          display.clearDisplay();
          display.setCursor(0, 20);
          display.println("Latitude: "+ latitude);
          display.setCursor(0, 30);
          display.println("Longitude: "+ longitude);
          display.display();
          newData = true;
          break;
        }
        else
        {
          Serial.println("==> Brak danych GPS");
          display.clearDisplay();
          display.setCursor(0, 20);
          display.println("Brak danych GPS");
          display.display();
        }
        delay(1000);
      }
    }
  }
  Serial.println(" --- GPS zakonczone --- ");
  SIM800.listen(); //zmiana aktualnie nasluchiwanego portu szeregowego
  delay(5000);
  //GPRS
  if(newData==true)
  {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Pozyskano dane:");
    display.display();
    display.setCursor(0, 20);
    display.println("Latitude: "+ latitude);
    display.setCursor(0, 30);
    display.println("Longitude: "+ longitude);
    display.display();
    Serial.println(" --- Test polaczenia GSM --- ");
    while(true)
    {
      SIM800.println("AT+CREG?");
      if(getResponse("0,1","0,5",6000) == true)
      {
        Serial.println("==> Siec GSM aktywna");
        display.clearDisplay();
        display.setCursor(0, 10);
        display.println("Siec aktywna");
        display.display(); 
        break;
      }
      else
      {
        Serial.println("...");
        display.clearDisplay();
        display.setCursor(0, 20);
        display.println("Szukanie sieci...");
        display.display(); 
      }
      delay(2000);
    }
    Serial.println(" --- Polaczenie GSM aktywne --- ");
    delay(5000);
    //String bat = sim_get_battery("AT+CBC"); // brak tej funkcjonalnosci
    String bat = "0";
    delay(5000);
    String signal = sim_get_signal("AT+CSQ");
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Info:");
    display.setCursor(0, 20);
    display.println("Bateria: "+ bat);
    display.setCursor(0, 30);
    display.println("Zasieg: "+ signal);
    display.display();
    Serial.println("");
    delay(5000);
    Serial.println("==> Proba przesylania danych przez GPRS...");
    delay(1000);
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("POST request:");
    display.setCursor(0, 20);
    display.println("Proba wysylania POST...");
    display.display();
    gsm_http_post("{\"id\": "+ SIMID +", \"bat\": "+ bat +", \"signal\": "+ signal +", \"lat\": "+ latitude +", \"lng\": "+ longitude +"}");
  }
  delay(10000);
}