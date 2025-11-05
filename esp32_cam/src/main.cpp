/**
 * ESP32-CAM Cámara Trampa para Hipopótamos
 * 
 * Este código permite que el ESP32-CAM:
 * 1. Se conecte a WiFi
 * 2. Capture fotos bajo demanda desde el servidor Flask
 * 3. Envíe las fotos capturadas al servidor
 * 4. Transmita video en streaming en tiempo real
 * 
 * Desarrollado con PlatformIO para ESP32-CAM (AI-Thinker)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include "config.h"
#include "camera_pins.h"

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

bool cameraInitialized = false;
bool wifiConnected = false;
bool streamingActive = false;

unsigned long lastCaptureCheck = 0;
unsigned long lastStreamingCheck = 0;
unsigned long lastStreamFrame = 0;

// ============================================================================
// DECLARACIÓN DE FUNCIONES
// ============================================================================

bool initCamera();
bool connectWiFi();
void checkCaptureRequest();
void checkStreamingStatus();
void captureAndSendPhoto();
void sendStreamFrame();
bool sendImageToServer(camera_fb_t *fb, const char* endpoint);
void printStatus();
void blinkLED(int times, int delayMs);

// ============================================================================
// SETUP - INICIALIZACIÓN
// ============================================================================

void setup() {
  // Inicializar Serial para debug
  Serial.begin(115200);
  delay(1000);
  
  DEBUG_PRINTLN("\n\n" + String('=', 60));
  DEBUG_PRINTLN("ESP32-CAM Cámara Trampa para Hipopótamos");
  DEBUG_PRINTLN(String('=', 60));
  
  // Configurar LED flash
  pinMode(LED_FLASH_PIN, OUTPUT);
  digitalWrite(LED_FLASH_PIN, LOW);
  
  // Indicar inicio con LED
  blinkLED(3, 200);
  
  // Inicializar cámara
  DEBUG_PRINTLN("\n[1/2] Inicializando cámara...");
  if (initCamera()) {
    DEBUG_PRINTLN("✓ Cámara inicializada correctamente");
    cameraInitialized = true;
  } else {
    DEBUG_PRINTLN("✗ Error al inicializar cámara");
    DEBUG_PRINTLN("REINICIANDO EN 5 SEGUNDOS...");
    delay(5000);
    ESP.restart();
  }
  
  // Conectar a WiFi
  DEBUG_PRINTLN("\n[2/2] Conectando a WiFi...");
  if (connectWiFi()) {
    DEBUG_PRINTLN("✓ Conectado a WiFi");
    wifiConnected = true;
    blinkLED(5, 100);
  } else {
    DEBUG_PRINTLN("✗ Error al conectar a WiFi");
    DEBUG_PRINTLN("REINICIANDO EN 5 SEGUNDOS...");
    delay(5000);
    ESP.restart();
  }
  
  // Mostrar información
  DEBUG_PRINTLN("\n" + String('=', 60));
  printStatus();
  DEBUG_PRINTLN(String('=', 60));
  DEBUG_PRINTLN("\nESP32-CAM listo y operando...\n");
}

// ============================================================================
// LOOP - BUCLE PRINCIPAL
// ============================================================================

void loop() {
  // Verificar conexión WiFi
  if (WiFi.status() != WL_CONNECTED) {
    DEBUG_PRINTLN("WiFi desconectado. Reconectando...");
    wifiConnected = false;
    connectWiFi();
    return;
  }
  
  // Verificar si debe capturar foto
  if (millis() - lastCaptureCheck >= CAPTURE_CHECK_INTERVAL) {
    lastCaptureCheck = millis();
    checkCaptureRequest();
  }
  
  // Verificar estado de streaming
  if (millis() - lastStreamingCheck >= STREAMING_CHECK_INTERVAL) {
    lastStreamingCheck = millis();
    checkStreamingStatus();
  }
  
  // Si streaming está activo, enviar frames
  if (streamingActive) {
    if (millis() - lastStreamFrame >= STREAMING_FRAME_DELAY) {
      lastStreamFrame = millis();
      sendStreamFrame();
    }
  }
  
  // Pequeño delay para no saturar el CPU
  delay(10);
}

// ============================================================================
// INICIALIZACIÓN DE CÁMARA
// ============================================================================

bool initCamera() {
  camera_config_t config;
  
  // Configuración de pines
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Configuración de calidad
  if (psramFound()) {
    DEBUG_PRINTLN("  PSRAM encontrada");
    config.frame_size = FRAME_SIZE_CAPTURE;
    config.jpeg_quality = JPEG_QUALITY_CAPTURE;
    config.fb_count = 2;
  } else {
    DEBUG_PRINTLN("  PSRAM no encontrada - usando configuración reducida");
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Inicializar cámara
  esp_err_t err = esp_camera_init(&config);
  
  if (err != ESP_OK) {
    DEBUG_PRINTF("  Error al inicializar cámara: 0x%x\n", err);
    return false;
  }
  
  // Configuraciones adicionales del sensor
  sensor_t *s = esp_camera_sensor_get();
  if (s != NULL) {
    // Ajustes opcionales
    s->set_brightness(s, 0);     // -2 a 2
    s->set_contrast(s, 0);       // -2 a 2
    s->set_saturation(s, 0);     // -2 a 2
    s->set_special_effect(s, 0); // 0 = Sin efecto
    s->set_whitebal(s, 1);       // Balance de blancos automático
    s->set_awb_gain(s, 1);       // Ganancia AWB automática
    s->set_wb_mode(s, 0);        // Modo WB automático
    s->set_exposure_ctrl(s, 1);  // Exposición automática
    s->set_aec2(s, 0);           // AEC DSP
    s->set_gain_ctrl(s, 1);      // Control de ganancia automático
    s->set_agc_gain(s, 0);       // Ganancia AGC
    s->set_gainceiling(s, (gainceiling_t)0); // Techo de ganancia
    s->set_bpc(s, 0);            // BPC
    s->set_wpc(s, 1);            // WPC
    s->set_raw_gma(s, 1);        // Raw GMA
    s->set_lenc(s, 1);           // Corrección de lente
    s->set_hmirror(s, 0);        // Espejo horizontal
    s->set_vflip(s, 0);          // Volteo vertical
    s->set_dcw(s, 1);            // DCW
    s->set_colorbar(s, 0);       // Barra de color de prueba
  }
  
  return true;
}

// ============================================================================
// CONEXIÓN WIFI
// ============================================================================

bool connectWiFi() {
  DEBUG_PRINTLN("  SSID: " + String(WIFI_SSID));
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long startTime = millis();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(".");
    
    if (millis() - startTime > WIFI_TIMEOUT) {
      DEBUG_PRINTLN("\n  Timeout al conectar a WiFi");
      return false;
    }
  }
  
  DEBUG_PRINTLN();
  return true;
}

// ============================================================================
// VERIFICAR SOLICITUD DE CAPTURA
// ============================================================================

void checkCaptureRequest() {
  if (!wifiConnected || !cameraInitialized) return;
  
  HTTPClient http;
  
  // Construir URL completa
  String url = String("http://") + SERVER_IP + ":" + String(SERVER_PORT) + "/api/capture";
  
  http.begin(url);
  http.setTimeout(HTTP_TIMEOUT);
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    // Parsear JSON
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      bool shouldCapture = doc["capture"] | false;
      
      if (shouldCapture) {
        DEBUG_PRINTLN("\n>>> SOLICITUD DE CAPTURA RECIBIDA <<<");
        captureAndSendPhoto();
      }
    }
  } else if (httpCode > 0) {
    DEBUG_PRINTF("Error en checkCaptureRequest: HTTP %d\n", httpCode);
  }
  
  http.end();
}

// ============================================================================
// VERIFICAR ESTADO DE STREAMING
// ============================================================================

void checkStreamingStatus() {
  if (!wifiConnected || !cameraInitialized) return;
  
  HTTPClient http;
  
  String url = String("http://") + SERVER_IP + ":" + String(SERVER_PORT) + "/api/streaming-status";
  
  http.begin(url);
  http.setTimeout(HTTP_TIMEOUT);
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      bool shouldStream = doc["streaming"] | false;
      
      if (shouldStream != streamingActive) {
        streamingActive = shouldStream;
        
        if (streamingActive) {
          DEBUG_PRINTLN("\n>>> STREAMING ACTIVADO <<<");
          
          // Cambiar configuración de cámara para streaming
          sensor_t *s = esp_camera_sensor_get();
          if (s != NULL) {
            s->set_framesize(s, FRAME_SIZE_STREAM);
            s->set_quality(s, JPEG_QUALITY_STREAM);
          }
        } else {
          DEBUG_PRINTLN("\n>>> STREAMING DESACTIVADO <<<");
          
          // Restaurar configuración para captura
          sensor_t *s = esp_camera_sensor_get();
          if (s != NULL) {
            s->set_framesize(s, FRAME_SIZE_CAPTURE);
            s->set_quality(s, JPEG_QUALITY_CAPTURE);
          }
        }
      }
    }
  }
  
  http.end();
}

// ============================================================================
// CAPTURAR Y ENVIAR FOTO
// ============================================================================

void captureAndSendPhoto() {
  DEBUG_PRINTLN("  Capturando foto...");
  
  // Encender flash si está habilitado
  if (USE_FLASH) {
    digitalWrite(LED_FLASH_PIN, HIGH);
    delay(100);
  }
  
  // Capturar imagen
  camera_fb_t *fb = esp_camera_fb_get();
  
  // Apagar flash
  if (USE_FLASH) {
    digitalWrite(LED_FLASH_PIN, LOW);
  }
  
  if (!fb) {
    DEBUG_PRINTLN("  ✗ Error al capturar imagen");
    return;
  }
  
  DEBUG_PRINTF("  ✓ Foto capturada: %d bytes\n", fb->len);
  DEBUG_PRINTLN("  Enviando al servidor...");
  
  // Enviar al servidor
  String url = String("http://") + SERVER_IP + ":" + String(SERVER_PORT) + "/api/upload";
  bool success = sendImageToServer(fb, url.c_str());
  
  if (success) {
    DEBUG_PRINTLN("  ✓ Foto enviada exitosamente");
    blinkLED(2, 100);
  } else {
    DEBUG_PRINTLN("  ✗ Error al enviar foto");
  }
  
  // Liberar buffer
  esp_camera_fb_return(fb);
}

// ============================================================================
// ENVIAR FRAME DE STREAMING
// ============================================================================

void sendStreamFrame() {
  if (!wifiConnected || !cameraInitialized) return;
  
  // Capturar frame
  camera_fb_t *fb = esp_camera_fb_get();
  
  if (!fb) {
    DEBUG_PRINTLN("Error al capturar frame de streaming");
    return;
  }
  
  // Enviar al servidor
  String url = String("http://") + SERVER_IP + ":" + String(SERVER_PORT) + "/api/stream-frame";
  sendImageToServer(fb, url.c_str());
  
  // Liberar buffer
  esp_camera_fb_return(fb);
}

// ============================================================================
// ENVIAR IMAGEN AL SERVIDOR
// ============================================================================

bool sendImageToServer(camera_fb_t *fb, const char* endpoint) {
  if (!fb) return false;
  
  HTTPClient http;
  
  http.begin(endpoint);
  http.setTimeout(HTTP_TIMEOUT);
  
  // Crear boundary para multipart/form-data
  String boundary = "ESP32CAM-" + String(random(1000, 9999));
  String contentType = "multipart/form-data; boundary=" + boundary;
  
  // Construir el cuerpo de la petición
  String head = "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"image\"; filename=\"esp32cam.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";
  
  String tail = "\r\n--" + boundary + "--\r\n";
  
  uint32_t totalLen = head.length() + fb->len + tail.length();
  
  http.addHeader("Content-Type", contentType);
  http.addHeader("Content-Length", String(totalLen));
  
  // Crear buffer completo
  uint8_t *fbBuf = (uint8_t *)malloc(totalLen);
  if (!fbBuf) {
    DEBUG_PRINTLN("Error al asignar memoria para envío");
    http.end();
    return false;
  }
  
  // Copiar datos al buffer
  memcpy(fbBuf, head.c_str(), head.length());
  memcpy(fbBuf + head.length(), fb->buf, fb->len);
  memcpy(fbBuf + head.length() + fb->len, tail.c_str(), tail.length());
  
  // Enviar petición
  int httpCode = http.POST(fbBuf, totalLen);
  
  // Liberar buffer
  free(fbBuf);
  
  bool success = (httpCode == 200);
  
  if (!success && httpCode > 0) {
    DEBUG_PRINTF("Error HTTP: %d\n", httpCode);
  }
  
  http.end();
  
  return success;
}

// ============================================================================
// UTILIDADES
// ============================================================================

void printStatus() {
  DEBUG_PRINTLN("Estado del sistema:");
  DEBUG_PRINTLN("  WiFi SSID: " + String(WIFI_SSID));
  DEBUG_PRINTLN("  IP Local: " + WiFi.localIP().toString());
  DEBUG_PRINTLN("  Servidor: " + String(SERVER_IP) + ":" + String(SERVER_PORT));
  DEBUG_PRINTLN("  Resolución captura: " + String(FRAME_SIZE_CAPTURE));
  DEBUG_PRINTLN("  Resolución streaming: " + String(FRAME_SIZE_STREAM));
  DEBUG_PRINTLN("  Calidad JPEG captura: " + String(JPEG_QUALITY_CAPTURE));
  DEBUG_PRINTLN("  Calidad JPEG streaming: " + String(JPEG_QUALITY_STREAM));
  DEBUG_PRINTLN("  Memoria libre: " + String(ESP.getFreeHeap() / 1024) + " KB");
}

void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_FLASH_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_FLASH_PIN, LOW);
    delay(delayMs);
  }
}

