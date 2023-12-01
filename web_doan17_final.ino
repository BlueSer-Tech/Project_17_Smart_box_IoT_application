#include <ESPAsyncWebServer.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <ESP_Mail_Client.h>
#include <LiquidCrystal_I2C.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <Stepper.h>


// WiFi credentials
const char* ssid = "Mastermind";         // change SSID
const char* password = "2001zone";    // change password

/** The smtp host name e.g. smtp.gmail.com for GMail or smtp.office365.com for Outlook or smtp.mail.yahoo.com */
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "ngominhkhoi1231@gmail.com"
#define AUTHOR_PASSWORD "gdob wvon nztp vcfg"

/* Recipient's email*/
#define RECIPIENT_EMAIL "hieu333.hunn@gmail.com"

/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

#include <HTTPClient.h>

// Google script ID and required credentials
String old_data_recieved="";
String GOOGLE_SCRIPT_ID = "AKfycbx6xVSCzEZwiUy16uS_pZhL2XBN_9Ti4v3RVRfnbTFa_c-4eXRhW0gDIGbW-o2fgk8";    // change Gscript ID

LiquidCrystal_I2C lcd(0x3F,16,2);

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 1; // Connects to module's RX 
static const uint8_t PIN_MP3_RX = 3; // Connects to module's TX 
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(16, 17); //SIM800L Tx & Rx is connected to Arduino #3 & #2

DFRobotDFPlayerMini player;

#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 23

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution

Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

int inputPin = 25;       // chọn ngõ tín hiệu vào cho PIR
int pirState = LOW;     // Bắt đầu với không có báo động
int val = 0;
int pinSpeaker = 26;    //chọn chân cho chuông khi có đột nhập

int state =0;

int relay = 15;
int IR1 = 4;
int IR2 = 0;
int value_IR1;
int value_IR2;
int counter=0;
int default_value1=1;
int default_value2=1;

int dem;
const int ledPin1 = 2;      // Pin kết nối đèn 1
bool led1State = false;
int begin_flag=0;

AsyncWebServer server(80);
// Nội dung HTML nhúng trực tiếp vào mã nguồn
const char index_html[] PROGMEM = R"rawliteral(
 <html><head>
   <meta charset="UTF-8">
   <meta name="viewport" content="width=device-width, initial-scale=1.0">
   <script>
   function toggleDevice(device) {
     var xhttp = new XMLHttpRequest()
     xhttp.onreadystatechange = function() {
       if (this.readyState == 4 && this.status == 200) {
         updateData()  // Làm mới dữ liệu sau khi thực hiện xong yêu cầu
       }
     }
     xhttp.open("GET", "/" + device, true)
     xhttp.send()
   }
   function updateButton(device, status) {
      var button = document.getElementById(device + "-button");
      if (status === "Bật") {
        button.checked = true;
      } else {
        button.checked = false;
      }
    }
   function updateData() {
     var xhttp = new XMLHttpRequest()
     xhttp.onreadystatechange = function() {
       if (this.readyState == 4 && this.status == 200) {
         var data = JSON.parse(this.responseText)
         document.getElementById("led1-status").innerHTML = data.led1Status
         updateButton("led1", data.led1Status)
       }
     }
     xhttp.open("GET", "/data", true)
     xhttp.send()
   }
   setInterval(updateData, 1000)  // Làm mới dữ liệu mỗi giây
   </script>
  <style>
    body {
    font-family: "Arial", sans-serif;
    text-align: center;
    margin: 0;
    background: linear-gradient(to right, #00d2ff, #3a7bd5); /* Gradient màu xanh da trời */
    }
    .container { display: grid ;grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); grid-gap: 20px ;padding: 20px }
    .device { background-color:#000;border: 4px solid #ccc;border-radius: 8px;color:white;height:100%;width: 80%;max-width: 600px; margin: 20px auto; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2); }
    .device h2 { color:white;font-size: 24px }
    .button { display: inline-block; color: white ;padding: 10px 20px ;border: 4px solid #ccc;border-radius: 8px;text-decoration: none ;border-radius: 5px ;font-size: 16px; margin-top: 10px ;cursor: pointer }
    .status { margin-top: 10px ;font-size: 18px }
    .video-block h2 { color: white; font-size: 24px }
    .video-block {background-color:#000;border: 4px solid #ccc;border-radius: 8px;overflow: hidden;width: 100%;max-width: 600px; margin: 20px auto; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);}
    .video-block img {display: inline-block;width: 99%;height: auto;} 
    .switch {position: relative;display: inline-block;width: 60px;height: 34px;}
    .switch input {opacity: 0;width: 0;height: 0;}.slider {position: absolute;cursor: pointer;top: 0;left: 0;right: 0;bottom: 0;background-color: #ccc;-webkit-transition: .4s;transition: .4s;border-radius: 34px;}
    .slider:before {position: absolute;content: "";height: 26px;width: 26px;left: 4px;bottom: 4px;background-color: white;-webkit-transition: .4s;transition: .4s;border-radius: 50%;}
    input:checked + .slider {background-color: green;}
    input:focus + .slider {box-shadow: 0 0 1px #2196F3;}
    input:checked + .slider:before {-webkit-transform: translateX(26px);-ms-transform: translateX(26px);transform: translateX(26px);}
   </style>
   </head><body>
   <h1 style="color: #ecf0f1">THÙNG HÀNG THÔNG MINH</h1>
   <div class="container">
      <div class="device">
        <h2>XÁC NHẬN MỞ CỬA</h2>
        <p>Trạng thái: <span id="led1-status">...</span></p>
        <label class="switch">
          <input type="checkbox" id="led1-button" class="button" onclick="toggleDevice('led1')">
          <span class="slider"></span>
        </label>
      </div>
  </div>
  <div class="video-block">
    <h2>Camera An Ninh</h2>
   <img alt="Video stream" src='http://192.168.1.26/mjpeg/1'>
  </div>
  </body></html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  pinMode(ledPin1, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    char response[100];
    strcpy_P(response, PSTR("{\"led1Status\":\""));
    strcat(response, (led1State ? "Bật" : "Tắt"));
    strcat_P(response, PSTR("\"}"));
    request->send(200, "application/json", response);
  });

  server.on("/led1", HTTP_GET, [](AsyncWebServerRequest *request) {
    led1State = !led1State;
 
    digitalWrite(ledPin1, led1State ? HIGH : LOW);
    request->send(200, "text/plain", "OK");
  });

  server.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("THCS HOANG VAN");
  lcd.setCursor(0,1);
  lcd.print("Moi ban quet ma");
  softwareSerial.begin(9600);
  if (player.begin(softwareSerial)) {
    Serial.println("OK");
    // Set volume to maximum (0 to 30).
    player.volume(30);
  } else {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }
  mySerial.begin(9600);

  Serial.println("Initializing..."); 
  delay(1000);
  mySerial.println("AT"); //Once the handshake test is successful, i t will back to OK
  updateSerial();

  myStepper.setSpeed(15);

  pinMode(relay,OUTPUT);
  digitalWrite (relay,LOW);
  pinMode(IR1,INPUT);
  digitalWrite(IR1,LOW);
  pinMode(IR2,INPUT);
  digitalWrite(IR2,LOW);

  pinMode(inputPin, INPUT);
	pinMode(pinSpeaker, OUTPUT);
}

void loop() {
  // Code trong loop nếu có
  val = digitalRead(inputPin);    // đọc giá trị đầu vào.
	if (val == HIGH && state == 0)                // nếu giá trị ở mức cao.(1)
	{
		// digitalWrite(ledPin, HIGH); // LED On
		playTone(300, 160);         // thời gian chuông kêu
		delay(150);
    state=1;
    player.play(5);
    delay(5000);

		if (pirState == LOW)
		{
			Serial.println("Motion detected!");
			pirState = HIGH;
		}
	}
	else
	{
		// digitalWrite(ledPin, LOW);
		playTone(0, 0);
		delay(300);
		if (pirState == HIGH)
		{
			Serial.println("Motion ended!");
			pirState = LOW;
		}
	}

  
  while(led1State == true)
          {
            open_door();
            IR();
            lcd_QR();
            led1State = false;
          }
  Check_MaNhanDon();
  Serial.println(led1State);
}


void Check_MaNhanDon(){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?read";
    //Serial.println("Making a request");
    http.begin(url.c_str()); //Specify the URL and certificate
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    String data_recieved;
    if (httpCode > 0) { //Check for the returning code
      data_recieved = http.getString();
      //Serial.println(httpCode);
      // Serial.println(data_recieved.length());
      String mavandon = data_recieved.substring(0,data_recieved.length()-4);
      // Serial.println(data_recieved.substring(data_recieved.length()-4,data_recieved.length()));
      // Serial.println(mavandon);
      if(mavandon != old_data_recieved && begin_flag == 1){
        if(data_recieved.substring(data_recieved.length()-3,data_recieved.length())=="Yes"){
          Serial.println("Yes!!!!!!");
          lcd_QR_successful();
          send_email(mavandon);
          calling();
          open_door();
          IR();
          lcd_QR();
        }
        if(data_recieved.substring(data_recieved.length()-3,data_recieved.length())==" No"){
          Serial.println("No!!!!!!");
          lcd_QR_failed();
          send_email(mavandon);
          calling();
          lcd_QR();
          //Hãy để code gmail vào đây
        }
      }
      old_data_recieved=mavandon;
    }
    else {
      Serial.println("Error on HTTP request");
    }
    http.end();
    begin_flag=1;
  }
}

///----------mail--------------------------

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port                                                                                                                                                                                                                                                                                                                                              
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)
      
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}

void send_email(String textMsg)
{
  MailClient.networkReconnect(true);

  /** Enable the debug via Serial port
   * 0 for no debugging
   * 1 for basic level debugging
   *
   * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
   */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the Session_Config for user defined session credentials */
  Session_Config config;

  /* Set the session config */
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  /*
  Set the NTP config time
  For times east of the Prime Meridian use 0-12
  For times west of the Prime Meridian add 12 to the offset.
  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = F("Automation BOX");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F("You have new order. Can you check it?");
  message.addRecipient(F("User"), RECIPIENT_EMAIL);
    
  /*Send HTML message*/
  /*String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Hello World!</h1><p>- Sent from ESP board</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;*/

   
  //Send raw text message
  // String textMsg = "Hello World! - Sent from ESP board";
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;


  /* Connect to the server */
  if (!smtp.connect(&config)){
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()){
    Serial.println("\nNot yet logged in.");
  }
  else{
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

}




void lcd_QR()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("THCS HOANG VAN");
  lcd.setCursor(0,1);
  lcd.print("Moi ban quet ma");
}

void lcd_QR_successful()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Quet thanh cong");
  player.play(3);
  delay(5000);
}

void open_door(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Cua dang mo");
  player.play(2);
  delay(10000);
  Serial.println("clockwise");
  myStepper.step(-0.8*stepsPerRevolution);
}

void playTone(long duration, int freq)
{
	duration *= 1000;
	int period = (1.0 / freq) * 1000000;
	long elapsed_time = 0;
	while (elapsed_time < duration)
	{
		digitalWrite(pinSpeaker,HIGH);
		delayMicroseconds(period / 2);
		digitalWrite(pinSpeaker, LOW);
		delayMicroseconds(period / 2);
		elapsed_time += (period);
	}
}

void close_door(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Cua dang dong");
  Serial.println("counterclockwise");
  myStepper.step(0.85*stepsPerRevolution);
  player.play(4);
  delay(5000);
   state=0;
}


void lcd_QR_failed()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Quet sai");
  player.play(1);
  delay(5000);
}

void send_SMS(){
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println("AT+CMGS=\"+84339938689\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  mySerial.print("Có đơn hàng"); //text content
  updateSerial();
  mySerial.write(26);
  updateSerial();
}

void calling(){
  delay(10000);
  updateSerial();
  mySerial.println("ATD+ +84339938689;"); //  change ZZ with country code and xxxxxxxxxxx with phone number to dial //  change ZZ with country code and xxxxxxxxxxx with phone number to dial
  updateSerial();
  delay(20000); // wait for 20 seconds...
  mySerial.println("ATH"); //hang up
  updateSerial();
}
void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

void bodem(int value_IR, int *default_value, int* counter, int statuss) {
  if (value_IR != *default_value) {
    if (value_IR == 0 && statuss == 0)
      {
      *counter += 1;
    }
    else if (value_IR == 0 && statuss == 1) 
      {
      if (*counter > 0) 
      {  
        *counter -= 1;
      }
    }
    *default_value = value_IR;
  }
}
void IR() {
while (true){
  // put your main code here, to run repeatedly:
  value_IR1 = digitalRead(IR1);
  value_IR2 = digitalRead(IR2);
  bodem(value_IR1, &default_value1, &counter, 0);//0 là tăng
  bodem(value_IR2, &default_value2, &counter,  1);// 1 là giảm
  if(counter > 0)
  {
    digitalWrite(relay,HIGH);
    Serial.println(counter);
  }
  else
  {
    delay(2000);
    digitalWrite(relay, LOW);
    close_door();
    Serial.println(counter);
    break;
  }
}
}

