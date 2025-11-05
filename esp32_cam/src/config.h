/**
 * Archivo de Configuración para ESP32-CAM
 * 
 * IMPORTANTE: Modifica estos valores según tu configuración de red y servidor
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// CONFIGURACIÓN DE WIFI
// ============================================================================

// Reemplaza con tus credenciales de WiFi
#define WIFI_SSID "moto"
#define WIFI_PASSWORD "Un*1234@"

// Tiempo máximo de espera para conectar a WiFi (milisegundos)
#define WIFI_TIMEOUT 60000

// ============================================================================
// CONFIGURACIÓN DEL SERVIDOR FLASK
// ============================================================================

// IP del servidor Flask (reemplaza con la IP de tu computadora en la red local)
// Para obtener tu IP:
// - Linux: ip addr show o hostname -I
// - Windows: ipconfig
// - Mac: ifconfig
#define SERVER_IP "10.185.27.42"

// Puerto del servidor Flask
#define SERVER_PORT 5000

// URLs de endpoints del servidor
#define SERVER_URL_CAPTURE "http://" SERVER_IP ":" STR(SERVER_PORT) "/api/capture"
#define SERVER_URL_UPLOAD "http://" SERVER_IP ":" STR(SERVER_PORT) "/api/upload"
#define SERVER_URL_STREAM "http://" SERVER_IP ":" STR(SERVER_PORT) "/api/stream-frame"
#define SERVER_URL_STREAMING_STATUS "http://" SERVER_IP ":" STR(SERVER_PORT) "/api/streaming-status"

// Macro auxiliar para convertir número a string
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// ============================================================================
// CONFIGURACIÓN DE LA CÁMARA
// ============================================================================

// Modelo de cámara (AI-Thinker ESP32-CAM)
#define CAMERA_MODEL_AI_THINKER

// Resolución de imagen
// Opciones disponibles:
// FRAMESIZE_QVGA    (320x240)
// FRAMESIZE_VGA     (640x480)   <- Recomendado para captura
// FRAMESIZE_SVGA    (800x600)
// FRAMESIZE_XGA     (1024x768)
// FRAMESIZE_UXGA    (1600x1200)
#define FRAME_SIZE_CAPTURE FRAMESIZE_VGA     // Para fotos capturadas
#define FRAME_SIZE_STREAM  FRAMESIZE_QVGA    // Para streaming (menor resolución = más FPS)

// Calidad JPEG (0-63, menor número = mejor calidad, mayor tamaño)
#define JPEG_QUALITY_CAPTURE 10   // Alta calidad para fotos
#define JPEG_QUALITY_STREAM  20   // Calidad media para streaming

// ============================================================================
// CONFIGURACIÓN DE TEMPORIZACIÓN
// ============================================================================

// Intervalo para verificar si debe capturar foto (milisegundos)
#define CAPTURE_CHECK_INTERVAL 1000  // 1 segundo

// Intervalo para verificar si debe hacer streaming (milisegundos)
#define STREAMING_CHECK_INTERVAL 5000  // 5 segundos

// Delay entre frames de streaming (milisegundos)
// Valores más bajos = más FPS pero más carga de red
#define STREAMING_FRAME_DELAY 100  // ~10 FPS

// Timeout para peticiones HTTP (milisegundos)
#define HTTP_TIMEOUT 5000

// ============================================================================
// CONFIGURACIÓN DE DEBUG
// ============================================================================

// Habilitar mensajes de debug en Serial Monitor
#define DEBUG_MODE true

// Macro para imprimir mensajes de debug
#if DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x, ...) Serial.printf(x, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, ...)
#endif

// ============================================================================
// CONFIGURACIÓN DE LED
// ============================================================================

// Pin del LED flash (opcional)
#define LED_FLASH_PIN 4

// Usar flash al capturar foto
// NOTA: Si las fotos salen oscuras, intenta con false
// Si las fotos con flash salen negras, el delay puede ser demasiado corto
#define USE_FLASH true

#endif // CONFIG_H

