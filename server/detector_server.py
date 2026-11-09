#!/usr/bin/env python3
"""People detection server for the ESP32-CAM people counter.

The ESP32 uploads a JPEG with ``POST /detect`` and receives a plain-text
answer, one line per detected person::

    count=2
    box=0.121,0.304,0.082,0.421,0.913
    box=0.548,0.281,0.101,0.447,0.874

Boxes are normalised ``x,y,width,height,confidence`` in the 0..1 range, so
the firmware can store them and the browser can scale them to any image size.

The confidence threshold is read from the ``X-Threshold`` header (0..1). If
the header is missing, the value passed with ``--conf`` is used.
"""

from __future__ import annotations

import argparse
import io
import json
import logging
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Optional

import torch
from PIL import Image, UnidentifiedImageError
from ultralytics import YOLO

LOGGER = logging.getLogger("people-detector")

MODEL: Optional[YOLO] = None
MODEL_LOCK = threading.Lock()
MAX_BODY_BYTES = 8 * 1024 * 1024
DEFAULT_THRESHOLD = 0.5

# Weights shipped next to this script, so the server never depends on the
# working directory or re-downloads the model.
LOCAL_WEIGHTS = Path(__file__).resolve().parent / "yolo11n.pt"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="0.0.0.0",
                        help="interface to listen on (default: 0.0.0.0)")
    parser.add_argument("--port", type=int, default=8080,
                        help="TCP port (default: 8080)")
    parser.add_argument("--model",
                        default=str(LOCAL_WEIGHTS) if LOCAL_WEIGHTS.is_file()
                        else "yolo11n.pt",
                        help="Ultralytics model (default: yolo11n.pt next to "
                             "this script)")
    parser.add_argument("--imgsz", type=int, default=960,
                        help="inference size (default: 960)")
    parser.add_argument("--conf", type=float, default=DEFAULT_THRESHOLD,
                        help="fallback confidence threshold (default: 0.5)")
    parser.add_argument("--device", default=None,
                        help="torch device, for example cpu or 0 (default: auto)")
    parser.add_argument("--verbose", action="store_true",
                        help="enable debug logging")
    return parser.parse_args()


def load_model(name: str, imgsz: int) -> None:
    global MODEL
    LOGGER.info("Cargando modelo %s (imgsz=%d)...", name, imgsz)
    MODEL = YOLO(name)
    if torch.cuda.is_available():
        LOGGER.info("CUDA disponible: %s", torch.cuda.get_device_name(0))
    else:
        LOGGER.info("Usando CPU")
    LOGGER.info("Modelo listo")


def detect(image: Image.Image, threshold: float, imgsz: int,
           device: Optional[str]) -> list[tuple[float, float, float, float, float]]:
    assert MODEL is not None

    with MODEL_LOCK:
        results = MODEL.predict(
            source=image,
            imgsz=imgsz,
            conf=threshold,
            classes=[0],          # COCO class 0 == person
            device=device,
            verbose=False,
        )

    boxes: list[tuple[float, float, float, float, float]] = []
    if not results:
        return boxes

    for row in results[0].boxes:
        x1, y1, x2, y2 = (float(v) for v in row.xyxyn[0])
        score = float(row.conf[0])
        boxes.append((x1, y1, x2 - x1, y2 - y1, score))

    boxes.sort(key=lambda item: item[4], reverse=True)
    return boxes


class DetectorHandler(BaseHTTPRequestHandler):
    server_version = "PeopleDetector/1.0"
    protocol_version = "HTTP/1.1"

    # Injected by main()
    imgsz = 960
    device: Optional[str] = None
    fallback_threshold = DEFAULT_THRESHOLD

    def log_message(self, fmt: str, *args) -> None:
        LOGGER.info("%s - %s", self.address_string(), fmt % args)

    def _send(self, status: int, body: bytes, content_type: str) -> None:
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Connection", "close")
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self) -> None:  # noqa: N802 (stdlib naming)
        if self.path in ("/", "/health"):
            payload = json.dumps({
                "status": "ok",
                "model_loaded": MODEL is not None,
                "imgsz": self.imgsz,
            }).encode()
            self._send(200, payload, "application/json")
            return
        self._send(404, b'{"error":"not found"}', "application/json")

    def do_POST(self) -> None:  # noqa: N802 (stdlib naming)
        if self.path.split("?", 1)[0] != "/detect":
            self._send(404, b'{"error":"not found"}', "application/json")
            return

        try:
            length = int(self.headers.get("Content-Length", "0"))
        except ValueError:
            length = 0
        if length <= 0:
            self._send(400, b'{"error":"empty body"}', "application/json")
            return
        if length > MAX_BODY_BYTES:
            self._send(413, b'{"error":"body too large"}', "application/json")
            return

        body = self.rfile.read(length)
        if MODEL is None:
            self._send(503, b'{"error":"model not loaded"}', "application/json")
            return

        try:
            threshold = float(self.headers.get("X-Threshold",
                                               self.fallback_threshold))
        except ValueError:
            threshold = self.fallback_threshold
        threshold = min(max(threshold, 0.01), 0.99)

        try:
            image = Image.open(io.BytesIO(body)).convert("RGB")
        except (UnidentifiedImageError, OSError) as exc:
            LOGGER.warning("JPEG inválido: %s", exc)
            self._send(400, b'{"error":"invalid jpeg"}', "application/json")
            return

        try:
            boxes = detect(image, threshold, self.imgsz, self.device)
        except Exception:  # noqa: BLE001 - never kill the server thread
            LOGGER.exception("Fallo durante la inferencia")
            self._send(500, b'{"error":"inference failed"}', "application/json")
            return

        lines = [f"count={len(boxes)}"]
        lines += [
            "box={:.4f},{:.4f},{:.4f},{:.4f},{:.4f}".format(*box)
            for box in boxes
        ]
        payload = ("\n".join(lines) + "\n").encode()

        LOGGER.info("Detección: %d persona(s) sobre %dx%d",
                    len(boxes), image.width, image.height)
        self._send(200, payload, "text/plain; charset=utf-8")


class DetectorServer(ThreadingHTTPServer):
    allow_reuse_address = True
    daemon_threads = True


def main() -> None:
    args = parse_args()
    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(asctime)s %(levelname)s %(name)s: %(message)s",
    )

    load_model(args.model, args.imgsz)

    DetectorHandler.imgsz = args.imgsz
    DetectorHandler.device = args.device
    DetectorHandler.fallback_threshold = args.conf

    server = DetectorServer((args.host, args.port), DetectorHandler)
    LOGGER.info("Escuchando en http://%s:%d/detect", args.host, args.port)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        LOGGER.info("Deteniendo...")
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
