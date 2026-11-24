
@echo off
setlocal
cd /d "%~dp0"
title Regex-to-DFA Simulator

echo.
echo ================================================
echo       INTERACTIVE REGEX TO DFA SIMULATOR
echo ================================================
echo.

:: Check Python
where python >nul 2>nul
if errorlevel 1 (
    echo [ERROR] Python was not found.
    echo Install Python and add it to PATH.
    pause
    exit /b 1
)

:: Check C++ compiler
where g++ >nul 2>nul
if errorlevel 1 (
    echo [ERROR] g++ compiler was not found.
    echo Install MSYS2 UCRT64 GCC and configure PATH.
    pause
    exit /b 1
)

:: Check Graphviz
where dot >nul 2>nul
if errorlevel 1 (
    echo [WARNING] Graphviz was not found.
    echo DFA diagrams may not render correctly.
    echo.
)

:: Create virtual environment if needed
if not exist ".venv\Scripts\python.exe" (
    echo [1/3] Setting up Python environment...
    python -m venv .venv
    if errorlevel 1 goto :error
)

:: Install packages only if required imports are missing
.venv\Scripts\python.exe -c "import streamlit, pandas, graphviz" >nul 2>nul

if errorlevel 1 (
    echo [2/3] Installing required packages...
    .venv\Scripts\python.exe -m pip install --disable-pip-version-check -q -r requirements.txt
    if errorlevel 1 goto :error
) else (
    echo [2/3] Python packages are ready.
)

:: Launch application
echo [3/3] Starting simulator...
echo.
echo Opening the simulator at http://localhost:8501
echo Press Ctrl+C to stop the application.
echo.

.venv\Scripts\python.exe -m streamlit run python_frontend\app.py

echo.
echo Simulator stopped.
pause
exit /b 0

:error
echo.
echo [ERROR] Setup failed.
echo Check the error above and try again.
pause
exit /b 1
