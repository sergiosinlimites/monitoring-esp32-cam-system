/**
 * Servidor de Streaming MJPEG para ESP32-CAM
 * Basado en el ejemplo CameraWebServer
 */

#ifndef STREAMING_H
#define STREAMING_H

#include "esp_http_server.h"
#include "esp_camera.h"
#include "esp_timer.h"

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

// Handler para el streaming MJPEG
static esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char part_buf[64];

    // Configurar tipo de respuesta
    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }

    // Agregar header CORS
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    Serial.println(">>> Cliente de streaming conectado <<<");

    // Bucle principal de streaming
    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Error al capturar frame para streaming");
            res = ESP_FAIL;
            break;
        }

        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;

        // Enviar boundary
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }

        // Enviar headers del frame
        if (res == ESP_OK) {
            size_t hlen = snprintf(part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, part_buf, hlen);
        }

        // Enviar datos del frame
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }

        // Liberar frame buffer
        esp_camera_fb_return(fb);
        fb = NULL;

        if (res != ESP_OK) {
            break;
        }

        // Pequeño delay para no saturar
        delay(30); // ~33 FPS max
    }

    Serial.println(">>> Cliente de streaming desconectado <<<");
    return res;
}

// Iniciar servidor de streaming
bool startStreamServer(int port = 81) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.ctrl_port = port + 1;

    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL
    };

    Serial.printf("Iniciando servidor de streaming en puerto: %d\n", config.server_port);
    
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
        Serial.println("✓ Servidor de streaming iniciado");
        return true;
    }

    Serial.println("✗ Error al iniciar servidor de streaming");
    return false;
}

// Detener servidor de streaming
void stopStreamServer() {
    if (stream_httpd != NULL) {
        httpd_stop(stream_httpd);
        stream_httpd = NULL;
    }
}

#endif // STREAMING_H

