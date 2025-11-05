/**
 * JavaScript para Pestaña de Captura de Fotos
 * Maneja la captura de fotos y la galería de imágenes
 */

// Inicializar Socket.IO
const socket = io();

// Variables globales
let totalImages = 0;
let lastCaptureTime = null;

// ============================================================================
// INICIALIZACIÓN
// ============================================================================

document.addEventListener('DOMContentLoaded', function() {
    console.log('Módulo de Captura inicializado');
    
    // Configurar event listeners
    setupEventListeners();
    
    // Cargar galería inicial
    loadImageGallery();
    
    // Configurar listeners de Socket.IO
    setupSocketListeners();
});

// ============================================================================
// EVENT LISTENERS
// ============================================================================

function setupEventListeners() {
    // Botón de captura
    const captureBtn = document.getElementById('captureBtn');
    if (captureBtn) {
        captureBtn.addEventListener('click', requestCapture);
    }
    
    // Botón de actualizar galería
    const refreshBtn = document.getElementById('refreshGalleryBtn');
    if (refreshBtn) {
        refreshBtn.addEventListener('click', loadImageGallery);
    }
}

// ============================================================================
// SOCKET.IO LISTENERS
// ============================================================================

function setupSocketListeners() {
    // Conexión establecida
    socket.on('connect', function() {
        console.log('Conectado al servidor');
        updateConnectionStatus(true);
    });
    
    // Conexión perdida
    socket.on('disconnect', function() {
        console.log('Desconectado del servidor');
        updateConnectionStatus(false);
    });
    
    // Nueva imagen recibida
    socket.on('new_image', function(data) {
        console.log('Nueva imagen recibida:', data);
        
        // Actualizar estado
        updateCaptureStatus('success', 'Imagen capturada exitosamente!');
        
        // Actualizar galería
        loadImageGallery();
        
        // Mostrar notificación
        showNotification('Nueva imagen capturada', 'success');
        
        // Actualizar última captura
        lastCaptureTime = data.timestamp;
        updateStats();
    });
    
    // Estado de captura
    socket.on('capture_status', function(data) {
        console.log('Estado de captura:', data);
        
        if (data.status === 'requested') {
            updateCaptureStatus('info', 'Esperando respuesta de ESP32-CAM...');
        }
    });
    
    // Estado de conexión
    socket.on('connection_status', function(data) {
        console.log('Estado de conexión:', data);
    });
}

// ============================================================================
// FUNCIONES DE CAPTURA
// ============================================================================

function requestCapture() {
    console.log('Solicitando captura de foto...');
    
    // Actualizar UI
    const captureBtn = document.getElementById('captureBtn');
    captureBtn.disabled = true;
    captureBtn.innerHTML = '<span class="spinner-border spinner-border-sm me-2"></span>Capturando...';
    
    updateCaptureStatus('info', 'Enviando solicitud a ESP32-CAM...');
    
    // Enviar solicitud al servidor vía Socket.IO
    socket.emit('request_capture', {
        timestamp: new Date().toISOString()
    });
    
    // También enviar vía REST API
    fetch('/api/capture', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ trigger: true })
    })
    .then(response => response.json())
    .then(data => {
        console.log('Respuesta del servidor:', data);
        
        // Restaurar botón después de 3 segundos
        setTimeout(() => {
            captureBtn.disabled = false;
            captureBtn.innerHTML = '<i class="bi bi-camera-fill"></i> Capturar Foto';
        }, 3000);
    })
    .catch(error => {
        console.error('Error al solicitar captura:', error);
        updateCaptureStatus('danger', 'Error al enviar solicitud');
        
        // Restaurar botón
        captureBtn.disabled = false;
        captureBtn.innerHTML = '<i class="bi bi-camera-fill"></i> Capturar Foto';
    });
}

// ============================================================================
// FUNCIONES DE GALERÍA
// ============================================================================

function loadImageGallery() {
    console.log('Cargando galería de imágenes...');
    
    const gallery = document.getElementById('imageGallery');
    
    // Mostrar loading
    gallery.innerHTML = `
        <div class="col-12 text-center">
            <div class="spinner-border text-primary" role="status">
                <span class="visually-hidden">Cargando...</span>
            </div>
            <p class="mt-2 text-muted">Cargando imágenes...</p>
        </div>
    `;
    
    // Obtener imágenes del servidor
    fetch('/api/images')
        .then(response => response.json())
        .then(data => {
            console.log('Imágenes obtenidas:', data);
            
            totalImages = data.count;
            updateStats();
            
            if (data.images.length === 0) {
                // No hay imágenes
                gallery.innerHTML = `
                    <div class="col-12 text-center text-muted">
                        <i class="bi bi-image" style="font-size: 3rem;"></i>
                        <p>No hay imágenes capturadas aún</p>
                    </div>
                `;
            } else {
                // Mostrar imágenes
                gallery.innerHTML = '';
                
                data.images.forEach(image => {
                    const imageCard = createImageCard(image);
                    gallery.appendChild(imageCard);
                });
            }
        })
        .catch(error => {
            console.error('Error al cargar imágenes:', error);
            gallery.innerHTML = `
                <div class="col-12 text-center text-danger">
                    <i class="bi bi-exclamation-triangle" style="font-size: 3rem;"></i>
                    <p>Error al cargar las imágenes</p>
                </div>
            `;
        });
}

function createImageCard(image) {
    const col = document.createElement('div');
    col.className = 'col-md-4 col-lg-3';
    
    col.innerHTML = `
        <div class="card image-card shadow-sm">
            <img src="${image.url}" class="card-img-top" alt="${image.filename}" 
                 style="cursor: pointer; height: 200px; object-fit: cover;"
                 onclick="showImageModal('${image.url}', '${image.filename}')">
            <div class="card-body">
                <p class="card-text small text-muted mb-1">
                    <i class="bi bi-calendar"></i> ${image.modified}
                </p>
                <p class="card-text small text-muted">
                    <i class="bi bi-file-earmark"></i> ${formatFileSize(image.size)}
                </p>
                <div class="d-grid gap-2">
                    <a href="${image.url}" download="${image.filename}" 
                       class="btn btn-sm btn-outline-primary">
                        <i class="bi bi-download"></i> Descargar
                    </a>
                </div>
            </div>
        </div>
    `;
    
    return col;
}

function showImageModal(imageUrl, filename) {
    const modal = new bootstrap.Modal(document.getElementById('imageModal'));
    const modalImage = document.getElementById('modalImage');
    const modalLabel = document.getElementById('imageModalLabel');
    const downloadBtn = document.getElementById('downloadImageBtn');
    
    modalImage.src = imageUrl;
    modalLabel.textContent = filename;
    downloadBtn.href = imageUrl;
    downloadBtn.download = filename;
    
    modal.show();
}

// ============================================================================
// FUNCIONES DE UI
// ============================================================================

function updateCaptureStatus(type, message) {
    const statusDiv = document.getElementById('captureStatus');
    
    // Remover clases anteriores
    statusDiv.className = 'alert';
    
    // Agregar nueva clase
    statusDiv.classList.add(`alert-${type}`);
    
    // Iconos según tipo
    const icons = {
        'info': 'info-circle',
        'success': 'check-circle',
        'warning': 'exclamation-triangle',
        'danger': 'x-circle'
    };
    
    const icon = icons[type] || 'info-circle';
    
    statusDiv.innerHTML = `<i class="bi bi-${icon}"></i> ${message}`;
}

function updateConnectionStatus(connected) {
    const statusBadge = document.getElementById('connectionStatus');
    
    if (connected) {
        statusBadge.className = 'badge bg-success me-2';
        statusBadge.innerHTML = '<i class="bi bi-circle-fill"></i> Conectado';
    } else {
        statusBadge.className = 'badge bg-danger me-2';
        statusBadge.innerHTML = '<i class="bi bi-circle-fill"></i> Desconectado';
    }
}

function updateStats() {
    const totalImagesEl = document.getElementById('totalImages');
    const lastCaptureEl = document.getElementById('lastCapture');
    
    totalImagesEl.textContent = totalImages;
    
    if (lastCaptureTime) {
        lastCaptureEl.textContent = lastCaptureTime;
    }
}

function showNotification(message, type = 'info') {
    // Crear elemento de notificación
    const notification = document.createElement('div');
    notification.className = `alert alert-${type} alert-dismissible fade show position-fixed`;
    notification.style.cssText = 'top: 80px; right: 20px; z-index: 9999; min-width: 300px;';
    
    notification.innerHTML = `
        ${message}
        <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
    `;
    
    document.body.appendChild(notification);
    
    // Auto-remover después de 5 segundos
    setTimeout(() => {
        notification.remove();
    }, 5000);
}

// ============================================================================
// UTILIDADES
// ============================================================================

function formatFileSize(bytes) {
    if (bytes === 0) return '0 Bytes';
    
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    
    return Math.round(bytes / Math.pow(k, i) * 100) / 100 + ' ' + sizes[i];
}

