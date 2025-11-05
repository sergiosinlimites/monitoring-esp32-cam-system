# Configuración del Streaming MJPEG

## Cambios Realizados

El sistema de streaming ha sido actualizado para usar **MJPEG streaming directo** desde el ESP32-CAM, similar al ejemplo CameraWebServer. Este método es mucho más eficiente que el anterior sistema de Socket.IO con frames individuales.

## Cómo Funciona

1. **ESP32-CAM**: Ahora tiene un servidor HTTP integrado en el puerto 81 que sirve streaming MJPEG en el endpoint `/stream`
2. **Frontend**: Se conecta directamente al ESP32 para recibir el video en tiempo real
3. **Servidor Flask**: Ya no se usa para el streaming, solo para la captura de fotos

## Pasos para Usar

### 1. Compilar y Subir el Código al ESP32

```bash
cd /home/sergio/Documents/TPI/esp32_cam

# Compilar
pio run

# Subir al ESP32 (asegúrate de tener permisos en /dev/ttyUSB0)
pio run --target upload

# Ver los logs del ESP32
pio device monitor
```

### 2. Obtener la IP del ESP32

Cuando el ESP32 se inicie, verás en el monitor serial algo como:

```
ESP32-CAM listo y operando...
IP Local: 192.168.x.x
Streaming MJPEG: http://192.168.x.x:81/stream
```

**Anota esta IP**, la necesitarás para el streaming.

### 3. Actualizar la IP en el Frontend (Opcional)

La IP del ESP32 se detecta automáticamente cuando el ESP32 envía la primera foto. Sin embargo, también puedes configurarla manualmente:

Edita `/home/sergio/Documents/TPI/server/static/js/streaming.js` línea 15:

```javascript
let ESP32_IP = '192.168.x.x'; // Reemplaza con la IP de tu ESP32
```

### 4. Iniciar el Servidor Flask

```bash
cd /home/sergio/Documents/TPI/server
source venv/bin/activate
python app.py
```

### 5. Usar el Sistema

1. Abre tu navegador en `http://localhost:5000`
2. **Para Captura de Fotos**: 
   - Ve a la pestaña "Capture"
   - Haz clic en "Capture Photo"
   - La foto se guardará en `server/static/images/`

3. **Para Streaming en Vivo**:
   - Ve a la pestaña "Streaming"
   - Haz clic en "Start Streaming"
   - El video se mostrará en tiempo real desde el ESP32

## Ventajas del Nuevo Sistema

✅ **Mucho más rápido**: Streaming directo sin intermediarios
✅ **Menor latencia**: No hay retrasos por Socket.IO
✅ **Más eficiente**: Menos carga en el servidor Flask
✅ **Estándar MJPEG**: Compatible con cualquier navegador
✅ **Mejor FPS**: Hasta 30 FPS dependiendo de la red

## Solución de Problemas

### El streaming no se muestra

1. **Verifica que el ESP32 esté encendido y conectado al WiFi**:
   ```bash
   pio device monitor
   ```
   Deberías ver que se conectó exitosamente.

2. **Verifica que la IP sea correcta**:
   - La IP del ESP32 se muestra en el monitor serial
   - Puedes probar acceder directamente en el navegador: `http://<IP_ESP32>:81/stream`

3. **Verifica CORS**: 
   - Algunos navegadores bloquean conexiones entre diferentes orígenes
   - Intenta acceder primero a `http://<IP_ESP32>:81/stream` directamente en otra pestaña

### El ESP32 no se conecta al WiFi

1. Verifica las credenciales en `/home/sergio/Documents/TPI/esp32_cam/src/config.h`:
   ```cpp
   #define WIFI_SSID "tu_red"
   #define WIFI_PASSWORD "tu_password"
   ```

2. Asegúrate de que el ESP32 y tu computadora estén en la misma red

### Problemas de permisos con /dev/ttyUSB0

```bash
# Agregar tu usuario al grupo dialout
sudo usermod -a -G dialout $USER

# Reiniciar para que los cambios surtan efecto
sudo reboot
```

## Configuraciones Adicionales

### Cambiar la resolución del streaming

Edita `/home/sergio/Documents/TPI/esp32_cam/src/config.h`:

```cpp
// Para más FPS pero menor calidad
#define FRAME_SIZE_STREAM  FRAMESIZE_QVGA    // 320x240

// Para mejor calidad pero menos FPS
#define FRAME_SIZE_STREAM  FRAMESIZE_VGA     // 640x480
```

### Cambiar el puerto del streaming

Edita `/home/sergio/Documents/TPI/esp32_cam/src/main.cpp` línea 96:

```cpp
if (startStreamServer(81)) {  // Cambiar 81 por otro puerto
```

Y actualiza en `/home/sergio/Documents/TPI/server/static/js/streaming.js`:

```javascript
const ESP32_STREAM_PORT = 81; // Cambiar al mismo puerto
```

### Activar/Desactivar el Flash

Edita `/home/sergio/Documents/TPI/esp32_cam/src/config.h`:

```cpp
#define USE_FLASH true   // true = activado, false = desactivado
```

## URLs del Sistema

- **Frontend/Interfaz Web**: `http://localhost:5000`
- **Stream MJPEG Directo**: `http://<IP_ESP32>:81/stream`
- **API del Servidor**: `http://localhost:5000/api/`

## Arquitectura

```
┌─────────────────┐
│   Navegador     │
│   (Frontend)    │
└────┬────────┬───┘
     │        │
     │        └────────────────────────┐
     │                                 │
     │ (Socket.IO + REST)              │ (HTTP directo)
     │                                 │
┌────▼────────────┐              ┌────▼────────────┐
│ Servidor Flask  │              │   ESP32-CAM     │
│   (Puerto 5000) │◄─────────────┤ (Puerto 81)     │
└─────────────────┘  (HTTP POST) │ Servidor MJPEG  │
                     Captura Fotos└─────────────────┘
```

## Archivos Modificados

- `/home/sergio/Documents/TPI/esp32_cam/src/main.cpp` - Añadido servidor de streaming
- `/home/sergio/Documents/TPI/esp32_cam/src/streaming.h` - Nuevo archivo con el servidor MJPEG
- `/home/sergio/Documents/TPI/esp32_cam/platformio.ini` - Actualizado con nuevas librerías
- `/home/sergio/Documents/TPI/server/static/js/streaming.js` - Reescrito para usar MJPEG directo
- `/home/sergio/Documents/TPI/server/app.py` - Agregado endpoint para detectar IP del ESP32

## Siguiente Paso

Compila y sube el código al ESP32:

```bash
cd /home/sergio/Documents/TPI/esp32_cam
pio run --target upload
```

¡Y listo! El streaming debería funcionar ahora. 🎥

