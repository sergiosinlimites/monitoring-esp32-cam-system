/**
 * JavaScript para Pestaña de Streaming en Vivo
 * Maneja el streaming en tiempo real de video desde ESP32-CAM
 */

// Variables globales para streaming
let streamingActive = false;
let streamImg = null;
let frameCount = 0;
let lastFrameTime = 0;
let fps = 0;
let fpsInterval = null;

// IP del ESP32-CAM (se obtiene automáticamente del servidor)
let ESP32_IP = '192.168.0.100'; // IP por defecto, se actualizará automáticamente
const ESP32_STREAM_PORT = 81;

// ============================================================================
// INICIALIZACIÓN
// ============================================================================

document.addEventListener('DOMContentLoaded', function() {
    console.log('Módulo de Streaming inicializado');
    
    // Inicializar imagen de streaming
    initStreamImage();
    
    // Configurar event listeners
    setupStreamingEventListeners();
    
    // Solicitar IP del ESP32 al servidor
    fetchESP32IP();
});

// ============================================================================
// IMAGEN DE STREAMING
// ============================================================================

function initStreamImage() {
    streamImg = document.getElementById('streamCanvas');
    
    if (streamImg) {
        // Convertir canvas a img si es necesario
        if (streamImg.tagName === 'CANVAS') {
            const parent = streamImg.parentNode;
            const newImg = document.createElement('img');
            newImg.id = 'streamCanvas';
            newImg.className = streamImg.className;
            newImg.style.width = '100%';
            newImg.style.height = 'auto';
            newImg.style.display = 'none';
            parent.replaceChild(newImg, streamImg);
            streamImg = newImg;
        }
        
        // Agregar event listener para contar frames
        streamImg.addEventListener('load', function() {
            frameCount++;
        });
        
        // Error handler
        streamImg.addEventListener('error', function() {
            console.error('Error al cargar stream');
            if (streamingActive) {
                updateStreamingStatus('danger', 'Error al conectar con el stream del ESP32');
            }
        });
    }
}

function fetchESP32IP() {
    // Intentar obtener la IP del ESP32 desde el servidor
    fetch('/api/esp32-ip')
        .then(response => response.json())
        .then(data => {
            if (data.ip) {
                ESP32_IP = data.ip;
                console.log('IP del ESP32 obtenida:', ESP32_IP);
            }
        })
        .catch(error => {
            console.log('No se pudo obtener IP del ESP32, usando IP por defecto');
        });
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
// CÁLCULO DE FPS
// ============================================================================

function startFPSCounter() {
    frameCount = 0;
    
    fpsInterval = setInterval(function() {
        fps = frameCount;
        frameCount = 0;
        updateStreamingMetrics();
    }, 1000);
}

function stopFPSCounter() {
    if (fpsInterval) {
        clearInterval(fpsInterval);
        fpsInterval = null;
    }
    fps = 0;
    frameCount = 0;
}

// ============================================================================
// FUNCIONES DE STREAMING
// ============================================================================

function startStreaming() {
    console.log('Iniciando streaming...');
    
    streamingActive = true;
    
    // Actualizar UI
    document.getElementById('startStreamBtn').disabled = true;
    document.getElementById('stopStreamBtn').disabled = false;
    
    updateStreamingStatus('success', `Conectando al streaming del ESP32...`);
    
    // Mostrar imagen y ocultar placeholder
    if (streamImg) {
        streamImg.style.display = 'block';
        document.getElementById('streamPlaceholder').style.display = 'none';
        
        // Construir URL del stream MJPEG
        const streamUrl = `http://${ESP32_IP}:${ESP32_STREAM_PORT}/stream?t=${Date.now()}`;
        console.log('Stream URL:', streamUrl);
        
        // Establecer la fuente del stream
        streamImg.src = streamUrl;
        
        // Iniciar contador de FPS
        startFPSCounter();
        
        updateStreamingStatus('success', `Streaming activo desde ${ESP32_IP}:${ESP32_STREAM_PORT}`);
    } else {
        updateStreamingStatus('danger', 'Error: Elemento de streaming no encontrado');
    }
}

function stopStreaming() {
    console.log('Deteniendo streaming...');
    
    streamingActive = false;
    
    // Actualizar UI
    document.getElementById('startStreamBtn').disabled = false;
    document.getElementById('stopStreamBtn').disabled = true;
    
    updateStreamingStatus('secondary', 'Streaming detenido');
    
    // Ocultar imagen y mostrar placeholder
    if (streamImg) {
        streamImg.style.display = 'none';
        streamImg.src = ''; // Detener el stream
        document.getElementById('streamPlaceholder').style.display = 'block';
    }
    
    // Detener contador de FPS
    stopFPSCounter();
    
    // Reset métricas
    updateStreamingMetrics();
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
    
    // Actualizar latencia (N/A para MJPEG directo)
    const latencyCounter = document.getElementById('latencyCounter');
    if (latencyCounter) {
        latencyCounter.textContent = streamingActive ? 'N/A' : '0';
    }
    
    // Actualizar contador de frames (total acumulado)
    const frameCounter = document.getElementById('frameCounter');
    if (frameCounter) {
        // Incrementar el total cada segundo basado en FPS
        const currentTotal = parseInt(frameCounter.textContent) || 0;
        frameCounter.textContent = currentTotal + fps;
    }
}

// ============================================================================
// MONITOREO
// ============================================================================

// Verificar conectividad con ESP32
setInterval(function() {
    if (streamingActive && fps === 0) {
        updateStreamingStatus('warning', 'No se reciben frames. Verifica que el ESP32 esté encendido y conectado.');
    }
}, 5000);

