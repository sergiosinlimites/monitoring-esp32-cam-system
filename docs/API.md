# Documentación de la API - Sistema de Cámara Trampa ESP32-CAM

Esta documentación detalla todos los endpoints y eventos de WebSocket disponibles en el servidor Flask.

## URL Base

```
http://[IP_SERVIDOR]:5000
```

Ejemplo: `http://192.168.1.100:5000`

---

## 📡 REST API Endpoints

### 1. Página Principal

**GET** `/`

Sirve la interfaz web HTML con las 2 pestañas (Captura y Streaming).

**Respuesta:**
- Código: `200 OK`
- Tipo: `text/html`
- Contenido: Página HTML

---

### 2. Solicitar/Consultar Captura

**POST** `/api/capture` (Desde Frontend)

Solicita al ESP32-CAM que capture una foto.

**Request Body:**
```json
{
  "trigger": true
}
```

**Respuesta:**
```json
{
  "status": "capture_requested",
  "timestamp": "2025-11-05 14:30:25"
}
```

**GET** `/api/capture` (Desde ESP32-CAM)

El ESP32-CAM consulta si debe capturar una foto.

**Respuesta:**
```json
{
  "capture": true,
  "timestamp": "2025-11-05 14:30:25"
}
```

o

```json
{
  "capture": false
}
```

---

### 3. Subir Imagen Capturada

**POST** `/api/upload`

El ESP32-CAM envía una imagen capturada al servidor.

**Request:**
- Tipo: `multipart/form-data`
- Campo: `image` (archivo JPEG)

**Respuesta exitosa:**
```json
{
  "status": "success",
  "filename": "capture_20251105_143025.jpg",
  "timestamp": "2025-11-05 14:30:25"
}
```

**Respuesta de error:**
```json
{
  "status": "error",
  "message": "No se encontró el campo image"
}
```

**Códigos de estado:**
- `200 OK` - Imagen subida exitosamente
- `400 Bad Request` - Error en la petición

---

### 4. Enviar Frame de Streaming

**POST** `/api/stream-frame`

El ESP32-CAM envía un frame para streaming en tiempo real.

**Request:**
- Tipo: `multipart/form-data`
- Campo: `image` (archivo JPEG)

**Respuesta:**
```json
{
  "status": "success"
}
```

**Nota:** Este endpoint automáticamente emite el frame a todos los clientes WebSocket conectados.

---

### 5. Listar Imágenes

**GET** `/api/images`

Obtiene la lista de todas las imágenes capturadas.

**Respuesta:**
```json
{
  "images": [
    {
      "filename": "capture_20251105_143025.jpg",
      "url": "/static/images/capture_20251105_143025.jpg",
      "size": 125487,
      "modified": "2025-11-05 14:30:25"
    },
    {
      "filename": "capture_20251105_142010.jpg",
      "url": "/static/images/capture_20251105_142010.jpg",
      "size": 134562,
      "modified": "2025-11-05 14:20:10"
    }
  ],
  "count": 2
}
```

**Nota:** Las imágenes están ordenadas de más reciente a más antigua.

---

### 6. Obtener Última Imagen

**GET** `/api/latest-image`

Obtiene información de la imagen más reciente.

**Respuesta exitosa:**
```json
{
  "filename": "capture_20251105_143025.jpg",
  "url": "/static/images/capture_20251105_143025.jpg",
  "timestamp": "2025-11-05 14:30:25"
}
```

**Respuesta sin imágenes:**
```json
{
  "status": "error",
  "message": "No hay imágenes disponibles"
}
```

---

### 7. Estado de Streaming

**GET** `/api/streaming-status` (Desde ESP32-CAM)

El ESP32-CAM consulta si hay clientes activos esperando streaming.

**Respuesta:**
```json
{
  "streaming": true
}
```

o

```json
{
  "streaming": false
}
```

**POST** `/api/streaming-status` (Desde Frontend)

El frontend activa o desactiva el streaming.

**Request Body:**
```json
{
  "active": true
}
```

**Respuesta:**
```json
{
  "status": "success",
  "streaming": true
}
```

---

### 8. Servir Imágenes

**GET** `/static/images/[filename]`

Sirve una imagen específica.

**Ejemplo:**
```
GET /static/images/capture_20251105_143025.jpg
```

**Respuesta:**
- Código: `200 OK`
- Tipo: `image/jpeg`
- Contenido: Archivo de imagen

---

## 🔌 WebSocket (Socket.IO)

### URL de Conexión

```javascript
const socket = io('http://[IP_SERVIDOR]:5000');
```

---

## Eventos: Cliente → Servidor

### connect

Se emite automáticamente cuando el cliente se conecta.

**No requiere datos adicionales**

---

### disconnect

Se emite automáticamente cuando el cliente se desconecta.

**No requiere datos adicionales**

---

### request_capture

Solicita captura de una foto.

**Datos enviados:**
```javascript
socket.emit('request_capture', {
  timestamp: "2025-11-05T14:30:25.000Z"
});
```

---

### start_streaming

Inicia el streaming de video.

**Datos enviados:**
```javascript
socket.emit('start_streaming', {
  timestamp: "2025-11-05T14:30:25.000Z"
});
```

---

### stop_streaming

Detiene el streaming de video.

**Datos enviados:**
```javascript
socket.emit('stop_streaming', {
  timestamp: "2025-11-05T14:30:25.000Z"
});
```

---

## Eventos: Servidor → Cliente

### connection_status

Informa sobre el estado de conexión del cliente.

**Datos recibidos:**
```javascript
{
  "status": "connected",
  "message": "Conectado al servidor",
  "timestamp": "2025-11-05 14:30:25"
}
```

**Escuchar evento:**
```javascript
socket.on('connection_status', function(data) {
  console.log(data.message);
});
```

---

### new_image

Notifica que hay una nueva imagen disponible.

**Datos recibidos:**
```javascript
{
  "filename": "capture_20251105_143025.jpg",
  "url": "/static/images/capture_20251105_143025.jpg",
  "timestamp": "2025-11-05 14:30:25"
}
```

**Escuchar evento:**
```javascript
socket.on('new_image', function(data) {
  console.log('Nueva imagen:', data.filename);
  // Actualizar galería
});
```

---

### capture_status

Informa sobre el estado de una captura solicitada.

**Datos recibidos:**
```javascript
{
  "status": "requested",
  "message": "Solicitud de captura enviada a ESP32-CAM",
  "timestamp": "2025-11-05 14:30:25"
}
```

**Estados posibles:**
- `requested` - Captura solicitada
- `success` - Captura exitosa
- `error` - Error en captura

**Escuchar evento:**
```javascript
socket.on('capture_status', function(data) {
  console.log('Estado de captura:', data.message);
});
```

---

### stream_frame

Envía un frame de video para streaming.

**Datos recibidos:**
```javascript
{
  "image": "base64_encoded_jpeg_data...",
  "timestamp": "2025-11-05 14:30:25.123"
}
```

**Escuchar evento:**
```javascript
socket.on('stream_frame', function(data) {
  const img = new Image();
  img.src = 'data:image/jpeg;base64,' + data.image;
  // Mostrar en canvas
});
```

---

### streaming_status

Informa sobre cambios en el estado del streaming.

**Datos recibidos:**
```javascript
{
  "active": true,
  "message": "Streaming iniciado",
  "timestamp": "2025-11-05 14:30:25"
}
```

**Escuchar evento:**
```javascript
socket.on('streaming_status', function(data) {
  console.log('Streaming activo:', data.active);
});
```

---

## 📝 Ejemplos de Uso

### Ejemplo 1: Solicitar Captura (JavaScript)

```javascript
// Método 1: Usando REST API
fetch('/api/capture', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json'
  },
  body: JSON.stringify({ trigger: true })
})
.then(response => response.json())
.then(data => {
  console.log('Captura solicitada:', data);
});

// Método 2: Usando WebSocket
socket.emit('request_capture', {
  timestamp: new Date().toISOString()
});
```

### Ejemplo 2: ESP32-CAM Verifica Captura (C++)

```cpp
HTTPClient http;
http.begin("http://192.168.1.100:5000/api/capture");
int httpCode = http.GET();

if (httpCode == 200) {
  String payload = http.getString();
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  
  bool shouldCapture = doc["capture"];
  
  if (shouldCapture) {
    // Capturar y enviar foto
    captureAndSendPhoto();
  }
}

http.end();
```

### Ejemplo 3: ESP32-CAM Sube Imagen (C++)

```cpp
HTTPClient http;
http.begin("http://192.168.1.100:5000/api/upload");

String boundary = "ESP32CAM-1234";
String contentType = "multipart/form-data; boundary=" + boundary;

// Construir cuerpo multipart
String head = "--" + boundary + "\r\n";
head += "Content-Disposition: form-data; name=\"image\"; filename=\"photo.jpg\"\r\n";
head += "Content-Type: image/jpeg\r\n\r\n";

String tail = "\r\n--" + boundary + "--\r\n";

// Combinar: head + imageData + tail
// ...

int httpCode = http.POST(fullData, totalLength);

if (httpCode == 200) {
  Serial.println("Imagen subida exitosamente");
}

http.end();
```

### Ejemplo 4: Recibir Frames de Streaming (JavaScript)

```javascript
const canvas = document.getElementById('streamCanvas');
const ctx = canvas.getContext('2d');

socket.on('stream_frame', function(data) {
  const img = new Image();
  
  img.onload = function() {
    canvas.width = img.width;
    canvas.height = img.height;
    ctx.drawImage(img, 0, 0);
  };
  
  img.src = 'data:image/jpeg;base64,' + data.image;
});
```

---

## 🔧 Códigos de Estado HTTP

| Código | Significado |
|--------|-------------|
| 200 | OK - Petición exitosa |
| 400 | Bad Request - Error en los datos enviados |
| 404 | Not Found - Recurso no encontrado |
| 500 | Internal Server Error - Error del servidor |

---

## 🚨 Manejo de Errores

### REST API

Todas las respuestas de error siguen este formato:

```json
{
  "status": "error",
  "message": "Descripción del error"
}
```

### WebSocket

Los errores se reportan mediante eventos específicos con el campo `status`:

```javascript
{
  "status": "error",
  "message": "Descripción del error",
  "timestamp": "2025-11-05 14:30:25"
}
```

---

## 📊 Límites y Restricciones

- **Tamaño máximo de imagen:** 16 MB
- **Formatos permitidos:** JPG, JPEG, PNG
- **Timeout de peticiones HTTP:** 5 segundos
- **Conexiones WebSocket simultáneas:** Sin límite (limitado por recursos del servidor)

---

## 🔒 Consideraciones de Seguridad

⚠️ **Esta API no tiene autenticación implementada**

Para producción, considerar:
- Implementar autenticación con tokens (JWT)
- Usar HTTPS
- Validar origen de peticiones (CORS estricto)
- Implementar rate limiting
- Sanitizar nombres de archivos

---

## 📞 Contacto y Soporte

Para más información, consultar:
- `README.md` - Guía de uso general
- `PLANIFICACION_PROYECTO.md` - Documentación de planificación

---

**Última actualización:** 2025-11-05

