@echo off
echo ========================================
echo   Demarrage du Moteur Morphologique
echo ========================================
echo.

cd /d "%~dp0"

echo [1/2] Demarrage du serveur backend (port 3001)...
start "Backend Server" cmd /k "node api/src/server.js"
timeout /t 3 /nobreak >nul

echo [2/2] Demarrage du frontend Angular (port 4200)...
cd moteur-morpho-ui
start "Frontend Angular" cmd /k "ng serve"

echo.
echo ========================================
echo   Les deux serveurs sont en cours de demarrage
echo   Backend:  http://localhost:3001
echo   Frontend: http://localhost:4200
echo ========================================
echo.
echo Appuyez sur une touche pour fermer cette fenetre...
pause >nul
