# Planificación: Sistema de Cámara Trampa con ESP32-CAM para Hipopótamos

## 📋 Descripción General del Proyecto

Sistema de monitoreo de fauna mediante ESP32-CAM que captura imágenes bajo demanda y transmite video en tiempo real a un servidor Flask, con interfaz web para visualización y control.

---

## 🏗️ Arquitectura del Sistema

```
┌─────────────────┐
│   ESP32-CAM     │
│  (Cliente IoT)  │
└────────┬────────┘
         │
         │ WiFi (HTTP/WebSocket)
         │
         ▼
┌─────────────────────────┐
│   Servidor Flask        │
│   (Backend Python)      │
│                         │
│  - API REST             │
│  - WebSocket Server     │
│  - Almacenamiento       │
└────────┬────────────────┘
         │
         │ HTTP/WebSocket
         │
         ▼
┌─────────────────────────┐
│   Interfaz Web          │
│   (Frontend HTML/JS)    │
│                         │
│  - Pestaña Captura      │
│  - Pestaña Streaming    │
└─────────────────────────┘
```

---

## 🔧 Tecnologías y Herramientas

### Backend (Servidor)
- **Python 3.8+**
- **Flask** - Framework web principal
- **Flask-CORS** - Manejo de CORS para peticiones cross-origin
- **Flask-SocketIO** - Comunicación en tiempo real con WebSocket
- **python-socketio** - Servidor WebSocket
- **Pillow** - Procesamiento de imágenes
- **eventlet** o **gevent** - Servidor asíncrono

### Frontend (Interfaz Web)
- **HTML5** - Estructura
- **CSS3** - Estilos (con Bootstrap 5 para diseño responsive)
- **JavaScript (Vanilla)** - Lógica del cliente
- **Socket.IO Client** - Cliente WebSocket para streaming
- **Bootstrap 5** - Framework CSS para UI moderna

### Hardware y Firmware (ESP32-CAM)
- **ESP32-CAM** (módulo AI-Thinker)
- **PlatformIO**
- **Librerías Arduino:**
  - `WiFi.h` - Conectividad WiFi
  - `HTTPClient.h` - Cliente HTTP
  - `esp_camera.h` - Control de cámara
  - `ArduinoJson.h` - Manejo de JSON
  - `WebSocketsClient.h` - Cliente WebSocket (opcional)

---

## 📁 Estructura del Proyecto

```
TPI/
├── server/
│   ├── app.py                  # Aplicación Flask principal
│   ├── requirements.txt        # Dependencias Python
│   ├── config.py              # Configuraciones del servidor
│   ├── static/
│   │   ├── css/
│   │   │   └── styles.css     # Estilos personalizados
│   │   ├── js/
│   │   │   ├── capture.js     # Lógica pestaña de captura
│   │   │   └── streaming.js   # Lógica pestaña de streaming
│   │   └── images/            # Imágenes capturadas
│   └── templates/
│       └── index.html         # Interfaz web principal
│
├── esp32_cam/
│   ├── esp32_cam.ino          # Código principal ESP32
│   ├── camera_pins.h          # Definición de pines
│   └── config.h               # Configuración WiFi y servidor
│
├── docs/
│   └── API.md                 # Documentación de endpoints
│
├── README.md                  # Documentación general
└── PLANIFICACION_PROYECTO.md  # Este documento
```

---

## 🔌 Endpoints y APIs

### API REST (Flask)

#### 1. **POST** `/api/capture`
- **Descripción:** Comando para que ESP32 capture una foto
- **Request:** `{ "trigger": true }`
- **Response:** `{ "status": "capture_requested", "timestamp": "..." }`

#### 2. **POST** `/api/upload`
- **Descripción:** ESP32 envía imagen capturada
- **Request:** `multipart/form-data` con campo `image`
- **Response:** `{ "status": "success", "filename": "...", "timestamp": "..." }`

#### 3. **GET** `/api/images`
- **Descripción:** Lista todas las imágenes capturadas
- **Response:** `{ "images": [...], "count": 10 }`

#### 4. **GET** `/api/latest-image`
- **Descripción:** Obtiene la última imagen capturada
- **Response:** Archivo de imagen o JSON con URL

#### 5. **GET** `/`
- **Descripción:** Página principal con interfaz web
- **Response:** HTML

### WebSocket (SocketIO)

#### Eventos del Cliente (Frontend → Servidor)
- `connect` - Cliente se conecta
- `disconnect` - Cliente se desconecta
- `request_capture` - Solicitar captura de foto

#### Eventos del Servidor (Servidor → Frontend)
- `new_image` - Nueva imagen disponible
- `capture_status` - Estado de captura
- `stream_frame` - Frame de streaming en tiempo real

#### Eventos ESP32 (ESP32 → Servidor)
- `stream_frame` - ESP32 envía frames para streaming
- `device_status` - Estado del dispositivo

---

## 🔄 Flujos de Comunicación

### Flujo 1: Captura de Foto Bajo Demanda

```
Usuario (Web) → Servidor Flask → ESP32-CAM
     │               │                │
     │  Click botón  │                │
     ├──────────────►│                │
     │               │  POST /capture │
     │               ├───────────────►│
     │               │                │ Captura foto
     │               │                │
     │               │  POST /upload  │
     │               │◄───────────────┤
     │               │                │
     │  Notificación │                │
     │◄──────────────┤                │
     │  (WebSocket)  │                │
```

### Flujo 2: Streaming en Tiempo Real

```
ESP32-CAM → Servidor Flask → Cliente Web
    │            │                │
    │ Frames     │                │
    │ continuos  │                │
    ├───────────►│                │
    │ (HTTP POST)│  WebSocket     │
    │            ├───────────────►│
    │            │  stream_frame  │ Display
    │            │                │
```

---

## 🚀 Inicialización del Proyecto

### Paso 1: Preparar el Entorno del Servidor

```bash
# Navegar al directorio del proyecto
cd /media/sergio/DATOS/TPI

# Crear estructura de directorios
mkdir -p server/static/{css,js,images}
mkdir -p server/templates
mkdir -p esp32_cam
mkdir -p docs

# Crear entorno virtual Python
python3 -m venv venv

# Activar entorno virtual
source venv/bin/activate  # Linux/Mac
# venv\Scripts\activate   # Windows

# Instalar dependencias (después de crear requirements.txt)
pip install -r server/requirements.txt
```

### Paso 2: Configurar ESP32-CAM

```bash
# Instalar Arduino IDE o PlatformIO

# Instalar librerías necesarias:
# - ESP32 Board Support (en Board Manager)
# - ArduinoJson (en Library Manager)
# - WebSockets (por Links2004)
```

### Paso 3: Configurar WiFi y Servidor

Editar `esp32_cam/config.h` con:
- SSID de WiFi
- Password de WiFi
- IP del servidor Flask
- Puerto del servidor

### Paso 4: Cargar Código a ESP32-CAM

1. Conectar ESP32-CAM con adaptador FTDI
2. Conectar GPIO0 a GND para modo programación
3. Seleccionar placa "AI Thinker ESP32-CAM"
4. Cargar el sketch
5. Desconectar GPIO0 de GND
6. Reiniciar ESP32

### Paso 5: Iniciar Servidor

```bash
cd server
python app.py
```

El servidor estará disponible en: `http://localhost:5000`

---

## 🔐 Configuraciones Importantes

### Servidor Flask (config.py)

```python
# Puerto del servidor
PORT = 5000

# Host
HOST = '0.0.0.0'  # Accesible desde red local

# Carpeta de imágenes
UPLOAD_FOLDER = 'static/images'

# Tamaño máximo de archivo (16MB)
MAX_CONTENT_LENGTH = 16 * 1024 * 1024

# Formato de imágenes permitido
ALLOWED_EXTENSIONS = {'jpg', 'jpeg', 'png'}
```

### ESP32-CAM (config.h)

```cpp
// WiFi
#define WIFI_SSID "TU_SSID"
#define WIFI_PASSWORD "TU_PASSWORD"

// Servidor
#define SERVER_IP "192.168.1.100"  // IP del servidor Flask
#define SERVER_PORT 5000

// Cámara
#define CAMERA_MODEL_AI_THINKER
#define FRAME_SIZE FRAMESIZE_VGA  // 640x480
#define JPEG_QUALITY 10  // 0-63 (menor = mejor calidad)
```

---

## 📱 Interfaz Web - Funcionalidades

### Pestaña 1: Captura de Fotos

**Componentes:**
- Botón "Capturar Foto"
- Galería de imágenes capturadas
- Timestamp de cada captura
- Indicador de estado de conexión con ESP32

**Funcionalidades:**
- Click en botón → envía señal a ESP32 → ESP32 captura y envía foto
- Actualización automática al recibir nueva imagen
- Descarga de imágenes
- Eliminación de imágenes (opcional)

### Pestaña 2: Streaming en Tiempo Real

**Componentes:**
- Visor de video en tiempo real
- Indicador de FPS
- Indicador de latencia
- Botón para iniciar/detener streaming

**Funcionalidades:**
- Conexión WebSocket con servidor
- Recepción continua de frames
- Display de frames en canvas HTML5
- Control de calidad/resolución (opcional)

---

## 🔒 Consideraciones de Seguridad

1. **Autenticación:** Implementar login básico para acceso a la interfaz web
2. **HTTPS:** Usar certificados SSL en producción
3. **Validación:** Validar todas las imágenes recibidas
4. **Rate Limiting:** Limitar peticiones por IP para evitar abuso
5. **Sanitización:** Validar nombres de archivos para evitar path traversal

---

## ⚡ Optimizaciones

### ESP32-CAM
- Ajustar resolución según ancho de banda disponible
- Usar JPEG con compresión adecuada
- Implementar deep sleep entre capturas para ahorrar batería
- Usar buffer para frames de streaming

### Servidor Flask
- Implementar caché para imágenes frecuentes
- Comprimir imágenes antes de enviar al frontend
- Usar CDN para recursos estáticos
- Implementar base de datos (SQLite/PostgreSQL) para metadatos

### Frontend
- Lazy loading para galería de imágenes
- Paginación para lista de imágenes
- Comprimir recursos CSS/JS
- Usar Service Workers para caché offline

---

## 📊 Monitoreo y Logs

### Logs del Servidor
- Registro de todas las capturas
- Errores de conexión con ESP32
- Tiempo de respuesta de endpoints
- Uso de almacenamiento

### Logs de ESP32
- Estado de conexión WiFi
- Errores de cámara
- Memoria disponible
- Temperatura (si disponible)

---

## 🧪 Testing

### Tests del Servidor
- Pruebas unitarias de endpoints
- Pruebas de carga con múltiples clientes
- Pruebas de WebSocket
- Validación de subida de imágenes

### Tests de ESP32
- Prueba de conectividad WiFi
- Prueba de captura de imágenes
- Prueba de envío de datos
- Prueba de reconexión automática

---

## 📦 Dependencias del Proyecto

### Python (requirements.txt)
```
Flask==2.3.0
Flask-CORS==4.0.0
Flask-SocketIO==5.3.0
python-socketio==5.9.0
Pillow==10.0.0
eventlet==0.33.3
python-engineio==4.7.1
```

### ESP32 (Arduino Libraries)
- ESP32 by Espressif Systems (Board Support)
- ArduinoJson by Benoit Blanchon
- WebSockets by Markus Sattler (opcional para WebSocket)

---

## 🐛 Problemas Comunes y Soluciones

### ESP32-CAM no se conecta a WiFi
- Verificar SSID y password
- Verificar que WiFi sea 2.4GHz (ESP32 no soporta 5GHz)
- Verificar rango de señal

### Imágenes no se muestran en la web
- Verificar permisos de carpeta `static/images`
- Verificar CORS habilitado
- Verificar ruta de archivos

### Streaming con mucho lag
- Reducir resolución de imagen
- Aumentar compresión JPEG
- Verificar ancho de banda de red
- Reducir FPS

### ESP32-CAM se reinicia constantemente
- Verificar fuente de alimentación (mínimo 5V 2A)
- Verificar consumo de memoria
- Deshabilitar brownout detector si es necesario

---

## 🔮 Futuras Mejoras

1. **Detección de movimiento:** ESP32 captura solo cuando detecta movimiento
2. **Visión nocturna:** Soporte para luz IR
3. **Base de datos:** Almacenar metadatos en DB
4. **Machine Learning:** Integrar modelo de detección de hipopótamos
5. **Notificaciones:** Alertas por email/SMS cuando se detecta un animal
6. **Multi-cámara:** Soporte para múltiples ESP32-CAM
7. **Almacenamiento en nube:** Integración con AWS S3 o Google Cloud Storage
8. **App móvil:** Aplicación nativa para Android/iOS
9. **Análisis de datos:** Dashboard con estadísticas de detecciones
10. **Modo offline:** ESP32 guarda en SD card cuando no hay conexión

---

## 📚 Referencias y Recursos

### Documentación Oficial
- [Flask Documentation](https://flask.palletsprojects.com/)
- [Flask-SocketIO Documentation](https://flask-socketio.readthedocs.io/)
- [ESP32-CAM Datasheet](https://github.com/espressif/esp32-camera)
- [Arduino ESP32 Documentation](https://docs.espressif.com/projects/arduino-esp32/)

### Tutoriales Útiles
- Random Nerd Tutorials - ESP32-CAM
- Miguel Grinberg - Flask WebSocket
- ESP32.net - Camera Examples

### Herramientas
- Postman/Insomnia - Testing de APIs
- Chrome DevTools - Debug del frontend
- Arduino Serial Monitor - Debug de ESP32

---

## 👥 Equipo y Roles

- **Desarrollo Backend:** Servidor Flask, APIs, WebSocket
- **Desarrollo Frontend:** Interfaz web, JavaScript, UI/UX
- **Desarrollo Firmware:** Código ESP32-CAM, optimizaciones
- **Testing:** Pruebas de integración, pruebas de campo
- **Deployment:** Configuración de servidor, red, seguridad

---

## ✅ Checklist de Implementación

### Fase 1: Configuración Básica
- [ ] Crear estructura de directorios
- [ ] Configurar entorno virtual Python
- [ ] Instalar dependencias del servidor
- [ ] Configurar Arduino IDE para ESP32

### Fase 2: Backend
- [ ] Implementar servidor Flask básico
- [ ] Crear endpoints REST
- [ ] Implementar subida de archivos
- [ ] Configurar CORS
- [ ] Implementar WebSocket con SocketIO

### Fase 3: Frontend
- [ ] Crear estructura HTML con pestañas
- [ ] Implementar estilos con Bootstrap
- [ ] Desarrollar pestaña de captura
- [ ] Desarrollar pestaña de streaming
- [ ] Integrar cliente WebSocket

### Fase 4: ESP32-CAM
- [ ] Configurar pines de cámara
- [ ] Implementar conexión WiFi
- [ ] Implementar captura de fotos
- [ ] Implementar subida de imágenes
- [ ] Implementar streaming de video
- [ ] Implementar sistema de comandos

### Fase 5: Integración
- [ ] Probar comunicación ESP32 ↔ Servidor
- [ ] Probar comunicación Servidor ↔ Frontend
- [ ] Probar flujo completo de captura
- [ ] Probar streaming en tiempo real

### Fase 6: Testing y Optimización
- [ ] Pruebas de carga
- [ ] Optimizar latencia de streaming
- [ ] Optimizar consumo de memoria en ESP32
- [ ] Pruebas de campo

### Fase 7: Documentación
- [ ] Documentar código
- [ ] Crear guía de usuario
- [ ] Crear guía de instalación
- [ ] Documentar API

---

## 📝 Notas Finales

Este documento es una guía viva que se actualizará según avance el proyecto. Cualquier cambio en la arquitectura o requisitos debe reflejarse aquí.

**Fecha de creación:** 2025-11-05  
**Última actualización:** 2025-11-05  
**Versión:** 1.0

