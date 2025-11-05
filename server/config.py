"""
Configuración del servidor Flask
"""

import os

# Configuración del servidor
HOST = '0.0.0.0'  # Accesible desde cualquier dispositivo en la red local
PORT = 5000

# Carpetas
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
UPLOAD_FOLDER = os.path.join(BASE_DIR, 'static', 'images')
STATIC_FOLDER = os.path.join(BASE_DIR, 'static')
TEMPLATE_FOLDER = os.path.join(BASE_DIR, 'templates')

# Crear carpeta de imágenes si no existe
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

# Tamaño máximo de archivo (16MB)
MAX_CONTENT_LENGTH = 16 * 1024 * 1024

# Formatos de imagen permitidos
ALLOWED_EXTENSIONS = {'jpg', 'jpeg', 'png'}

# Configuración de CORS
CORS_ORIGINS = '*'  # En producción, especificar dominios permitidos

# Configuración de SocketIO
SOCKETIO_ASYNC_MODE = 'eventlet'
SOCKETIO_CORS_ALLOWED_ORIGINS = '*'

# Variables globales para control
CAPTURE_REQUESTED = False
STREAMING_ACTIVE = False

