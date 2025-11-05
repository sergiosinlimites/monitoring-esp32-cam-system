# Sistema de Cámara Trampa con ESP32-CAM para Hipopótamos

Sistema completo de monitoreo de fauna mediante ESP32-CAM que captura imágenes bajo demanda y transmite video en tiempo real a un servidor Flask, con interfaz web para visualización y control.

## 📋 Características

- ✅ Captura de fotos bajo demanda desde interfaz web
- ✅ Streaming de video en tiempo real
- ✅ Interfaz web moderna con Bootstrap 5
- ✅ Comunicación bidireccional con WebSocket
- ✅ Galería de imágenes capturadas
- ✅ Métricas en tiempo real (FPS, latencia)
- ✅ Código modular y bien documentado

## 🏗️ Arquitectura

```
ESP32-CAM (Cliente IoT) ←→ Servidor Flask (Backend) ←→ Navegador Web (Frontend)
```

- **ESP32-CAM**: Captura imágenes y las envía al servidor
- **Servidor Flask**: Gestiona comunicación, almacena imágenes, sirve interfaz web
- **Frontend Web**: Interfaz para control y visualización (2 pestañas)

## 📦 Requisitos

### Hardware
- ESP32-CAM (módulo AI-Thinker)
- Cable USB-TTL (FTDI) para programación
- Fuente de alimentación 5V 2A mínimo
- Computadora con Python 3.8+

### Software
- **Servidor:**
  - Python 3.8 o superior
  - pip (gestor de paquetes de Python)
  
- **ESP32-CAM:**
  - PlatformIO (extensión de VS Code o CLI)
  - Drivers USB-TTL

## 🚀 Instalación y Configuración

### Paso 1: Clonar o Descargar el Proyecto

El proyecto ya está en `/media/sergio/DATOS/TPI`

### Paso 2: Configurar el Servidor Flask

#### 2.1. Crear entorno virtual de Python

```bash
cd /media/sergio/DATOS/TPI/server

# Crear entorno virtual
python3 -m venv venv

# Activar entorno virtual
source venv/bin/activate  # Linux/Mac
# En Windows: venv\Scripts\activate
```

#### 2.2. Instalar dependencias

```bash
pip install -r requirements.txt
```

#### 2.3. Obtener la IP de tu computadora

El ESP32-CAM necesita conocer la IP de tu computadora en la red local.

**Linux:**
```bash
hostname -I
# o
ip addr show
```

**Windows:**
```cmd
ipconfig
```

**Mac:**
```bash
ifconfig
```

Anota la IP (ejemplo: `192.168.1.100`)

### Paso 3: Configurar ESP32-CAM

#### 3.1. Instalar PlatformIO

Si usas VS Code:
1. Instalar extensión "PlatformIO IDE"
2. Reiniciar VS Code

#### 3.2. Configurar credenciales WiFi y servidor

Edita el archivo `esp32_cam/src/config.h`:

```cpp
// CONFIGURACIÓN DE WIFI
#define WIFI_SSID "TU_SSID_AQUI"          // Reemplaza con tu SSID
#define WIFI_PASSWORD "TU_PASSWORD_AQUI"   // Reemplaza con tu contraseña

// CONFIGURACIÓN DEL SERVIDOR FLASK
#define SERVER_IP "192.168.1.100"  // Reemplaza con la IP de tu PC
#define SERVER_PORT 5000
```

**IMPORTANTE:** 
- El WiFi debe ser de 2.4GHz (ESP32 no soporta 5GHz)
- El ESP32-CAM y tu computadora deben estar en la misma red

#### 3.3. Conectar ESP32-CAM para programación

1. Conecta el adaptador USB-TTL al ESP32-CAM:
   ```
   FTDI          ESP32-CAM
   ----          ---------
   GND    →      GND
   5V     →      5V
   TX     →      RX (U0R)
   RX     →      TX (U0T)
   ```

2. **Para entrar en modo programación:**
   - Conecta GPIO0 a GND
   - Conecta el USB
   - Presiona el botón RESET (si tiene)

#### 3.4. Cargar código al ESP32-CAM

**Desde VS Code con PlatformIO:**
1. Abrir carpeta `esp32_cam`
2. Click en el ícono de PlatformIO (hormiga)
3. Click en "Upload"

**Desde terminal:**
```bash
cd /media/sergio/DATOS/TPI/esp32_cam
pio run --target upload
```

#### 3.5. Desconectar GPIO0 y reiniciar

1. Desconecta GPIO0 de GND
2. Presiona RESET o desconecta/conecta la alimentación

#### 3.6. Verificar funcionamiento

Abre el monitor serial:

**VS Code:** Click en "Monitor" en PlatformIO

**Terminal:**
```bash
pio device monitor
```

Deberías ver:
```
============================================================
ESP32-CAM Cámara Trampa para Hipopótamos
============================================================
[1/2] Inicializando cámara...
✓ Cámara inicializada correctamente
[2/2] Conectando a WiFi...
✓ Conectado a WiFi
...
ESP32-CAM listo y operando...
```

### Paso 4: Iniciar el Servidor Flask

```bash
cd /media/sergio/DATOS/TPI/server

# Activar entorno virtual si no está activado
source venv/bin/activate

# Iniciar servidor
python app.py
```

Deberías ver:
```
============================================================
Servidor Flask para ESP32-CAM Cámara Trampa
============================================================
Host: 0.0.0.0
Puerto: 5000
Carpeta de imágenes: /media/sergio/DATOS/TPI/server/static/images
============================================================
Servidor iniciado. Presiona Ctrl+C para detener.
============================================================
```

### Paso 5: Acceder a la Interfaz Web

Abre tu navegador y ve a:

```
http://localhost:5000
```

O desde otro dispositivo en la misma red:
```
http://[IP_DE_TU_PC]:5000
```

## 🎮 Uso del Sistema

### Pestaña 1: Captura de Fotos

1. **Capturar Foto:**
   - Click en el botón "Capturar Foto"
   - El servidor envía señal al ESP32-CAM
   - ESP32-CAM captura y envía la foto
   - La imagen aparece automáticamente en la galería

2. **Ver Imágenes:**
   - Click en cualquier imagen para verla en grande
   - Botón de descarga para guardar localmente

3. **Actualizar Galería:**
   - Click en "Actualizar Galería" para recargar las imágenes

### Pestaña 2: Streaming en Vivo

1. **Iniciar Streaming:**
   - Click en "Iniciar Streaming"
   - El ESP32-CAM comienza a enviar frames continuos
   - El video aparece en tiempo real

2. **Detener Streaming:**
   - Click en "Detener Streaming"
   - El ESP32-CAM deja de enviar frames

3. **Métricas:**
   - FPS: Frames por segundo
   - Latencia: Delay en milisegundos
   - Frames recibidos: Total de frames

## 📁 Estructura del Proyecto

```
TPI/
├── server/
│   ├── app.py                  # Servidor Flask principal
│   ├── config.py              # Configuraciones
│   ├── requirements.txt       # Dependencias Python
│   ├── static/
│   │   ├── css/
│   │   │   └── styles.css    # Estilos
│   │   ├── js/
│   │   │   ├── capture.js    # Lógica de captura
│   │   │   └── streaming.js  # Lógica de streaming
│   │   └── images/           # Imágenes guardadas
│   └── templates/
│       └── index.html        # Interfaz web
│
├── esp32_cam/
│   ├── platformio.ini        # Configuración PlatformIO
│   └── src/
│       ├── main.cpp          # Código principal
│       ├── config.h          # Configuración WiFi/servidor
│       └── camera_pins.h     # Definición de pines
│
├── README.md                 # Este archivo
└── PLANIFICACION_PROYECTO.md # Documentación de planificación
```

## 🔧 Configuración Avanzada

### Cambiar Resolución de Cámara

Edita `esp32_cam/src/config.h`:

```cpp
// Resoluciones disponibles:
// FRAMESIZE_QVGA    (320x240)
// FRAMESIZE_VGA     (640x480)
// FRAMESIZE_SVGA    (800x600)
// FRAMESIZE_XGA     (1024x768)
// FRAMESIZE_UXGA    (1600x1200)

#define FRAME_SIZE_CAPTURE FRAMESIZE_VGA     // Para fotos
#define FRAME_SIZE_STREAM  FRAMESIZE_QVGA    // Para streaming
```

### Cambiar Calidad JPEG

```cpp
// Calidad JPEG (0-63, menor = mejor calidad, mayor tamaño)
#define JPEG_QUALITY_CAPTURE 10   // Alta calidad
#define JPEG_QUALITY_STREAM  20   // Calidad media
```

### Cambiar Puerto del Servidor

**En `server/config.py`:**
```python
PORT = 5000  # Cambiar al puerto deseado
```

**En `esp32_cam/src/config.h`:**
```cpp
#define SERVER_PORT 5000  // Mismo puerto
```

### Ajustar FPS de Streaming

Edita `esp32_cam/src/config.h`:

```cpp
// Delay entre frames (milisegundos)
// Valores más bajos = más FPS
#define STREAMING_FRAME_DELAY 100  // ~10 FPS
// Para 20 FPS: 50
// Para 5 FPS: 200
```

## 🐛 Solución de Problemas

### ESP32-CAM no se conecta a WiFi

**Problema:** No puede conectarse a la red WiFi

**Soluciones:**
1. Verificar que SSID y contraseña sean correctos
2. Asegurarse de que el WiFi sea 2.4GHz (no 5GHz)
3. Verificar que el ESP32-CAM esté dentro del rango del router
4. Revisar el monitor serial para ver errores específicos

### No se muestran imágenes en la interfaz web

**Problema:** Las imágenes no aparecen en la galería

**Soluciones:**
1. Verificar que la carpeta `server/static/images` exista y tenga permisos de escritura
2. Abrir consola del navegador (F12) para ver errores
3. Verificar que el servidor Flask esté corriendo
4. Revisar logs del servidor para errores

### Streaming con mucho lag

**Problema:** El video se ve con retraso o frames perdidos

**Soluciones:**
1. Reducir resolución en `config.h`: usar `FRAMESIZE_QVGA` o inferior
2. Aumentar compresión JPEG: `JPEG_QUALITY_STREAM 30` o más
3. Aumentar el delay entre frames: `STREAMING_FRAME_DELAY 150`
4. Verificar la calidad de la conexión WiFi
5. Acercar el ESP32-CAM al router

### ESP32-CAM se reinicia constantemente

**Problema:** El dispositivo se reinicia en bucle

**Soluciones:**
1. **Verificar alimentación:** Usar fuente de al menos 5V 2A
2. Conectar capacitor de 100µF entre 5V y GND
3. No alimentar solo desde USB-TTL (insuficiente corriente)
4. Verificar conexiones físicas

### Error al subir código a ESP32-CAM

**Problema:** No puede cargar el código

**Soluciones:**
1. Verificar que GPIO0 esté conectado a GND durante la carga
2. Seleccionar el puerto serial correcto en PlatformIO
3. Presionar RESET mientras se inicia la carga
4. Verificar drivers USB-TTL instalados
5. Probar con velocidad de subida más baja: `upload_speed = 57600`

### No se puede acceder a la interfaz web desde otro dispositivo

**Problema:** Otros dispositivos en la red no pueden acceder

**Soluciones:**
1. Verificar que el servidor esté configurado con `HOST = '0.0.0.0'`
2. Desactivar firewall temporalmente para probar
3. Usar la IP correcta (no localhost) desde otros dispositivos
4. Verificar que ambos dispositivos estén en la misma red

## 📊 Endpoints de la API

### REST API

| Método | Endpoint | Descripción |
|--------|----------|-------------|
| POST | `/api/capture` | Solicitar captura de foto |
| GET | `/api/capture` | ESP32 consulta si debe capturar |
| POST | `/api/upload` | ESP32 envía imagen capturada |
| POST | `/api/stream-frame` | ESP32 envía frame de streaming |
| GET | `/api/images` | Listar todas las imágenes |
| GET | `/api/latest-image` | Obtener última imagen |
| GET/POST | `/api/streaming-status` | Estado del streaming |

### WebSocket (SocketIO)

**Eventos del Cliente → Servidor:**
- `connect` - Conexión establecida
- `disconnect` - Desconexión
- `request_capture` - Solicitar captura
- `start_streaming` - Iniciar streaming
- `stop_streaming` - Detener streaming

**Eventos del Servidor → Cliente:**
- `new_image` - Nueva imagen disponible
- `capture_status` - Estado de captura
- `stream_frame` - Frame de streaming
- `streaming_status` - Estado de streaming
- `connection_status` - Estado de conexión

## 🔒 Notas de Seguridad

⚠️ **Este proyecto es un prototipo para uso local.**

Para uso en producción, considerar:
- Implementar autenticación de usuarios
- Usar HTTPS con certificados SSL
- Validar y sanitizar todas las entradas
- Implementar rate limiting
- Restringir acceso por IP
- Usar contraseñas fuertes para WiFi

## 🎯 Próximos Pasos / Mejoras Futuras

- [ ] Detección de movimiento en ESP32-CAM
- [ ] Integración con algoritmos de ML para reconocimiento de hipopótamos
- [ ] Almacenamiento de imágenes en base de datos
- [ ] Soporte para múltiples ESP32-CAM
- [ ] Notificaciones por email/SMS
- [ ] Almacenamiento en la nube (AWS S3, Google Cloud)
- [ ] Aplicación móvil nativa
- [ ] Dashboard con estadísticas y análisis

## 📞 Soporte

Para problemas o preguntas:
1. Revisar la sección de "Solución de Problemas"
2. Verificar los logs del servidor y del ESP32-CAM
3. Consultar la documentación de planificación: `PLANIFICACION_PROYECTO.md`

## 📄 Licencia

Proyecto educativo para monitoreo de fauna.

## 🙏 Agradecimientos

- Comunidad de ESP32
- Documentación de Flask y Flask-SocketIO
- Tutoriales de Random Nerd Tutorials

---

**Desarrollado para monitoreo de hipopótamos con ESP32-CAM** 🦛📷

