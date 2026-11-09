# Local people detector for the ESP32-CAM counter
#
#   .\run.ps1                      # 8080, imgsz 960
#   .\run.ps1 --imgsz 640          # faster on CPU
#   .\run.ps1 --device 0           # first NVIDIA GPU
#
# Everything (venv, torch, weights) lives in this folder.

$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

$python = Join-Path $PSScriptRoot '.venv\Scripts\python.exe'
if (-not (Test-Path $python)) {
    Write-Error "Falta .venv. Crearlo con: uv venv .venv --python 3.11; uv pip install --python .venv\Scripts\python.exe -r requirements.txt"
}

& $python .\detector_server.py @args
exit $LASTEXITCODE
