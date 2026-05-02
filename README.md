# Low-Latency Order Matching — Local Run Instructions

This workspace contains a simple React frontend, a Flask backend, and a C++ order matching engine (in `OrderMatchingEngine`). The backend provides minimal demo endpoints and a small in-memory order book for quick development.

Quick start (development)

1. Backend (Flask)

```bash
# from project root
cd backend
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
python app.py
```

The backend runs on http://localhost:5000 by default.

2. Frontend (React dev server)

```bash
cd frontend
npm install
npm start
```

The React dev server runs on http://localhost:3000 and proxies `/api` calls to the Flask backend (see `frontend/package.json`).

Production build served by Flask

1. Build the frontend:

```bash
cd frontend
npm run build
```

2. Start Flask (it will serve `frontend/build` if present):

```bash
cd backend
source .venv/bin/activate
python app.py
```

Notes
- The Flask backend contains a tiny in-memory order book used for demo purposes. The C++ engine in `OrderMatchingEngine` is separate; you can build and run it according to its `Makefile`/`CMakeLists.txt` but the Flask app doesn't yet invoke the C++ engine directly except via `/api/run_demo` if the `demo` executable exists.
- This is intended as a development starting point.
