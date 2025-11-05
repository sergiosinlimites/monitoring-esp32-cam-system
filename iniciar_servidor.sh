#!/bin/bash

# Script de inicio rápido para el servidor Flask
# Sistema de Cámara Trampa ESP32-CAM

echo "============================================================"
echo "    Sistema de Cámara Trampa ESP32-CAM"
echo "    Iniciando servidor Flask..."
echo "============================================================"
echo ""

# Navegar al directorio del servidor
cd "$(dirname "$0")/server"

# Verificar si existe el entorno virtual
if [ ! -d "venv" ]; then
    echo "❌ No se encontró el entorno virtual."
    echo ""
    echo "Por favor, ejecuta primero:"
    echo "  cd server"
    echo "  python3 -m venv venv"
    echo "  source venv/bin/activate"
    echo "  pip install -r requirements.txt"
    echo ""
    exit 1
fi

# Activar entorno virtual
echo "✓ Activando entorno virtual..."
source venv/bin/activate

# Verificar si están instaladas las dependencias
if ! python -c "import flask" 2>/dev/null; then
    echo "❌ Dependencias no instaladas."
    echo ""
    echo "Instalando dependencias..."
    pip install -r requirements.txt
fi

# Obtener IP local
echo ""
echo "Obteniendo IP local..."
IP=$(hostname -I | awk '{print $1}')

if [ -z "$IP" ]; then
    IP="localhost"
fi

echo ""
echo "============================================================"
echo "  Servidor iniciado exitosamente"
echo "============================================================"
echo ""
echo "  Acceder desde este equipo:"
echo "    http://localhost:5000"
echo ""
echo "  Acceder desde otros dispositivos en la red:"
echo "    http://$IP:5000"
echo ""
echo "  Recuerda configurar esta IP en el ESP32-CAM:"
echo "    Editar: esp32_cam/src/config.h"
echo "    #define SERVER_IP \"$IP\""
echo ""
echo "============================================================"
echo "  Presiona Ctrl+C para detener el servidor"
echo "============================================================"
echo ""

# Iniciar servidor
python app.py

