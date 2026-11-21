/*
  ========================================================================
  DỰ ÁN: ĐIỀU KHIỂN 4 LED ESP32 QUA TELEGRAM BOT & WEB LOCAL
  Bot Name: esp32led_bot (@esp32led1234_bot)
  ========================================================================

  * Cấu hình phần cứng:
    - LED 1: GPIO 25
    - LED 2: GPIO 26
    - LED 3: GPIO 27
    - LED 4: GPIO 32

  * Thư viện cần cài trong Arduino IDE (Vào Sketch -> Include Library -> Manage Libraries):
    1. "UniversalTelegramBot" by Brian Lough (bản 1.3.0 trở lên)
    2. "ArduinoJson" by Benoit Blanchon (khuyên dùng bản 6.x)
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WebServer.h>

// ==============================================
// 1. CẤU HÌNH THÔNG TIN WI-FI & TELEGRAM TOKEN
// ==============================================

// Nhập tên và mật khẩu Wi-Fi nhà bạn (Wi-Fi có Internet 2.4GHz)
const char* ssid     = "TEN_WIFI_NHA_BAN";
const char* password = "MAT_KHAU_WIFI";

// Token Bot Telegram của bạn
#define BOT_TOKEN "9036377993:AAEvbfS6SDkgVknag0Hy2G5yDGT7WuSFQ4A"

// ==============================================
// 2. CẤU HÌNH CHÂN GPIO 4 ĐÈN LED
// ==============================================
const int LED1_PIN = 25;
const int LED2_PIN = 26;
const int LED3_PIN = 27;
const int LED4_PIN = 32;

// Trạng thái của 4 LED (true = SÁNG, false = TẮT)
bool led1_state = false;
bool led2_state = false;
bool led3_state = false;
bool led4_state = false;

// Chế độ nhấp nháy không làm đứng máy (Non-blocking Blink)
bool isBlinking = false;
unsigned long lastBlinkTime = 0;
const unsigned long BLINK_INTERVAL = 350; // ms
bool blinkToggle = false;

// ==============================================
// 3. KHỞI TẠO TELEGRAM BOT & WEB SERVER
// ==============================================
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

WebServer server(80);

unsigned long lastTimeBotRan = 0;
const unsigned long botRequestDelay = 1000; // Kiểm tra tin nhắn mới mỗi 1 giây

// ==============================================
// 4. HÀM GỬI MENU NÚT BẤM (INLINE KEYBOARD)
// ==============================================
void sendMainMenu(String chat_id) {
  String message = "👋 Chào bạn! Đây là bảng điều khiển 4 LED ESP32:\n\n";
  message += "📊 *Trạng thái hiện tại:*\n";
  message += "• LED 1 (GPIO 25): " + String(led1_state ? "🟢 BẬT" : "🔴 TẮT") + "\n";
  message += "• LED 2 (GPIO 26): " + String(led2_state ? "🟢 BẬT" : "🔴 TẮT") + "\n";
  message += "• LED 3 (GPIO 27): " + String(led3_state ? "🟢 BẬT" : "🔴 TẮT") + "\n";
  message += "• LED 4 (GPIO 32): " + String(led4_state ? "🟢 BẬT" : "🔴 TẮT") + "\n";
  if (isBlinking) message += "⚡ *Đang ở chế độ nhấp nháy*\n";
  message += "\n👇 *Chạm vào các nút bên dưới để điều khiển:*";

  // Tạo bàn phím Inline Keyboard
  String keyboardJson = "[";
  keyboardJson += "[{\"text\":\"💡 Bật LED 1\",\"callback_data\":\"/led1_on\"},{\"text\":\"🔴 Tắt LED 1\",\"callback_data\":\"/led1_off\"}],";
  keyboardJson += "[{\"text\":\"💡 Bật LED 2\",\"callback_data\":\"/led2_on\"},{\"text\":\"🔴 Tắt LED 2\",\"callback_data\":\"/led2_off\"}],";
  keyboardJson += "[{\"text\":\"💡 Bật LED 3\",\"callback_data\":\"/led3_on\"},{\"text\":\"🔴 Tắt LED 3\",\"callback_data\":\"/led3_off\"}],";
  keyboardJson += "[{\"text\":\"💡 Bật LED 4\",\"callback_data\":\"/led4_on\"},{\"text\":\"🔴 Tắt LED 4\",\"callback_data\":\"/led4_off\"}],";
  keyboardJson += "[{\"text\":\"⚡ Nhấp nháy cả 4 LED\",\"callback_data\":\"/blink\"},{\"text\":\"❌ Tắt tất cả\",\"callback_data\":\"/all_off\"}],";
  keyboardJson += "[{\"text\":\"🔄 Cập nhật trạng thái\",\"callback_data\":\"/status\"}]";
  keyboardJson += "]";

  bot.sendMessageWithInlineKeyboard(chat_id, message, "Markdown", keyboardJson);
}

// ==============================================
// 5. HÀM XỬ LÝ LỆNH TỪ TELEGRAM
// ==============================================
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text    = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.println("Lệnh từ [" + from_name + "]: " + text);

    if (text == "/start" || text == "/menu" || text == "/help") {
      sendMainMenu(chat_id);
    }
    else if (text == "/led1_on") {
      isBlinking = false;
      led1_state = true;
      digitalWrite(LED1_PIN, HIGH);
      bot.sendMessage(chat_id, "💡 LED 1 (GPIO 25) đã BẬT!", "");
    }
    else if (text == "/led1_off") {
      isBlinking = false;
      led1_state = false;
      digitalWrite(LED1_PIN, LOW);
      bot.sendMessage(chat_id, "🔴 LED 1 (GPIO 25) đã TẮT!", "");
    }
    else if (text == "/led2_on") {
      isBlinking = false;
      led2_state = true;
      digitalWrite(LED2_PIN, HIGH);
      bot.sendMessage(chat_id, "💡 LED 2 (GPIO 26) đã BẬT!", "");
    }
    else if (text == "/led2_off") {
      isBlinking = false;
      led2_state = false;
      digitalWrite(LED2_PIN, LOW);
      bot.sendMessage(chat_id, "🔴 LED 2 (GPIO 26) đã TẮT!", "");
    }
    else if (text == "/led3_on") {
      isBlinking = false;
      led3_state = true;
      digitalWrite(LED3_PIN, HIGH);
      bot.sendMessage(chat_id, "💡 LED 3 (GPIO 27) đã BẬT!", "");
    }
    else if (text == "/led3_off") {
      isBlinking = false;
      led3_state = false;
      digitalWrite(LED3_PIN, LOW);
      bot.sendMessage(chat_id, "🔴 LED 3 (GPIO 27) đã TẮT!", "");
    }
    else if (text == "/led4_on") {
      isBlinking = false;
      led4_state = true;
      digitalWrite(LED4_PIN, HIGH);
      bot.sendMessage(chat_id, "💡 LED 4 (GPIO 32) đã BẬT!", "");
    }
    else if (text == "/led4_off") {
      isBlinking = false;
      led4_state = false;
      digitalWrite(LED4_PIN, LOW);
      bot.sendMessage(chat_id, "🔴 LED 4 (GPIO 32) đã TẮT!", "");
    }
    else if (text == "/all_on") {
      isBlinking = false;
      led1_state = true; digitalWrite(LED1_PIN, HIGH);
      led2_state = true; digitalWrite(LED2_PIN, HIGH);
      led3_state = true; digitalWrite(LED3_PIN, HIGH);
      led4_state = true; digitalWrite(LED4_PIN, HIGH);
      bot.sendMessage(chat_id, "💡 Đã BẬT tất cả 4 đèn LED!", "");
    }
    else if (text == "/all_off") {
      isBlinking = false;
      led1_state = false; digitalWrite(LED1_PIN, LOW);
      led2_state = false; digitalWrite(LED2_PIN, LOW);
      led3_state = false; digitalWrite(LED3_PIN, LOW);
      led4_state = false; digitalWrite(LED4_PIN, LOW);
      bot.sendMessage(chat_id, "❌ Đã TẮT tất cả 4 đèn LED!", "");
    }
    else if (text == "/blink") {
      isBlinking = true;
      bot.sendMessage(chat_id, "⚡ Đang nhấp nháy cả 4 đèn LED! (Gửi lệnh tắt bất kỳ để dừng)", "");
    }
    else if (text == "/status") {
      sendMainMenu(chat_id);
    }
    else {
      bot.sendMessage(chat_id, "❓ Lệnh không hợp lệ. Hãy bấm /start để mở menu.", "");
    }
  }
}

// ==============================================
// 6. XỬ LÝ NHẤP NHÁY KHÔNG BLOCKING
// ==============================================
void handleBlink() {
  if (!isBlinking) return;

  unsigned long currentMillis = millis();
  if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
    lastBlinkTime = currentMillis;
    blinkToggle = !blinkToggle;

    digitalWrite(LED1_PIN, blinkToggle ? HIGH : LOW);
    digitalWrite(LED2_PIN, blinkToggle ? HIGH : LOW);
    digitalWrite(LED3_PIN, blinkToggle ? HIGH : LOW);
    digitalWrite(LED4_PIN, blinkToggle ? HIGH : LOW);

    led1_state = blinkToggle;
    led2_state = blinkToggle;
    led3_state = blinkToggle;
    led4_state = blinkToggle;
  }
}

// ==============================================
// 7. SETUP WEB SERVER ĐỒNG BỘ VỚI WEB / CAMERA
// ==============================================
void setupWebServer() {
  server.enableCORS(true);

  server.on("/led1/on", []() {
    isBlinking = false; led1_state = true; digitalWrite(LED1_PIN, HIGH);
    server.send(200, "text/plain", "LED 1 ON");
  });
  server.on("/led1/off", []() {
    isBlinking = false; led1_state = false; digitalWrite(LED1_PIN, LOW);
    server.send(200, "text/plain", "LED 1 OFF");
  });
  server.on("/led2/on", []() {
    isBlinking = false; led2_state = true; digitalWrite(LED2_PIN, HIGH);
    server.send(200, "text/plain", "LED 2 ON");
  });
  server.on("/led2/off", []() {
    isBlinking = false; led2_state = false; digitalWrite(LED2_PIN, LOW);
    server.send(200, "text/plain", "LED 2 OFF");
  });
  server.on("/led3/on", []() {
    isBlinking = false; led3_state = true; digitalWrite(LED3_PIN, HIGH);
    server.send(200, "text/plain", "LED 3 ON");
  });
  server.on("/led3/off", []() {
    isBlinking = false; led3_state = false; digitalWrite(LED3_PIN, LOW);
    server.send(200, "text/plain", "LED 3 OFF");
  });
  server.on("/led4/on", []() {
    isBlinking = false; led4_state = true; digitalWrite(LED4_PIN, HIGH);
    server.send(200, "text/plain", "LED 4 ON");
  });
  server.on("/led4/off", []() {
    isBlinking = false; led4_state = false; digitalWrite(LED4_PIN, LOW);
    server.send(200, "text/plain", "LED 4 OFF");
  });
  server.on("/all/off", []() {
    isBlinking = false;
    led1_state = led2_state = led3_state = led4_state = false;
    digitalWrite(LED1_PIN, LOW); digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, LOW); digitalWrite(LED4_PIN, LOW);
    server.send(200, "text/plain", "ALL OFF");
  });
  server.on("/blink", []() {
    isBlinking = true;
    server.send(200, "text/plain", "BLINKING");
  });

  server.begin();
}

// ==============================================
// 8. SETUP & LOOP CHÍNH
// ==============================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- KHỞI ĐỘNG ESP32 TELEGRAM & WEB ---");

  // Thiết lập chân GPIO LED
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);

  // Mặc định ban đầu tắt tất cả LED
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(LED4_PIN, LOW);

  // Kết nối Wi-Fi có Internet
  Serial.print("Đang kết nối Wi-Fi: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int wifiTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTimeout < 30) {
    delay(500);
    Serial.print(".");
    wifiTimeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi đã kết nối!");
    Serial.print("Địa chỉ IP ESP32: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nChưa kết nối được Wi-Fi! Vui lòng kiểm tra lại tên Wi-Fi và mật khẩu.");
  }

  // Bỏ qua chứng chỉ SSL Telegram (để kết nối HTTPS mượt mà không bị lỗi cert)
  secured_client.setInsecure();

  // Khởi động Web Server (hỗ trợ điều khiển song song qua Web/Camera)
  setupWebServer();
  Serial.println("Bot Telegram @esp32led1234_bot đã sẵn sàng nhận lệnh!");
}

void loop() {
  // 1. Lắng nghe yêu cầu từ Web Server
  server.handleClient();

  // 2. Xử lý nhấp nháy đèn nếu đang bật chế độ blink
  handleBlink();

  // 3. Quét tin nhắn mới từ Telegram Bot mỗi 1 giây
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}
