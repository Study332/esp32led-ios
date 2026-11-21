#include <WiFi.h>
#include <WebServer.h>
#include <TM1637Display.h>

// =========================
// CẤU HÌNH CHÂN GPIO
// =========================
const int LED1 = 25;
const int LED2 = 26;
const int LED3 = 27;
const int LED4 = 32;

// Chân màn hình LED 7 đoạn TM1637
#define CLK 14
#define DIO 33

TM1637Display display(CLK, DIO);

// Lưu trạng thái 4 đèn (false = Tắt, true = Bật)
bool ledState[4] = {false, false, false, false};

// =========================
// CHẾ ĐỘ NHẤP NHÁY THEO THỨ TỰ (500MS)
// =========================
bool isBlinking = false;
unsigned long lastBlinkTime = 0;
int currentBlinkLed = 0; // 0: LED1, 1: LED2, 2: LED3, 3: LED4

// =========================
// WIFI ESP32
// =========================
const char* ssid = "ESP32_LED";
const char* password = "12345678";

WebServer server(80);

// =========================
// CẬP NHẬT MÀN HÌNH 7 ĐOẠN
// =========================
void updateDisplay() {
  uint8_t data[4];
  for (int i = 0; i < 4; i++) {
    if (ledState[i]) {
      // Đèn bật -> Hiện số thứ tự đèn (1, 2, 3, 4)
      data[i] = display.encodeDigit(i + 1);
    } else {
      // Đèn tắt -> Hiện dấu gạch ngang "-"
      data[i] = SEG_G;
    }
  }
  display.setSegments(data);
}

// =========================
// CORS
// =========================
void allowCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");
}

// =========================
// LED 1
// =========================
void led1On() {
  isBlinking = false;
  digitalWrite(LED1, HIGH);
  ledState[0] = true;
  updateDisplay();
  allowCORS();
  server.send(200, "text/plain", "LED 1 ON");
}

void led1Off() {
  isBlinking = false;
  digitalWrite(LED1, LOW);
  ledState[0] = false;
  updateDisplay();
  allowCORS();
  server.send(200, "text/plain", "LED 1 OFF");
}

// =========================
// LED 2
// =========================
void led2On() {
  isBlinking = false;
  digitalWrite(LED2, HIGH);
  ledState[1] = true;
  updateDisplay();
  allowCORS();
  server.send(200, "text/plain", "LED 2 ON");
}

void led2Off() {
  isBlinking = false;
  digitalWrite(LED2, LOW);
  ledState[1] = false;
  updateDisplay();
  allowCORS();
  server.send(200, "text/plain", "LED 2 OFF");
}

// =========================
// LED 3
// =========================
void led3On() {
  isBlinking = false;
  digitalWrite(LED3, HIGH);
  ledState[2] = true;
  updateDisplay();
  allowCORS();
  server.send(200, "text/plain", "LED 3 ON");
}

void led3Off() {
  isBlinking = false;
  digitalWrite(LED3, LOW);
  ledState[2] = false;
  updateDisplay();
  allowCORS();
  server.send(200, "text/plain", "LED 3 OFF");
}

// =========================
// LED 4
// =========================
void led4On() {
  isBlinking = false;
  digitalWrite(LED4, HIGH);
  ledState[3] = true;
  updateDisplay();
  allowCORS();
  server.send(200, "text/plain", "LED 4 ON");
}

void led4Off() {
  isBlinking = false;
  digitalWrite(LED4, LOW);
  ledState[3] = false;
  updateDisplay();
  allowCORS();
  server.send(200, "text/plain", "LED 4 OFF");
}

// =========================
// TẮT TẤT CẢ
// =========================
void allOff() {
  isBlinking = false;
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);

  for (int i = 0; i < 4; i++) {
    ledState[i] = false;
  }
  updateDisplay();

  allowCORS();
  server.send(200, "text/plain", "ALL LED OFF");
}

// =========================
// BẬT CHẾ ĐỘ NHẤP NHÁY THEO THỨ TỰ (500MS)
// =========================
void handleBlinkRoute() {
  isBlinking = true;
  currentBlinkLed = 0;
  lastBlinkTime = 0;
  allowCORS();
  server.send(200, "text/plain", "BLINK SEQUENTIAL 500MS");
}

// Xử lý đổi đèn mỗi 500ms không làm đứng máy
void updateBlinkLoop() {
  if (!isBlinking) return;

  unsigned long currentMillis = millis();
  if (currentMillis - lastBlinkTime >= 500) {
    lastBlinkTime = currentMillis;

    // Tắt hết cả 4 LED
    digitalWrite(LED1, LOW); ledState[0] = false;
    digitalWrite(LED2, LOW); ledState[1] = false;
    digitalWrite(LED3, LOW); ledState[2] = false;
    digitalWrite(LED4, LOW); ledState[3] = false;

    // Bật đèn tiếp theo theo thứ tự: 1 -> 2 -> 3 -> 4
    if (currentBlinkLed == 0) {
      digitalWrite(LED1, HIGH); ledState[0] = true;
    } else if (currentBlinkLed == 1) {
      digitalWrite(LED2, HIGH); ledState[1] = true;
    } else if (currentBlinkLed == 2) {
      digitalWrite(LED3, HIGH); ledState[2] = true;
    } else if (currentBlinkLed == 3) {
      digitalWrite(LED4, HIGH); ledState[3] = true;
    }

    updateDisplay();
    currentBlinkLed = (currentBlinkLed + 1) % 4;
  }
}

// =========================
// SETUP
// =========================
void setup() {
  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);

  // Khởi tạo màn hình 7 đoạn
  display.setBrightness(5);
  updateDisplay(); // Hiện "- - - -" lúc mới cấp nguồn

  // ESP32 phát WiFi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("==========================");
  Serial.println("ESP32 LED & TM1637 CONTROL");
  Serial.println("==========================");
  Serial.print("WiFi: ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  // =========================
  // ROUTE LED
  // =========================
  server.on("/led1/on", led1On);
  server.on("/led1/off", led1Off);

  server.on("/led2/on", led2On);
  server.on("/led2/off", led2Off);

  server.on("/led3/on", led3On);
  server.on("/led3/off", led3Off);

  server.on("/led4/on", led4On);
  server.on("/led4/off", led4Off);

  server.on("/all/off", allOff);
  server.on("/blink", handleBlinkRoute);

  server.begin();
  Serial.println("Web server started!");
}

// =========================
// LOOP
// =========================
void loop() {
  server.handleClient();
  updateBlinkLoop();
}
