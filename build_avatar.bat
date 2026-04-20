@echo off
echo Building Avatar Engine...
cd /d "%~dp0"
C:\msys64\ucrt64\bin\g++.exe -fdiagnostics-color=always -g ^
  src\main.cpp ^
  src\mesh.cpp ^
  src\texture_loader.cpp ^
  src\model.cpp ^
  src\shaders\shader.cpp ^
  src\AvatarEngine.cpp ^
  src\bone.cpp ^
  -o "OpenGL Setup.exe" ^
  -Idependencies\include ^
  -Iinclude ^
  -Ldependencies\lib ^
  -lglfw3dll ^
  -lglad ^
  -lassimp ^
  -lz ^
  -lws2_32

if %errorlevel% neq 0 (
  echo Build Failed!
  pause
  exit /b %errorlevel%
)
echo Build Succeeded!
