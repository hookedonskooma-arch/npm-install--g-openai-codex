#!/usr/bin/env python3
"""
MELEGI MusicGen Server — local AI music provider for the MELEGI PWA.

Loads MusicGen once and serves http://localhost:7863/generate.
The PWA calls it when you select LOCAL in the BUILD panel.

Setup:
  pip install audiocraft scipy
  # audiocraft pulls torch — GPU optional, CPU works (slower)

Run:
  python3 musicgen_server.py
  python3 musicgen_server.py --model facebook/musicgen-medium
  python3 musicgen_server.py --model /path/to/local/checkpoint --port 7863

Models (HuggingFace, auto-downloaded on first run):
  facebook/musicgen-small   ~300 MB  fast, good for testing
  facebook/musicgen-medium  ~1.5 GB  better quality
  facebook/musicgen-large   ~3.3 GB  best quality, needs ~8 GB RAM
  facebook/musicgen-melody  ~1.5 GB  supports melody conditioning
"""
import argparse, io, json, struct, sys
from http.server import BaseHTTPRequestHandler, HTTPServer

parser = argparse.ArgumentParser(description='MELEGI MusicGen local server')
parser.add_argument('--model',  default='facebook/musicgen-small',
                    help='HuggingFace model ID or local checkpoint path')
parser.add_argument('--port',   type=int, default=7863)
parser.add_argument('--device', default='',
                    help='Force device: cpu / cuda / mps (auto-detected if empty)')
args = parser.parse_args()

print(f'[musicgen] Loading {args.model} …', flush=True)
try:
    import torch
    from audiocraft.models import MusicGen
except ImportError:
    print('ERROR: audiocraft not installed.\n  pip install audiocraft', file=sys.stderr)
    sys.exit(1)

device = args.device or ('cuda' if torch.cuda.is_available()
                          else 'mps' if torch.backends.mps.is_available()
                          else 'cpu')
model = MusicGen.get_pretrained(args.model)
model.to(device)
SR = model.sample_rate
print(f'[musicgen] Ready — {args.model} on {device} | sample rate {SR} Hz', flush=True)
print(f'[musicgen] Listening on http://localhost:{args.port}/generate', flush=True)


def _encode_wav(tensor, sr: int) -> bytes:
    """Convert MusicGen output tensor → 16-bit mono WAV bytes."""
    # tensor shape: (batch, channels, time) or (channels, time)
    if tensor.dim() == 3:
        tensor = tensor.squeeze(0)
    samples = tensor.mean(0).cpu().float().numpy()          # mono mix
    peak = float(abs(samples).max())
    if peak > 0:
        samples = samples / peak                              # normalize before clip
    samples = (samples * 32767.0).clip(-32768, 32767).astype('int16')
    n = len(samples)
    data_size = n * 2
    hdr = struct.pack('<4sI4s4sIHHIIHH4sI',
        b'RIFF', 36 + data_size, b'WAVE', b'fmt ', 16,
        1, 1, sr, sr * 2, 2, 16, b'data', data_size)
    return hdr + samples.tobytes()


class _Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        pass  # suppress per-request logging; we print our own

    def _cors(self):
        self.send_header('Access-Control-Allow-Origin',  '*')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.send_header('Access-Control-Allow-Methods', 'POST, OPTIONS')

    def do_OPTIONS(self):
        self.send_response(200)
        self._cors()
        self.end_headers()

    def do_GET(self):
        if self.path == '/ping':
            body = b'MELEGI-MUSICGEN-OK'
            self.send_response(200)
            self._cors()
            self.send_header('Content-Type', 'text/plain')
            self.send_header('Content-Length', str(len(body)))
            self.end_headers()
            self.wfile.write(body)
        else:
            self.send_error(404)

    def do_POST(self):
        if self.path != '/generate':
            self.send_error(404)
            return
        try:
            length = int(self.headers.get('Content-Length', 0))
            body   = json.loads(self.rfile.read(length))
        except Exception as e:
            self.send_error(400, f'Bad JSON: {e}')
            return

        prompt   = str(body.get('prompt', '')).strip()
        duration = float(body.get('duration', 15))
        duration = max(1.0, min(duration, 190.0))

        if not prompt:
            self.send_error(400, 'Missing prompt')
            return

        print(f'[musicgen] {duration:.0f}s — {prompt[:100]}', flush=True)
        try:
            model.set_generation_params(duration=duration)
            import torch as _torch
            with _torch.no_grad():
                wav = model.generate([prompt])
            data = _encode_wav(wav, SR)
        except Exception as e:
            print(f'[musicgen] ERROR: {e}', flush=True)
            self.send_error(500, str(e))
            return

        print(f'[musicgen] Done — {len(data) // 1024} KB WAV', flush=True)
        self.send_response(200)
        self._cors()
        self.send_header('Content-Type',   'audio/wav')
        self.send_header('Content-Length', str(len(data)))
        self.end_headers()
        self.wfile.write(data)


print(f'[musicgen] Ctrl-C to stop', flush=True)
try:
    HTTPServer(('127.0.0.1', args.port), _Handler).serve_forever()
except KeyboardInterrupt:
    print('\n[musicgen] Stopped.')
