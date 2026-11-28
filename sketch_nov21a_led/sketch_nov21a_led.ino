#include <WiFi.h>
#include <WebServer.h>
#include <TM1637Display.h>

// =========================
// 1. CẤU HÌNH WIFI
// =========================
const char* ap_ssid = "nhom3";
const char* ap_password = "88888888";

const char* sta_ssid = "Tro 97";
const char* sta_password = "12346789";

WebServer server(80);

// =========================
// 2. CẤU HÌNH CHÂN GPIO ESP32 CŨ & TM1637
// (Nếu dùng cho ESP32-C6: đổi LED_PINS thành {4, 5, 6, 7}, CLK 10, DIO 11)
// =========================
const int LED_PINS[4] = {25, 26, 27, 32};
bool ledState[4] = {false, false, false, false};

#define CLK 14
#define DIO 33
TM1637Display display(CLK, DIO);

// =========================
// 3. GIAO DIỆN WEB TÍCH HỢP SẴN
// =========================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>Điều Khiển Đèn ESP32</title>

    <script src="https://cdn.jsdelivr.net/npm/@mediapipe/camera_utils/camera_utils.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/@mediapipe/control_utils/control_utils.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/@mediapipe/drawing_utils/drawing_utils.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/@mediapipe/hands/hands.js"></script>

    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            -webkit-tap-highlight-color: transparent;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background-color: #f1f5f9;
            color: #1e293b;
            padding: 16px;
            display: flex;
            justify-content: center;
        }

        .container {
            width: 100%;
            max-width: 480px;
            display: flex;
            flex-direction: column;
            gap: 14px;
        }

        header {
            text-align: center;
            padding: 4px 0 2px 0;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        header h1 {
            font-size: 20px;
            font-weight: 700;
            color: #0f172a;
            margin-bottom: 2px;
        }

        header p {
            font-size: 13px;
            color: #64748b;
        }

        .dev-credit {
            display: inline-flex;
            align-items: center;
            margin-top: 6px;
            padding: 3px 12px;
            font-size: 11px;
            color: #64748b;
            background: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 20px;
            letter-spacing: 0.3px;
            font-weight: 500;
        }

        .top-host-bar {
            display: flex;
            align-items: center;
            justify-content: flex-end;
            gap: 6px;
            font-size: 11px;
            color: #64748b;
        }

        .top-host-input {
            width: 175px;
            padding: 4px 8px;
            font-size: 11px;
            color: #1e293b;
            background: #ffffff;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            outline: none;
            transition: border-color 0.15s ease;
        }

        .top-host-input:focus {
            border-color: #2563eb;
        }

        .card {
            background: #ffffff;
            border-radius: 12px;
            padding: 14px;
            box-shadow: 0 1px 3px rgba(0, 0, 0, 0.06);
            border: 1px solid #e2e8f0;
        }

        .card-header {
            font-size: 14px;
            font-weight: 600;
            color: #334155;
            margin-bottom: 12px;
            padding-bottom: 6px;
            border-bottom: 1px solid #f1f5f9;
        }

        .led-list {
            display: flex;
            flex-direction: column;
            gap: 8px;
        }

        .led-row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 8px 12px;
            background: #f8fafc;
            border-radius: 8px;
            border: 1px solid #edf2f7;
        }

        .led-info {
            display: flex;
            align-items: center;
            gap: 8px;
        }

        .status-dot {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background-color: #cbd5e1;
            transition: all 0.15s ease;
        }

        .status-dot.active.led-1 {
            background-color: #ef4444;
            box-shadow: 0 0 8px rgba(239, 68, 68, 0.7);
        }
        .status-dot.active.led-2 {
            background-color: #eab308;
            box-shadow: 0 0 8px rgba(234, 179, 8, 0.7);
        }
        .status-dot.active.led-3 {
            background-color: #16a34a;
            box-shadow: 0 0 8px rgba(22, 163, 74, 0.7);
        }
        .status-dot.active.led-4 {
            background-color: #2563eb;
            box-shadow: 0 0 8px rgba(37, 99, 235, 0.7);
        }

        .led-name {
            font-size: 14px;
            font-weight: 600;
            color: #1e293b;
        }

        .led-pin {
            font-size: 12px;
            color: #94a3b8;
        }

        .btn-pair {
            display: flex;
            gap: 6px;
        }

        .btn {
            border: none;
            cursor: pointer;
            font-size: 13px;
            font-weight: 600;
            padding: 7px 14px;
            border-radius: 6px;
            transition: all 0.12s ease;
            user-select: none;
        }

        .btn:active {
            transform: scale(0.97);
        }

        .btn-on {
            background-color: #dcfce7;
            color: #15803d;
            border: 1px solid #86efac;
        }
        .btn-on:hover { background-color: #bbf7d0; }
        .btn-on.is-active {
            background-color: #16a34a;
            color: #ffffff;
            border-color: #15803d;
            box-shadow: 0 0 0 2px rgba(22, 163, 74, 0.25);
        }

        .btn-off {
            background-color: #ffffff;
            color: #dc2626;
            border: 1px solid #fca5a5;
        }
        .btn-off:hover { background-color: #fef2f2; }
        .btn-off.is-active {
            background-color: #dc2626;
            color: #ffffff;
            border-color: #b91c1c;
            box-shadow: 0 0 0 2px rgba(220, 38, 38, 0.2);
        }

        .all-actions {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 8px;
            margin-top: 10px;
            padding-top: 10px;
            border-top: 1px solid #f1f5f9;
        }

        .btn-blink {
            background-color: #2563eb;
            color: #ffffff;
            padding: 9px;
            font-size: 13px;
            text-align: center;
        }
        .btn-blink:hover { background-color: #1d4ed8; }

        .btn-all-off {
            background-color: #475569;
            color: #ffffff;
            padding: 9px;
            font-size: 13px;
            text-align: center;
        }
        .btn-all-off:hover { background-color: #334155; }

        .camera-box {
            position: relative;
            width: 100%;
            height: 270px;
            background: #000000;
            border-radius: 10px;
            overflow: hidden;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        #webcam {
            width: 100%;
            height: 100%;
            object-fit: cover;
            transform: scaleX(-1);
        }

        #output_canvas {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            object-fit: cover;
            transform: scaleX(-1);
            pointer-events: none;
        }

        .camera-badge {
            position: absolute;
            top: 8px;
            left: 8px;
            background: rgba(0, 0, 0, 0.65);
            color: #ffffff;
            font-size: 11px;
            font-weight: 500;
            padding: 4px 8px;
            border-radius: 12px;
            display: flex;
            align-items: center;
            gap: 5px;
        }

        .camera-dot {
            width: 6px;
            height: 6px;
            border-radius: 50%;
            background-color: #22c55e;
        }

        .cam-controls {
            display: flex;
            gap: 8px;
            margin-top: 10px;
        }

        .btn-secondary {
            flex: 1;
            background: #f8fafc;
            color: #475569;
            border: 1px solid #cbd5e1;
            padding: 7px;
            font-size: 12px;
            border-radius: 6px;
            cursor: pointer;
        }
        .btn-secondary:hover { background: #f1f5f9; }

        .status-box {
            margin-top: 10px;
            padding: 10px 12px;
            background: #f8fafc;
            border-radius: 8px;
            border: 1px solid #e2e8f0;
            display: flex;
            flex-direction: column;
            gap: 5px;
        }

        .status-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            font-size: 13px;
        }

        .status-label { color: #64748b; }
        .status-text { font-weight: 600; color: #0f172a; }

        .gesture-pill {
            background-color: #f1f5f9;
            color: #475569;
            padding: 3px 8px;
            border-radius: 6px;
            font-size: 12px;
            font-weight: 600;
        }

        .gesture-pill.detected {
            background-color: #dcfce7;
            color: #15803d;
        }

        .guide-box {
            margin-top: 10px;
            padding: 10px;
            background: #ffffff;
            border-radius: 8px;
            border: 1px dashed #cbd5e1;
            font-size: 12px;
            color: #475569;
        }

        .guide-title {
            font-weight: 600;
            color: #1e293b;
            margin-bottom: 4px;
        }

        .guide-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 4px 8px;
        }

        .alert-error {
            display: none;
            margin-top: 8px;
            padding: 8px 10px;
            background: #fef2f2;
            border: 1px solid #fecaca;
            color: #b91c1c;
            border-radius: 6px;
            font-size: 12px;
        }
    </style>
</head>
<body>

<div class="container">

    <div class="top-host-bar">
        <label for="esp32-host" title="Wi-Fi AP: nhom3 | MK: 88888888">ESP32:</label>
        <input type="text" id="esp32-host" class="top-host-input" value="http://192.168.4.1" placeholder="http://192.168.4.1">
    </div>

    <header>
        <h1>Điều Khiển Đèn ESP32</h1>
        <p>Bật tắt bằng nút bấm hoặc cử chỉ bàn tay</p>
        <div class="dev-credit">Developed by Quach Minh Hieu</div>
    </header>

    <div class="card">
        <div class="card-header">Bảng điều khiển đèn</div>
        
        <div class="led-list">
            <div class="led-row">
                <div class="led-info">
                    <span class="status-dot led-1" id="dot-1"></span>
                    <span class="led-name">LED 1 (Đỏ)</span>
                    <span class="led-pin">(GPIO 25)</span>
                </div>
                <div class="btn-pair">
                    <button class="btn btn-on" id="btn-on-1" onclick="ledOn(1)">Bật</button>
                    <button class="btn btn-off is-active" id="btn-off-1" onclick="ledOff(1)">Tắt</button>
                </div>
            </div>

            <div class="led-row">
                <div class="led-info">
                    <span class="status-dot led-2" id="dot-2"></span>
                    <span class="led-name">LED 2 (Vàng)</span>
                    <span class="led-pin">(GPIO 26)</span>
                </div>
                <div class="btn-pair">
                    <button class="btn btn-on" id="btn-on-2" onclick="ledOn(2)">Bật</button>
                    <button class="btn btn-off is-active" id="btn-off-2" onclick="ledOff(2)">Tắt</button>
                </div>
            </div>

            <div class="led-row">
                <div class="led-info">
                    <span class="status-dot led-3" id="dot-3"></span>
                    <span class="led-name">LED 3 (Xanh lá)</span>
                    <span class="led-pin">(GPIO 27)</span>
                </div>
                <div class="btn-pair">
                    <button class="btn btn-on" id="btn-on-3" onclick="ledOn(3)">Bật</button>
                    <button class="btn btn-off is-active" id="btn-off-3" onclick="ledOff(3)">Tắt</button>
                </div>
            </div>

            <div class="led-row">
                <div class="led-info">
                    <span class="status-dot led-4" id="dot-4"></span>
                    <span class="led-name">LED 4 (Xanh dương)</span>
                    <span class="led-pin">(GPIO 32)</span>
                </div>
                <div class="btn-pair">
                    <button class="btn btn-on" id="btn-on-4" onclick="ledOn(4)">Bật</button>
                    <button class="btn btn-off is-active" id="btn-off-4" onclick="ledOff(4)">Tắt</button>
                </div>
            </div>
        </div>

        <div class="all-actions">
            <button class="btn btn-blink" onclick="allOn()">Bật cả 4 đèn</button>
            <button class="btn btn-all-off" onclick="allOff()">Tắt tất cả đèn</button>
        </div>
    </div>

    <div class="card">
        <div class="card-header">Điều khiển bằng cử chỉ ngón tay</div>

        <div class="camera-box">
            <video id="webcam" autoplay playsinline muted></video>
            <canvas id="output_canvas"></canvas>
            <div class="camera-badge" id="camera-badge">
                <span class="camera-dot" id="camera-dot"></span>
                <span id="camera-badge-text">Camera đang bật</span>
            </div>
        </div>

        <div class="cam-controls">
            <button class="btn-secondary" onclick="switchFacingMode()">Đổi camera trước / sau</button>
            <button class="btn-secondary" onclick="restartCamera()">Mở lại camera</button>
        </div>

        <div id="camera-error" class="alert-error"></div>

        <div class="status-box">
            <div class="status-row">
                <span class="status-label">Cử chỉ nhận diện:</span>
                <span class="gesture-pill" id="gesture-pill">Đang chờ bàn tay...</span>
            </div>
            <div class="status-row">
                <span class="status-label">Trạng thái hệ thống:</span>
                <span class="status-text" id="status">Sẵn sàng</span>
            </div>
        </div>

        <div class="guide-box">
            <div class="guide-title">Quy ước số ngón tay:</div>
            <div class="guide-grid">
                <div>• <b>1 ngón</b>: Bật Đèn 1</div>
                <div>• <b>2 ngón</b>: Bật Đèn 2</div>
                <div>• <b>3 ngón</b>: Bật Đèn 3</div>
                <div>• <b>4 ngón</b>: Bật Đèn 4</div>
                <div>• <b>5 ngón</b>: Bật cả 4 đèn</div>
                <div>• <b>Nắm tay (0 ngón)</b>: Tắt hết</div>
            </div>
        </div>
    </div>

</div>

<script>
    const DEFAULT_ESP32_URL = window.location.protocol.startsWith("http")
        ? window.location.origin
        : "http://192.168.4.1";

    function getEsp32BaseUrl() {
        const input = document.getElementById("esp32-host");
        let raw = (input ? input.value : "").trim();
        if (!raw) raw = DEFAULT_ESP32_URL;
        if (!/^https?:\/\//i.test(raw)) raw = "http://" + raw;
        return raw.replace(/\/+$/, "");
    }

    const ledStates = { 1: false, 2: false, 3: false, 4: false };

    function updateLedIndicators() {
        for (let i = 1; i <= 4; i++) {
            const dot = document.getElementById(`dot-${i}`);
            const btnOn = document.getElementById(`btn-on-${i}`);
            const btnOff = document.getElementById(`btn-off-${i}`);
            const isOn = !!ledStates[i];

            if (dot) dot.classList.toggle("active", isOn);
            if (btnOn) btnOn.classList.toggle("is-active", isOn);
            if (btnOff) btnOff.classList.toggle("is-active", !isOn);
        }
    }

    async function sendEsp32Command(path, { timeoutMs = 1500, externalSignal = null } = {}) {
        const url = `${getEsp32BaseUrl()}${path}`;
        const controller = new AbortController();
        const timer = setTimeout(() => controller.abort(), timeoutMs);
        const onExternalAbort = () => controller.abort();
        if (externalSignal) {
            if (externalSignal.aborted) { clearTimeout(timer); return false; }
            externalSignal.addEventListener("abort", onExternalAbort, { once: true });
        }
        try {
            const response = await fetch(url, { method: "GET", cache: "no-store", signal: controller.signal });
            return (response.status === 200 || response.ok);
        } catch (err) {
            return false;
        } finally {
            clearTimeout(timer);
            if (externalSignal) externalSignal.removeEventListener("abort", onExternalAbort);
        }
    }

    async function allOn() {
        const ok = await sendEsp32Command("/all/on");
        if (ok) {
            for (let i = 1; i <= 4; i++) ledStates[i] = true;
            updateLedIndicators();
            setStatusText("Đã bật tất cả 4 đèn");
        } else {
            setStatusText("Lỗi kết nối ESP32");
        }
    }

    async function ledOn(id) {
        const ok = await sendEsp32Command(`/led${id}/on`);
        if (ok) {
            ledStates[id] = true;
            updateLedIndicators();
            setStatusText(`Đã bật Đèn ${id}`);
        } else {
            setStatusText(`Lỗi kết nối ESP32`);
        }
    }

    async function ledOff(id) {
        const ok = await sendEsp32Command(`/led${id}/off`);
        if (ok) {
            ledStates[id] = false;
            updateLedIndicators();
            setStatusText(`Đã tắt Đèn ${id}`);
        } else {
            setStatusText(`Lỗi kết nối ESP32`);
        }
    }

    async function allOff() {
        const ok = await sendEsp32Command("/all/off");
        if (ok) {
            for (let i = 1; i <= 4; i++) ledStates[i] = false;
            updateLedIndicators();
            setStatusText("Đã tắt tất cả đèn");
        } else {
            setStatusText("Lỗi kết nối ESP32");
        }
    }

    const statusElem = document.getElementById("status");
    const gesturePill = document.getElementById("gesture-pill");
    const cameraErrorBox = document.getElementById("camera-error");

    function setStatusText(msg) {
        if (statusElem) statusElem.innerText = msg;
    }

    function setGesturePill(text, isDetected) {
        if (!gesturePill) return;
        gesturePill.innerText = text;
        gesturePill.classList.toggle("detected", !!isDetected);
    }

    function setCameraBadge(text, dotColor) {
        const textElem = document.getElementById("camera-badge-text");
        const dotElem = document.getElementById("camera-dot");
        if (textElem && text) textElem.innerText = text;
        if (dotElem && dotColor) dotElem.style.backgroundColor = dotColor;
    }

    const video = document.getElementById("webcam");
    const canvas = document.getElementById("output_canvas");
    const ctx = canvas.getContext("2d");

    let currentStream = null;
    let isCameraRunning = false;
    let currentFacingMode = "user";
    let hands = null;
    let isMediaPipeReady = false;
    let isInitializingHands = false;
    let isProcessingFrame = false;

    async function startCamera() {
        cameraErrorBox.style.display = "none";
        setStatusText("Đang mở camera...");
        setCameraBadge("Đang mở camera...", "#f59e0b");

        if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
            cameraErrorBox.style.display = "block";
            cameraErrorBox.innerHTML = "Trình duyệt chặn camera trên HTTP thường. Nút bấm phía trên vẫn chạy bình thường.";
            setStatusText("Sẵn sàng điều khiển bằng nút bấm");
            setCameraBadge("Camera bị chặn (HTTP)", "#ef4444");
            return;
        }

        if (currentStream) {
            currentStream.getTracks().forEach(track => track.stop());
            currentStream = null;
        }

        try {
            const constraints = {
                video: { facingMode: currentFacingMode, width: { ideal: 320 }, height: { ideal: 240 } },
                audio: false
            };

            let stream;
            try {
                stream = await navigator.mediaDevices.getUserMedia(constraints);
            } catch (e) {
                stream = await navigator.mediaDevices.getUserMedia({ video: true, audio: false });
            }

            currentStream = stream;
            video.srcObject = stream;
            video.muted = true;
            video.playsInline = true;
            video.setAttribute("playsinline", "");
            video.setAttribute("webkit-playsinline", "");
            await video.play();

            isCameraRunning = true;
            if (isMediaPipeReady) {
                setCameraBadge("Camera & AI sẵn sàng", "#22c55e");
                setStatusText("Sẵn sàng! Đưa bàn tay vào khung hình.");
            } else {
                setCameraBadge("Camera bật. Đang nạp AI...", "#f59e0b");
                initMediaPipeHands();
            }
            requestAnimationFrame(processVideoFrame);
        } catch (error) {
            cameraErrorBox.style.display = "block";
            cameraErrorBox.innerHTML = "Không thể truy cập camera. Vui lòng cấp quyền camera trong trình duyệt.";
            setStatusText("Lỗi quyền camera");
            setCameraBadge("Lỗi quyền camera", "#ef4444");
        }
    }

    function restartCamera() {
        startCamera();
        if (!isMediaPipeReady) initMediaPipeHands();
    }

    function switchFacingMode() {
        currentFacingMode = (currentFacingMode === "user") ? "environment" : "user";
        startCamera();
    }

    async function initMediaPipeHands() {
        if (isMediaPipeReady && hands) return true;
        if (isInitializingHands) return false;
        if (typeof Hands === "undefined") {
            setCameraBadge("Cần Internet để tải AI", "#f59e0b");
            return false;
        }

        isInitializingHands = true;
        setCameraBadge("Nạp mô hình AI...", "#3b82f6");

        try {
            hands = new Hands({
                locateFile: (file) => `https://cdn.jsdelivr.net/npm/@mediapipe/hands/${file}`
            });
            hands.setOptions({
                maxNumHands: 1,
                modelComplexity: 0,
                minDetectionConfidence: 0.5,
                minTrackingConfidence: 0.5
            });
            hands.onResults(onHandResults);
            await hands.initialize();

            isMediaPipeReady = true;
            isInitializingHands = false;
            setCameraBadge("Camera & AI sẵn sàng", "#22c55e");
            setStatusText("AI cử chỉ sẵn sàng!");
            return true;
        } catch (cdnErr) {
            isInitializingHands = false;
            setCameraBadge("Lỗi tải AI (cần Internet)", "#ef4444");
            setStatusText("Không tải được AI cử chỉ. Vẫn dùng được nút bấm.");
            return false;
        }
    }

    let lastInferenceTime = 0;

    async function processVideoFrame() {
        if (!isCameraRunning) return;
        if (!isMediaPipeReady && !isInitializingHands && typeof Hands !== "undefined") {
            initMediaPipeHands();
        }
        const now = performance.now();
        if (video.readyState >= 2 && !isProcessingFrame && hands && isMediaPipeReady && video.videoWidth > 0 && (now - lastInferenceTime >= 50)) {
            lastInferenceTime = now;
            isProcessingFrame = true;
            try { await hands.send({ image: video }); } catch (err) {}
            isProcessingFrame = false;
        }
        requestAnimationFrame(processVideoFrame);
    }

    function countFingers(landmarks) {
        function dist(p1, p2) { return Math.hypot(p1.x - p2.x, p1.y - p2.y); }
        const wrist = landmarks[0];

        function checkFinger(tipIdx, pipIdx, mcpIdx) {
            const tip = landmarks[tipIdx], pip = landmarks[pipIdx], mcp = landmarks[mcpIdx];
            const dTipWrist = dist(tip, wrist), dPipWrist = dist(pip, wrist);
            const dTipMcp = dist(tip, mcp), dPipMcp = dist(pip, mcp);
            return ((dTipMcp > dPipMcp * 1.1) && (dTipWrist > dPipWrist)) ||
                   ((tip.y < pip.y) && (dTipWrist > dPipWrist * 0.95));
        }

        const isIndexOpen  = checkFinger(8, 6, 5);
        const isMiddleOpen = checkFinger(12, 10, 9);
        const isRingOpen   = checkFinger(16, 14, 13);
        const isPinkyOpen  = checkFinger(20, 18, 17);

        const palmWidth    = dist(landmarks[5], landmarks[17]);
        const dThumbMiddle = dist(landmarks[4], landmarks[9]);
        const dThumbMcp    = dist(landmarks[4], landmarks[2]);
        const dIpMcp       = dist(landmarks[3], landmarks[2]);
        const isThumbOpen  = (dThumbMiddle > palmWidth * 0.72) && (dThumbMcp > dIpMcp * 1.05);

        if (!isIndexOpen && !isMiddleOpen && !isRingOpen && !isPinkyOpen) return 0;
        if (isIndexOpen && !isMiddleOpen && !isRingOpen && !isPinkyOpen) return 1;
        if (isIndexOpen && isMiddleOpen && !isRingOpen && !isPinkyOpen) return 2;
        if (isIndexOpen && isMiddleOpen && isRingOpen && !isPinkyOpen) return 3;
        if (isIndexOpen && isMiddleOpen && isRingOpen && isPinkyOpen) return isThumbOpen ? 5 : 4;
        if (isThumbOpen && isIndexOpen && isMiddleOpen && (isRingOpen || isPinkyOpen)) return 5;

        let count = 0;
        if (isIndexOpen) count++;
        if (isMiddleOpen) count++;
        if (isRingOpen) count++;
        if (isPinkyOpen) count++;
        if (isThumbOpen) count++;
        return count;
    }

    let noHandCount = 0;

    function onHandResults(results) {
        if (!video.videoWidth || !video.videoHeight) return;
        canvas.width = video.videoWidth;
        canvas.height = video.videoHeight;

        ctx.save();
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        if (results.multiHandLandmarks && results.multiHandLandmarks.length > 0) {
            noHandCount = 0;
            const landmarks = results.multiHandLandmarks[0];
            if (typeof drawConnectors === "function" && typeof HAND_CONNECTIONS !== "undefined") {
                drawConnectors(ctx, landmarks, HAND_CONNECTIONS, { color: "#16a34a", lineWidth: 3 });
            }
            if (typeof drawLandmarks === "function") {
                drawLandmarks(ctx, landmarks, { color: "#2563eb", lineWidth: 2 });
            }
            processGestureWithFilter(countFingers(landmarks));
        } else {
            noHandCount++;
            if (noHandCount > 12) {
                gestureHistory.length = 0;
                setGesturePill("Đang chờ bàn tay...", false);
            }
        }
        ctx.restore();
    }

    // Bộ lọc rút gọn để phản hồi cử chỉ siêu nhạy
    const gestureHistory = [];
    const HISTORY_SIZE = 3;
    let activeGesture = -1;
    let gestureAbortController = null;

    function processGestureWithFilter(rawCount) {
        gestureHistory.push(rawCount);
        if (gestureHistory.length > HISTORY_SIZE) gestureHistory.shift();

        const counts = {};
        for (const g of gestureHistory) counts[g] = (counts[g] || 0) + 1;

        let bestGesture = rawCount, maxOccur = 0;
        for (const [g, occur] of Object.entries(counts)) {
            if (occur > maxOccur) { maxOccur = occur; bestGesture = Number(g); }
        }

        const labelMap = {
            0: "0 ngón (Nắm tay)",
            1: "1 ngón (Đèn 1)",
            2: "2 ngón (Đèn 2)",
            3: "3 ngón (Đèn 3)",
            4: "4 ngón (Đèn 4)",
            5: "5 ngón (Bật cả 4 đèn)"
        };
        setGesturePill(labelMap[bestGesture] || `${bestGesture} ngón`, true);

        if (maxOccur >= 2 && bestGesture !== activeGesture) {
            activeGesture = bestGesture;
            executeGestureChange(bestGesture);
        }
    }

    async function executeGestureChange(gesture) {
        if (gestureAbortController) gestureAbortController.abort();
        gestureAbortController = new AbortController();
        const signal = gestureAbortController.signal;

        if (gesture === 0) {
            if (await sendEsp32Command("/all/off", { timeoutMs: 1000, externalSignal: signal })) {
                for (let i = 1; i <= 4; i++) ledStates[i] = false;
                updateLedIndicators();
                setStatusText("Cử chỉ: Nắm tay (Tắt tất cả)");
            }
        } else if (gesture === 5) {
            if (await sendEsp32Command("/all/on", { timeoutMs: 1000, externalSignal: signal })) {
                for (let i = 1; i <= 4; i++) ledStates[i] = true;
                updateLedIndicators();
                setStatusText("Cử chỉ: 5 ngón (Bật cả 4 đèn)");
            }
        } else if (gesture >= 1 && gesture <= 4) {
            if (await sendEsp32Command(`/only/${gesture}`, { timeoutMs: 1000, externalSignal: signal })) {
                for (let i = 1; i <= 4; i++) ledStates[i] = (i === gesture);
                updateLedIndicators();
                setStatusText(`Cử chỉ: ${gesture} ngón (Bật Đèn ${gesture})`);
            }
        }
    }

    window.addEventListener("DOMContentLoaded", () => {
        const hostInput = document.getElementById("esp32-host");
        if (hostInput) {
            const savedHost = localStorage.getItem("esp32_host_url");
            hostInput.value = savedHost || DEFAULT_ESP32_URL;
            hostInput.addEventListener("change", () => {
                localStorage.setItem("esp32_host_url", getEsp32BaseUrl());
                hostInput.value = getEsp32BaseUrl();
            });
        }
        updateLedIndicators();
        initMediaPipeHands();
        startCamera();
    });
</script>

</body>
</html>
)rawliteral";

// =========================
// 4. ĐIỀU KHIỂN LED & MÀN HÌNH 7 ĐOẠN
// =========================
void updateDisplay() {
  uint8_t data[4];
  for (int i = 0; i < 4; i++) {
    data[i] = ledState[i] ? display.encodeDigit(i + 1) : SEG_G;
  }
  display.setSegments(data);
}

void setLed(int index, bool state) {
  if (index < 0 || index > 3) return;
  digitalWrite(LED_PINS[index], state ? HIGH : LOW);
  ledState[index] = state;
  updateDisplay();
}

void setOnlyLed(int index) {
  for (int i = 0; i < 4; i++) {
    bool st = (i == index);
    digitalWrite(LED_PINS[i], st ? HIGH : LOW);
    ledState[i] = st;
  }
  updateDisplay();
}

void setAllLeds(bool state) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_PINS[i], state ? HIGH : LOW);
    ledState[i] = state;
  }
  updateDisplay();
}

// =========================
// 5. ROUTE WEB SERVER & CORS
// =========================
void allowCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Private-Network", "true");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Connection", "close");
}

void handleRoot() {
  server.client().setNoDelay(true);
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleLedWeb(int index, bool state) {
  setLed(index, state);
  allowCORS();
  server.client().setNoDelay(true);
  server.send(200, "text/plain", "LED " + String(index + 1) + (state ? " ON" : " OFF"));
}

void handleOnlyLedWeb(int index) {
  setOnlyLed(index);
  allowCORS();
  server.client().setNoDelay(true);
  server.send(200, "text/plain", "ONLY LED " + String(index + 1));
}

void handleAllWeb(bool state) {
  setAllLeds(state);
  allowCORS();
  server.client().setNoDelay(true);
  server.send(200, "text/plain", state ? "ALL LED ON" : "ALL LED OFF");
}

// =========================
// 6. SETUP & LOOP (TỐI ƯU TỐC ĐỘ TỐI ĐA)
// =========================
void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }

  display.setBrightness(5);
  updateDisplay();

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false); // Tắt ngủ đông Wi-Fi để nhận lệnh tức thì

  WiFi.begin(sta_ssid, sta_password);
  Serial.print("Dang ket noi WiFi ");
  Serial.print(sta_ssid);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 6000) {
    delay(250);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[STA] IP mang nha: http://" + WiFi.localIP().toString());
  } else {
    Serial.println("\n[STA] Khong thay WiFi nha -> Khoa che do AP doc lap nhom3 (tranh quet kenh gay lag).");
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(false);
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false);
  }

  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("[AP] WiFi: nhom3 | Mo web tai: http://" + WiFi.softAPIP().toString());

  server.on("/", handleRoot);
  server.on("/led1/on",  []() { handleLedWeb(0, true); });
  server.on("/led1/off", []() { handleLedWeb(0, false); });
  server.on("/led2/on",  []() { handleLedWeb(1, true); });
  server.on("/led2/off", []() { handleLedWeb(1, false); });
  server.on("/led3/on",  []() { handleLedWeb(2, true); });
  server.on("/led3/off", []() { handleLedWeb(2, false); });
  server.on("/led4/on",  []() { handleLedWeb(3, true); });
  server.on("/led4/off", []() { handleLedWeb(3, false); });

  // Lệnh gộp cho cử chỉ tay (1 request thay vì 2-4 request)
  server.on("/only/1", []() { handleOnlyLedWeb(0); });
  server.on("/only/2", []() { handleOnlyLedWeb(1); });
  server.on("/only/3", []() { handleOnlyLedWeb(2); });
  server.on("/only/4", []() { handleOnlyLedWeb(3); });

  server.on("/all/on",  []() { handleAllWeb(true); });
  server.on("/all/off", []() { handleAllWeb(false); });
  server.on("/blink",   []() { allowCORS(); server.client().setNoDelay(true); server.send(200, "text/plain", "OK"); });

  server.onNotFound([]() {
    allowCORS();
    server.client().setNoDelay(true);
    if (server.method() == HTTP_OPTIONS) {
      server.send(204);
    } else {
      server.send(404, "text/plain", "Not found");
    }
  });

  server.begin();
  Serial.println("Web Server san sang (Che do toc do cao)!");
}

void loop() {
  server.handleClient();
}