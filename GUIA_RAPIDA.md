# 🚀 Guía Rápida de Inicio

Pasos esenciales para poner en funcionamiento el sistema de cámara trampa.

## ⚡ Inicio Rápido (5 minutos)

### 1. Configurar Servidor Flask

```bash
cd /media/sergio/DATOS/TPI/server

# Crear entorno virtual
python3 -m venv venv

# Activar entorno virtual
source venv/bin/activate

# Instalar dependencias
pip install -r requirements.txt
```

### 2. Obtener tu IP

```bash
hostname -I
```

Anota tu IP (ejemplo: `10.185.27.42`)

### 3. Configurar ESP32-CAM

Edita: `esp32_cam/src/config.h`

```cpp
#define WIFI_SSID "moto"           // ← Cambiar
#define WIFI_PASSWORD "Un*1234@"    // ← Cambiar
#define SERVER_IP "10.185.27.42"      // ← Tu IP del paso 2
```

### 4. Cargar Código a ESP32-CAM

```bash
cd /media/sergio/DATOS/TPI/esp32_cam

# Conectar GPIO0 a GND (modo programación)
# Conectar USB-TTL

pio run --target upload

# Desconectar GPIO0 de GND
# Presionar RESET
```

### 5. Iniciar Servidor

```bash
cd /media/sergio/DATOS/TPI

# Método 1: Usando script
./iniciar_servidor.sh

# Método 2: Manual
cd server
source venv/bin/activate
python app.py
```

### 6. Abrir Interfaz Web

Abre tu navegador en:
```
http://localhost:5000
```

## ✅ Verificación

Si todo funciona verás:

**✓ Serial Monitor ESP32:**
```
✓ Cámara inicializada correctamente
✓ Conectado a WiFi
ESP32-CAM listo y operando...
```

**✓ Terminal Servidor:**
```
Servidor Flask para ESP32-CAM Cámara Trampa
Host: 0.0.0.0
Puerto: 5000
Servidor iniciado...
```

**✓ Navegador:**
- Badge verde "Conectado" en la esquina superior derecha
- Dos pestañas: "Captura de Fotos" y "Streaming en Vivo"

## 🎮 Uso Básico

### Capturar Foto
1. Click en pestaña "Captura de Fotos"
2. Click botón "Capturar Foto"
3. Esperar ~2-3 segundos
4. La foto aparece en la galería

### Ver Streaming
1. Click en pestaña "Streaming en Vivo"
2. Click botón "Iniciar Streaming"
3. El video aparece en tiempo real

## 🐛 Problemas Comunes

| Problema | Solución |
|----------|----------|
| ESP32 no conecta a WiFi | Verificar SSID/password, usar WiFi 2.4GHz |
| No aparecen imágenes | Verificar permisos carpeta `server/static/images` |
| Streaming con lag | Reducir resolución en `config.h` |
| Error al cargar código | GPIO0 debe estar en GND durante carga |

## 📁 Archivos Importantes

```
TPI/
├── iniciar_servidor.sh        ← Script de inicio rápido
├── README.md                  ← Documentación completa
├── GUIA_RAPIDA.md            ← Este archivo
├── server/
│   ├── app.py                ← Servidor principal
│   └── config.py             ← Configuración servidor
└── esp32_cam/
    └── src/
        ├── main.cpp          ← Código ESP32
        └── config.h          ← Configuración ESP32 ⚠️
```

⚠️ = Debes editar este archivo

## 📞 Más Ayuda

- **Documentación completa:** `README.md`
- **API:** `docs/API.md`
- **Planificación:** `PLANIFICACION_PROYECTO.md`

## 🔧 Comandos Útiles

```bash
# Ver IP del servidor
hostname -I

# Monitorear ESP32-CAM
cd esp32_cam
pio device monitor

# Detener servidor (en terminal del servidor)
Ctrl + C

# Reiniciar ESP32-CAM desde código
ESP.restart()
```

## ✨ Atajos del Proyecto

```bash
# Activar entorno virtual
source server/venv/bin/activate

# Iniciar servidor
./iniciar_servidor.sh

# Compilar ESP32 (sin subir)
cd esp32_cam && pio run

# Subir código a ESP32
cd esp32_cam && pio run --target upload

# Monitor serial ESP32
cd esp32_cam && pio device monitor
```

---

**¿Todo funcionando?** ¡Perfecto! Ahora puedes capturar imágenes de hipopótamos 🦛📷

