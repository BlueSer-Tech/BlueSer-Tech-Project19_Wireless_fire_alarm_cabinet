#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6Xr9ceiyQ"
#define BLYNK_TEMPLATE_NAME "TU DIEN THONG MINH"
#define BLYNK_AUTH_TOKEN "nCJP2r0l6Dv96AT_pVAJvC38TzjPrWyS"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
//==========Gmail===========//
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <time.h>
#include <ESP_Mail_Client.h>
#define SMTP_server "smtp.gmail.com"
#define SMTP_Port 465
#define sender_email "hieu333.hunn@gmail.com"
#define sender_password "hmtvzwmlzriensdu"
#define Recipient_email "3h1v6nguoi1gd@gmail.com"
#define Recipient_name ""
SMTPSession smtp;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 25200;
const int daylightOffset_sec = 0;
int flag=0;
int flag1=0;
//======Wifi========//
char* ssid = "Mastermind";
char* password = "2001zone";

#define DHTPIN D1             // Chân kết nối DHT11
#define DHTTYPE DHT11        // Loại cảm biến DHT
#define FAN_PIN D4         // Chân kết nối LED
#define BUZZER_PIN D0      // Chân kết nối quạt
#define BUTTON_PIN  D6    // Button
#define MQ2 A0           // Chân kết nối cảm biến khí gas MQ

DHT dht(DHTPIN, DHTTYPE);
#define DHT_TEMP    V0 
#define DHT_HUM    V1
#define GAS_SENSOR V2
BlynkTimer timer;
WidgetLED led1(V3);
WidgetLED led2(V4);


int MQ2_Val = 0;

int wifiFlag = 0;
int button_state;       // the current state of button
int last_button_state;  // the previous state of button

void sendSensorData()
{
  //=====Cảm biến nhiệt độ và độ ẩm DHT11=====//
  float temperature = dht.readTemperature(); // Đọc nhiệt độ từ DHT11
  float humidity = dht.readHumidity(); // Đọc độ ẩm từ DHT11
  Serial.println(temperature);
  Serial.println(humidity);
  Blynk.virtualWrite(DHT_TEMP, temperature); // Hiển thị nhiệt độ trên Widget V1
  Blynk.virtualWrite(DHT_HUM, humidity); // Hiển thị độ ẩm trên Widget V2
  //=====Cảm biến khói MQ2=====//
  MQ2_Val = analogRead(MQ2);
  Blynk.virtualWrite(GAS_SENSOR, MQ2_Val);
  Serial.print("MQ2 Level: ");
  Serial.println(MQ2_Val);
}

void checkBlynkStatus() { // 
  bool state = Blynk.connected();             
  //digitalWrite(2, HIGH);  
  sendSensorData();
}
void send_notification()
{
  smtp.debug(1);
  ESP_Mail_Session session;
  session.server.host_name = SMTP_server ;
  session.server.port = SMTP_Port;
  session.login.email = sender_email;
  session.login.password = sender_password;
  session.login.user_domain = "";
  /* Declare the message class */
  SMTP_Message message;
  message.sender.name = "ESP 32";
  message.sender.email = sender_email;
  message.subject = "Cảnh Báo Phát Hiện Khí Gas!";
  message.addRecipient(Recipient_name,Recipient_email);
  //Send HTML message
  String htmlMsg = "<div style=\"color:#000000;\">";
  htmlMsg +="<p>Phát Hiện Nồng Độ Khí Gas Quá Cao!" ;
  htmlMsg += "</p>";
  htmlMsg += "</div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  /* //Send simple text message
  String textMsg = "How are you doing";
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;*/
  if (!smtp.connect(&session))
    return;
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}
void setup()
{
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // set ESP32 pin to input pull-up mode
  button_state = digitalRead(BUTTON_PIN);
  dht.begin();
  
}
void button_sensordata(){
  last_button_state = button_state;      // save the last state
  button_state = digitalRead(BUTTON_PIN); // read new state
  if (last_button_state == HIGH && button_state == LOW) {
    Serial.println("The button is pressed");
    digitalWrite(FAN_PIN, HIGH);
    led1.on();
    led2.on();
    for(int i=0;i<10;i++){
      soundAlarm();
    }
    analogWrite(BUZZER_PIN,0); 
    digitalWrite(FAN_PIN, LOW);
    led1.off();
    led2.off();
    send_notification();
  }
  else{
    analogWrite(BUZZER_PIN,0); 
    digitalWrite(FAN_PIN, LOW);
    led1.off();
    led2.off();
  }
  //===================MQ2=============//
  MQ2_Val = analogRead(MQ2);
  if(MQ2_Val > 800 && flag==0) {
    MQ2_Val = analogRead(MQ2);
    send_notification();
    flag=1;
  }
  if(MQ2_Val > 800 && flag==1) {
    MQ2_Val = analogRead(MQ2);
    digitalWrite(FAN_PIN, HIGH);
    led1.on();
    led2.on();
    for(int i=0;i<10;i++){
      soundAlarm();
    }
  }
  if(MQ2_Val < 800){
    MQ2_Val = analogRead(MQ2);
    flag=0;
    analogWrite(BUZZER_PIN,0); 
    digitalWrite(FAN_PIN, LOW);
    led1.off();
    led2.off();
  }
  //=================DHT============//
  float temperature = dht.readTemperature();
  if(temperature > 35 && flag1==0) {
    float temperature = dht.readTemperature();
    send_notification();
    flag1=1;
  }
  if(temperature > 35 && flag1==1) {
    float temperature = dht.readTemperature();
    digitalWrite(FAN_PIN, HIGH);
    led1.on();
    led2.on();
    for(int i=0;i<10;i++){
      soundAlarm();
    }
  }
  if(temperature < 35){
    float temperature = dht.readTemperature();
    flag1=0;
    analogWrite(BUZZER_PIN,0); 
    digitalWrite(FAN_PIN, LOW);
    led1.off();
    led2.off();
  }
  delay(100);
}

void loop()
{
  if (Blynk.connected()==false){
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password); // Thay YourSSID và YourPassword bằng thông tin WiFi của bạn
    timer.setInterval(100L, checkBlynkStatus); // Gửi dữ liệu từ cảm biến mỗi 100 milis giây
    Blynk.config(BLYNK_AUTH_TOKEN);
    WiFi.begin(ssid, password);
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    }
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(200);    
    Serial.println();
    Serial.println("WiFi connected.");
    Serial.println();
    Serial.println("Preparing to send email");
    Serial.println();
  }
  else if (Blynk.connected()==true){
    Blynk.run();
    timer.run();
  }
  button_sensordata();
}


void soundAlarm() {
  for (int i = 0; i < 255; i++) {
    analogWrite(BUZZER_PIN, 3 * i);  
    delay(2);
  }
  for (int i = 255; i > -1; i--) {
    analogWrite(BUZZER_PIN, 3 * i);  
    delay(2);
  }
  delay(1000);
}

