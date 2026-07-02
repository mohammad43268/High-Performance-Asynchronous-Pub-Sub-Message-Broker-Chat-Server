@echo off
echo ==========================================
echo Starting NexusChat Microservices
echo ==========================================

echo Starting C++ Chat Server...
start "C++ Chat Server (Port 8080)" cmd /c ".\build\Debug\chat_server.exe & pause"

:: Wait a second for the backend to bind to port 8080
timeout /t 2 /nobreak >nul

echo Starting Node Proxy...
cd client
start "Node TCP-to-WS Proxy (Port 8081)" cmd /c "node proxy.js & pause"

echo Starting React Frontend...
cd frontend
start "React Dev Server" cmd /c "npm run dev & pause"

echo ==========================================
echo All services launched!
echo Opening browser...
echo ==========================================
timeout /t 2 >nul
start http://localhost:5173
