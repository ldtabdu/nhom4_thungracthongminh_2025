#include <ESP8266WiFi.h> // Thư viện để kết nối WiFi cho ESP8266
#include <PubSubClient.h> // Thư viện MQTT (gửi/nhận dữ liệu đến broker MQTT)
#include <Servo.h> // Thư viện điều khiển servo

Servo servo1; // Tạo đối tượng servo tên là servo1

const int trigPin = D5; // Chân trig của cảm biến siêu âm
const int echoPin = D6; // Chân echo của cảm biến siêu âm
const int potPin = A0; // Chân đọc analog (độ ẩm)
const int ledPin = D1; // Chân LED báo trạng thái
const int buzzerPin = D2; // Chân điều khiển còi

const int maxDryValue = 1; // Giá trị ngưỡng xác định rác khô hay ướt
const int Ultra_Distance = 15; // Khoảng cách kích hoạt đo rác

long duration; // Biến lưu thời gian phản hồi của cảm biến siêu âm
int distance = 0; // Khoảng cách đo được
int soil = 0; // Giá trị độ ẩm hiện tại
int fsoil = 0; // Giá trị trung bình độ ẩm sau khi lọc
String trashStatus = "N/A"; // Trạng thái loại rác
String servoPosition = "Giữa"; // Vị trí servo hiện tại

int servoLeftCount = 0; // Đếm số lần servo nghiêng trái
int servoRightCount = 0; // Đếm số lần servo nghiêng phải

bool ledStatus = false; // Trạng thái LED
bool buzzerStatus = false; // Trạng thái còi

const char* ssid = "Fablab 2.4G"; // Tên WiFi
const char* password = "Fira@2024"; // Mật khẩu WiFi
const char* mqtt_server = "192.168.69.68"; // Địa chỉ IP v4

WiFiClient espClient; // Tạo client WiFi
PubSubClient client(espClient); // Tạo client MQTT dựa trên WiFi client

// Hàm xử lý dữ liệu từ MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Lệnh nhận từ MQTT: ");
  Serial.println(message);

  if (String(topic) == "trash/control/servo") {
    if (message == "LEFT") {
      servo1.write(165);
      servoPosition = "Trái";
      servoLeftCount++;
    } else if (message == "RIGHT") {
      servo1.write(15);
      servoPosition = "Phải";
      servoRightCount++;
    } else if (message == "CENTER") {
      servo1.write(90);
      servoPosition = "Giữa";
    }

    // Gửi thông tin lên OpenHAB
    client.publish("trash/servo", servoPosition.c_str());
    client.publish("trash/count/left", String(servoLeftCount).c_str());
    client.publish("trash/count/right", String(servoRightCount).c_str());
  }

  // Điều khiển LED từ MQTT
  if (String(topic) == "trash/control/led") {
    if (message == "ON") {
      digitalWrite(ledPin, HIGH);
      ledStatus = true;
    } else if (message == "OFF") {
      digitalWrite(ledPin, LOW);
      ledStatus = false;
    }
    client.publish("trash/status/led", ledStatus ? "ON" : "OFF");
  }

  // Điều khiển còi từ MQTT
  if (String(topic) == "trash/control/buzzer") {
    if (message == "ON") {
      digitalWrite(buzzerPin, HIGH);
      buzzerStatus = true;
    } else if (message == "OFF") {
      digitalWrite(buzzerPin, LOW);
      buzzerStatus = false;
    }
  }
  client.publish("trash/status/buzzer", buzzerStatus ? "ON" : "OFF");
}

// Hàm kết nối lại nếu mất kết nối MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Đang kết nối MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Thành công!");
      client.subscribe("trash/control/servo");
      client.subscribe("trash/control/led");
      client.subscribe("trash/control/buzzer");
    } else {
      Serial.print("Thất bại, mã lỗi=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  servo1.attach(D7);

  WiFi.begin(ssid, password);
  Serial.print("Kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println(" Đã kết nối!");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  distance = 0;
  fsoil = 0;

  for (int i = 0; i < 2; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(7);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH, 25000);
    distance += duration * 0.034 / 2;
    delay(10);
  }
  distance = distance / 2;

  if (distance < Ultra_Distance && distance > 1) {
    delay(1000);
    for (int i = 0; i < 3; i++) {
      soil = analogRead(potPin);
      soil = constrain(soil, 485, 1023);
      fsoil += map(soil, 485, 1023, 100, 0);
      delay(75);
    }
    fsoil = fsoil / 3;

    Serial.print("Humidity: ");
    Serial.print(fsoil);
    Serial.print("%    Distance: ");
    Serial.print(distance);
    Serial.print(" cm");

    if (fsoil > maxDryValue) {
      Serial.println("  ==> Rác ướt - Nghiêng phải");
      servo1.write(15);
      trashStatus = "Ướt";
      servoPosition = "Phải";
      servoRightCount++;

      // Còi kêu 3 tiếng bíp và đèn chớp 3 lần
      for (int i = 0; i < 3; i++) {
        digitalWrite(buzzerPin, HIGH);
        delay(200);
        digitalWrite(buzzerPin, LOW);
        delay(200);
      }
      for (int i = 0; i < 3; i++) {
        digitalWrite(ledPin, HIGH);
        delay(200);
        digitalWrite(ledPin, LOW);
        delay(200);
      }

    } else {
      Serial.println("  ==> Rác khô - Nghiêng trái");
      servo1.write(165);
      trashStatus = "Khô";
      servoPosition = "Trái";
      servoLeftCount++;

      // Còi kêu 3 tiếng bíp và đèn chớp 3 lần
      for (int i = 0; i < 3; i++) {
        digitalWrite(buzzerPin, HIGH);
        delay(200);
        digitalWrite(buzzerPin, LOW);
        delay(200);
      }
      for (int i = 0; i < 3; i++) {
        digitalWrite(ledPin, HIGH);
        delay(200);
        digitalWrite(ledPin, LOW);
        delay(200);
      }
    }

    client.publish("trash/humidity", String(fsoil).c_str());
    client.publish("trash/distance", String(distance).c_str());
    client.publish("trash/status", trashStatus.c_str());
    client.publish("trash/servo", servoPosition.c_str());
    client.publish("trash/count/left", String(servoLeftCount).c_str());
    client.publish("trash/count/right", String(servoRightCount).c_str());

    delay(3000);
    servo1.write(90);
    servoPosition = "Giữa";
    client.publish("trash/servo", servoPosition.c_str());
  }

  delay(1000);
}
