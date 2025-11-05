/**
 * JavaScript para Pestaña de Streaming en Vivo
 * Maneja el streaming en tiempo real de video desde ESP32-CAM
 */

// Variables globales para streaming
let streamingActive = false;
let canvas = null;
let ctx = null;
let frameCount = 0;
let lastFrameTime = 0;
let fps = 0;
let latency = 0;

// ============================================================================
// INICIALIZACIÓN
// ============================================================================

document.addEventListener('DOMContentLoaded', function() {
    console.log('Módulo de Streaming inicializado');
    
    // Inicializar canvas
    initCanvas();
    
    // Configurar event listeners
    setupStreamingEventListeners();
    
    // Configurar listeners de Socket.IO para streaming
    setupStreamingSocketListeners();
});

// ============================================================================
// CANVAS
// ============================================================================

function initCanvas() {
    canvas = document.getElementById('streamCanvas');
    
    if (canvas) {
        ctx = canvas.getContext('2d');
        
        // Configurar tamaño inicial del canvas
        canvas.width = 640;
        canvas.height = 480;
        
        // Ocultar canvas inicialmente
        canvas.style.display = 'none';
    }
}

// ============================================================================
// EVENT LISTENERS
// ============================================================================

function setupStreamingEventListeners() {
    // Botón iniciar streaming
    const startBtn = document.getElementById('startStreamBtn');
    if (startBtn) {
        startBtn.addEventListener('click', startStreaming);
    }
    
    // Botón detener streaming
    const stopBtn = document.getElementById('stopStreamBtn');
    if (stopBtn) {
        stopBtn.addEventListener('click', stopStreaming);
    }
}

// ============================================================================
// SOCKET.IO LISTENERS PARA STREAMING
// ============================================================================

function setupStreamingSocketListeners() {
    // Recibir frame de streaming
    socket.on('stream_frame', function(data) {
        if (!streamingActive) return;
        
        // Calcular FPS y latencia
        const now = Date.now();
        
        if (lastFrameTime > 0) {
            const timeDiff = now - lastFrameTime;
            fps = Math.round(1000 / timeDiff);
        }
        
        lastFrameTime = now;
        
        // Calcular latencia aproximada
        if (data.timestamp) {
            const frameTime = new Date(data.timestamp).getTime();
            latency = now - frameTime;
        }
        
        // Incrementar contador de frames
        frameCount++;
        
        // Actualizar métricas en UI
        updateStreamingMetrics();
        
        // Mostrar frame en canvas
        displayFrame(data.image);
    });
    
    // Estado de streaming
    socket.on('streaming_status', function(data) {
        console.log('Estado de streaming:', data);
    });
}

// ============================================================================
// FUNCIONES DE STREAMING
// ============================================================================

function startStreaming() {
    console.log('Iniciando streaming...');
    
    streamingActive = true;
    frameCount = 0;
    lastFrameTime = 0;
    fps = 0;
    latency = 0;
    
    // Actualizar UI
    document.getElementById('startStreamBtn').disabled = true;
    document.getElementById('stopStreamBtn').disabled = false;
    
    updateStreamingStatus('success', 'Streaming activo - Esperando frames...');
    
    // Mostrar canvas y ocultar placeholder
    canvas.style.display = 'block';
    document.getElementById('streamPlaceholder').style.display = 'none';
    
    // Notificar al servidor vía Socket.IO
    socket.emit('start_streaming', {
        timestamp: new Date().toISOString()
    });
    
    // También notificar vía REST API
    fetch('/api/streaming-status', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ active: true })
    })
    .then(response => response.json())
    .then(data => {
        console.log('Streaming iniciado:', data);
    })
    .catch(error => {
        console.error('Error al iniciar streaming:', error);
    });
}

function stopStreaming() {
    console.log('Deteniendo streaming...');
    
    streamingActive = false;
    
    // Actualizar UI
    document.getElementById('startStreamBtn').disabled = false;
    document.getElementById('stopStreamBtn').disabled = true;
    
    updateStreamingStatus('secondary', 'Streaming detenido');
    
    // Ocultar canvas y mostrar placeholder
    canvas.style.display = 'none';
    document.getElementById('streamPlaceholder').style.display = 'block';
    
    // Limpiar canvas
    if (ctx) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
    }
    
    // Reset métricas
    frameCount = 0;
    fps = 0;
    latency = 0;
    updateStreamingMetrics();
    
    // Notificar al servidor vía Socket.IO
    socket.emit('stop_streaming', {
        timestamp: new Date().toISOString()
    });
    
    // También notificar vía REST API
    fetch('/api/streaming-status', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ active: false })
    })
    .then(response => response.json())
    .then(data => {
        console.log('Streaming detenido:', data);
    })
    .catch(error => {
        console.error('Error al detener streaming:', error);
    });
}

function displayFrame(base64Image) {
    if (!ctx || !streamingActive) return;
    
    const img = new Image();
    
    img.onload = function() {
        // Ajustar tamaño del canvas si es necesario
        if (canvas.width !== img.width || canvas.height !== img.height) {
            canvas.width = img.width;
            canvas.height = img.height;
        }
        
        // Dibujar imagen en canvas
        ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
    };
    
    img.onerror = function() {
        console.error('Error al cargar frame');
    };
    
    // Cargar imagen desde base64
    img.src = 'data:image/jpeg;base64,' + base64Image;
}

// ============================================================================
// FUNCIONES DE UI
// ============================================================================

function updateStreamingStatus(type, message) {
    const statusDiv = document.getElementById('streamingStatus');
    
    // Remover clases anteriores
    statusDiv.className = 'alert';
    
    // Agregar nueva clase
    statusDiv.classList.add(`alert-${type}`);
    
    // Iconos según tipo
    const icons = {
        'info': 'info-circle',
        'success': 'check-circle',
        'warning': 'exclamation-triangle',
        'danger': 'x-circle',
        'secondary': 'info-circle'
    };
    
    const icon = icons[type] || 'info-circle';
    
    statusDiv.innerHTML = `<i class="bi bi-${icon}"></i> ${message}`;
}

function updateStreamingMetrics() {
    // Actualizar FPS
    const fpsCounter = document.getElementById('fpsCounter');
    if (fpsCounter) {
        fpsCounter.textContent = fps;
    }
    
    // Actualizar latencia
    const latencyCounter = document.getElementById('latencyCounter');
    if (latencyCounter) {
        latencyCounter.textContent = latency + ' ms';
    }
    
    // Actualizar contador de frames
    const frameCounter = document.getElementById('frameCounter');
    if (frameCounter) {
        frameCounter.textContent = frameCount;
    }
}

// ============================================================================
// MONITOREO
// ============================================================================

// Verificar si el streaming está activo y mostrar advertencia si no hay frames
setInterval(function() {
    if (streamingActive) {
        const now = Date.now();
        const timeSinceLastFrame = now - lastFrameTime;
        
        // Si han pasado más de 5 segundos sin frames
        if (timeSinceLastFrame > 5000 && lastFrameTime > 0) {
            updateStreamingStatus('warning', 'No se están recibiendo frames. Verificar ESP32-CAM.');
        }
    }
}, 2000);

