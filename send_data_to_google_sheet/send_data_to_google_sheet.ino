#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
 #include <LiquidCrystal_I2C.h>.

// Fingerprint for demo URL, expires on ....., needs to be updated well before this date
const uint8_t fingerprint[20] = {...Enter authentication key};
//25 36 C1 2F 11 C0 8D 64 42 57 0A 96 50 CP 72 2E DC E9 8F 22

#define RST_PIN  D3     // Configurable, see typical pin layout above
#define SS_PIN   D4     // Configurable, see typical pin layout above
#define BUZZER   D2     // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;  
ESP8266WiFiMulti WiFiMulti;
MFRC522::StatusCode status;      

/* Be aware of Sector Trailer Blocks */
int blockNum = 2;  

/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];

String data2;
String MessCode;
String Lcdmessage;
const String data1 = "https://script.google.com/macros/s/AK.....";

//LCD command--------------------------------
LiquidCrystal_I2C lcd(0x27, 20, 4); 

void setup() 
{
  /* Initialize serial communications with the PC */
  Serial.begin(9600);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) 
  {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  
  /* Put your WIFI Name and Password here */
  WiFiMulti.addAP("Suraj", "...");

  /* Set BUZZER as OUTPUT */
  pinMode(BUZZER, OUTPUT);
  /* Initialize SPI bus */
  SPI.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
}

void loop()
{
  
  /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  /* Read data from the same block */
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));
  ReadDataFromBlock(blockNum, readBlockData);
  /* If you want to print the full memory dump, uncomment the next line */
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  
  /* Print the data read from block */
  Serial.println();
  Serial.print(F("Last data in RFID:"));
  Serial.print(blockNum);
  Serial.print(F(" --> "));
  for (int j=0 ; j<16 ; j++)
  {
    Serial.write(readBlockData[j]);
  }
  Serial.println();
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  
    if ((WiFiMulti.run() == WL_CONNECTED)) 
  {
    
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

   //client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
     client->setInsecure();
     MessCode =String((char*)readBlockData);
     Lcdmessage ="Messcode="+MessCode;
     lcd.setCursor(3,0);
     lcd.print(Lcdmessage);

    data2 = data1 + MessCode ;
    data2.trim();
    Serial.println(data2);
    
    HTTPClient https;
    Serial.print(F("[HTTPS] begin...\n"));
    if (https.begin(*client, (String)data2))
    {  
      // HTTP
      Serial.print(F("[HTTPS] GET...\n"));
      // start connection and send HTTP header
      int httpCode = https.GET();
    
      // httpCode will be negative on error
      if (httpCode > 0) 
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        lcd.setCursor(1,1);
         lcd.print("AttendanceMarked");
//         for (int i=0; i<=500; i++)
//         {
//         lcd.scrollDisplayLeft();
//
//         delay(350);
//         }
        // file found at server
      }
      else 
      {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
         lcd.setCursor(3,0);
         lcd.print("SLOW NET!!");
         lcd.setCursor(7,1);
         lcd.print(httpCode);
      }
      https.end();
//      delay(1000);
    } 
    else 
    {
      Serial.printf("[HTTPS} Unable to connect\n");
      lcd.setCursor(7,0);
      lcd.print("Wifi");
      lcd.setCursor(1,1);
      lcd.print("Not connected");
    }
  }
}

void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{ 
  lcd.clear();
  /* Prepare the ksy for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK)
  {
     Serial.print("Authentication failed for Read: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
     return;
  }
  else
  {
    Serial.println("Authentication success");
  }

  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Block was read successfully");  
  }
}
