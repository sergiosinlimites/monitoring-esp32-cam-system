"""
Servidor Flask para Sistema de Cámara Trampa ESP32-CAM
"""

from flask import Flask, render_template, request, jsonify, send_from_directory
from flask_cors import CORS
from flask_socketio import SocketIO, emit
import os
from datetime import datetime
from werkzeug.utils import secure_filename
import base64
import config

# Inicializar Flask
app = Flask(__name__, 
            static_folder=config.STATIC_FOLDER,
            template_folder=config.TEMPLATE_FOLDER)

# Configuración
app.config['MAX_CONTENT_LENGTH'] = config.MAX_CONTENT_LENGTH
app.config['UPLOAD_FOLDER'] = config.UPLOAD_FOLDER

# Habilitar CORS
CORS(app, resources={r"/*": {"origins": config.CORS_ORIGINS}})

# Inicializar SocketIO
socketio = SocketIO(app, 
                   cors_allowed_origins=config.SOCKETIO_CORS_ALLOWED_ORIGINS,
                   async_mode=config.SOCKETIO_ASYNC_MODE)

# Variables globales
capture_requested = False
streaming_clients = set()
esp32_ip = None  # IP del ESP32-CAM (detectada automáticamente)

def allowed_file(filename):
    """Verifica si el archivo tiene una extensión permitida"""
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in config.ALLOWED_EXTENSIONS

# ============================================================================
# RUTAS WEB
# ============================================================================

@app.route('/')
def index():
    """Página principal con interfaz web"""
    return render_template('index.html')

@app.route('/static/images/<filename>')
def serve_image(filename):
    """Servir imágenes capturadas"""
    return send_from_directory(config.UPLOAD_FOLDER, filename)

# ============================================================================
# API REST ENDPOINTS
# ============================================================================

@app.route('/api/capture', methods=['POST', 'GET'])
def api_capture():
    """
    Endpoint para que ESP32 verifique si debe capturar una foto
    POST: Solicitar captura desde frontend
    GET: ESP32 consulta si debe capturar
    """
    global capture_requested
    
    if request.method == 'POST':
        # Frontend solicita captura
        capture_requested = True
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        
        # Notificar a todos los clientes web
        socketio.emit('capture_status', {
            'status': 'requested',
            'message': 'Solicitud de captura enviada a ESP32-CAM',
            'timestamp': timestamp
        })
        
        return jsonify({
            'status': 'capture_requested',
            'timestamp': timestamp
        }), 200
    
    else:  # GET
        # ESP32 consulta si debe capturar
        should_capture = capture_requested
        
        if should_capture:
            capture_requested = False  # Reset flag
            return jsonify({
                'capture': True,
                'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            }), 200
        else:
            return jsonify({
                'capture': False
            }), 200

@app.route('/api/upload', methods=['POST'])
def api_upload():
    """ESP32 envía imagen capturada"""
    global esp32_ip
    
    # Detectar IP del ESP32
    if esp32_ip is None and request.remote_addr:
        esp32_ip = request.remote_addr
        print(f'IP del ESP32-CAM detectada: {esp32_ip}')
    
    if 'image' not in request.files:
        return jsonify({
            'status': 'error',
            'message': 'No se encontró el campo image'
        }), 400
    
    file = request.files['image']
    
    if file.filename == '':
        return jsonify({
            'status': 'error',
            'message': 'Nombre de archivo vacío'
        }), 400
    
    if file and allowed_file(file.filename):
        # Generar nombre de archivo único con timestamp
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        extension = file.filename.rsplit('.', 1)[1].lower()
        filename = f'capture_{timestamp}.{extension}'
        
        # Guardar archivo
        filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
        file.save(filepath)
        
        # Notificar a clientes web que hay nueva imagen
        socketio.emit('new_image', {
            'filename': filename,
            'url': f'/static/images/{filename}',
            'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        })
        
        return jsonify({
            'status': 'success',
            'filename': filename,
            'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        }), 200
    
    return jsonify({
        'status': 'error',
        'message': 'Tipo de archivo no permitido'
    }), 400

@app.route('/api/stream-frame', methods=['POST'])
def api_stream_frame():
    """ESP32 envía frame para streaming"""
    
    if 'image' not in request.files:
        print('[stream-frame] ERROR: No se encontró el campo image')
        return jsonify({
            'status': 'error',
            'message': 'No se encontró el campo image'
        }), 400
    
    file = request.files['image']
    
    if file:
        # Leer imagen y convertir a base64
        image_data = file.read()
        image_base64 = base64.b64encode(image_data).decode('utf-8')
        
        print(f'[stream-frame] Frame recibido: {len(image_data)} bytes, emitiendo a {len(streaming_clients)} clientes')
        
        # Emitir frame a todos los clientes conectados al streaming
        socketio.emit('stream_frame', {
            'image': image_base64,
            'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
        })
        
        return jsonify({'status': 'success'}), 200
    
    return jsonify({
        'status': 'error',
        'message': 'Error al procesar frame'
    }), 400

@app.route('/api/images', methods=['GET'])
def api_images():
    """Lista todas las imágenes capturadas"""
    
    images = []
    
    if os.path.exists(config.UPLOAD_FOLDER):
        files = os.listdir(config.UPLOAD_FOLDER)
        files.sort(reverse=True)  # Más recientes primero
        
        for filename in files:
            if allowed_file(filename):
                filepath = os.path.join(config.UPLOAD_FOLDER, filename)
                stat = os.stat(filepath)
                
                images.append({
                    'filename': filename,
                    'url': f'/static/images/{filename}',
                    'size': stat.st_size,
                    'modified': datetime.fromtimestamp(stat.st_mtime).strftime('%Y-%m-%d %H:%M:%S')
                })
    
    return jsonify({
        'images': images,
        'count': len(images)
    }), 200

@app.route('/api/latest-image', methods=['GET'])
def api_latest_image():
    """Obtiene la última imagen capturada"""
    
    if os.path.exists(config.UPLOAD_FOLDER):
        files = os.listdir(config.UPLOAD_FOLDER)
        files = [f for f in files if allowed_file(f)]
        
        if files:
            files.sort(reverse=True)
            latest = files[0]
            
            return jsonify({
                'filename': latest,
                'url': f'/static/images/{latest}',
                'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            }), 200
    
    return jsonify({
        'status': 'error',
        'message': 'No hay imágenes disponibles'
    }), 404

@app.route('/api/esp32-ip', methods=['GET'])
def api_esp32_ip():
    """Obtiene la IP del ESP32-CAM"""
    global esp32_ip
    
    if esp32_ip:
        return jsonify({
            'ip': esp32_ip,
            'status': 'detected'
        }), 200
    else:
        return jsonify({
            'ip': None,
            'status': 'not_detected',
            'message': 'IP del ESP32 no detectada aún. Espera a que el ESP32 envíe una imagen.'
        }), 404

@app.route('/api/streaming-status', methods=['GET', 'POST'])
def api_streaming_status():
    """
    GET: ESP32 consulta si debe hacer streaming
    POST: Frontend activa/desactiva streaming
    """
    global streaming_clients
    
    if request.method == 'POST':
        data = request.get_json()
        active = data.get('active', False)
        
        print(f'[POST streaming-status] Active: {active}')
        
        socketio.emit('streaming_status', {
            'active': active,
            'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        })
        
        return jsonify({
            'status': 'success',
            'streaming': active
        }), 200
    
    else:  # GET
        # ESP32 consulta si hay clientes de streaming activos
        has_clients = len(streaming_clients) > 0
        print(f'[GET streaming-status] Clientes activos: {len(streaming_clients)}, Respuesta: {has_clients}')
        
        return jsonify({
            'streaming': has_clients
        }), 200

# ============================================================================
# WEBSOCKET EVENTOS
# ============================================================================

@socketio.on('connect')
def handle_connect():
    """Cliente web se conecta"""
    print(f'Cliente conectado: {request.sid}')
    emit('connection_status', {
        'status': 'connected',
        'message': 'Conectado al servidor',
        'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    })

@socketio.on('disconnect')
def handle_disconnect():
    """Cliente web se desconecta"""
    print(f'Cliente desconectado: {request.sid}')
    if request.sid in streaming_clients:
        streaming_clients.remove(request.sid)

@socketio.on('request_capture')
def handle_request_capture(data):
    """Cliente solicita captura de foto"""
    global capture_requested
    capture_requested = True
    
    emit('capture_status', {
        'status': 'requested',
        'message': 'Solicitud de captura enviada',
        'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    }, broadcast=True)

@socketio.on('start_streaming')
def handle_start_streaming():
    """Cliente inicia streaming"""
    streaming_clients.add(request.sid)
    print(f'Streaming iniciado para cliente: {request.sid}')
    print(f'Total clientes streaming: {len(streaming_clients)}')
    
    emit('streaming_status', {
        'active': True,
        'message': 'Streaming iniciado',
        'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    })

@socketio.on('stop_streaming')
def handle_stop_streaming():
    """Cliente detiene streaming"""
    if request.sid in streaming_clients:
        streaming_clients.remove(request.sid)
    
    print(f'Streaming detenido para cliente: {request.sid}')
    print(f'Total clientes streaming: {len(streaming_clients)}')
    
    emit('streaming_status', {
        'active': False,
        'message': 'Streaming detenido',
        'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    })

# ============================================================================
# MAIN
# ============================================================================

if __name__ == '__main__':
    print('=' * 60)
    print('Servidor Flask para ESP32-CAM Cámara Trampa')
    print('=' * 60)
    print(f'Host: {config.HOST}')
    print(f'Puerto: {config.PORT}')
    print(f'Carpeta de imágenes: {config.UPLOAD_FOLDER}')
    print('=' * 60)
    print('Servidor iniciado. Presiona Ctrl+C para detener.')
    print('=' * 60)
    
    socketio.run(app,
                 host=config.HOST,
                 port=config.PORT,
                 debug=True)

