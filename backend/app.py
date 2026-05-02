from flask import Flask, jsonify, request, send_from_directory
from flask_cors import CORS
import subprocess
import os
import threading
import time

app = Flask(__name__, static_folder=os.path.abspath(os.path.join(os.path.dirname(__file__), '../frontend/build')))
CORS(app)

# Simple in-memory order book for demo purposes
# This is not intended to replace the C++ matching engine; it's a minimal implementation
order_book = {
    'bids': [],  # list of {price, qty}
    'asks': []   # list of {price, qty}
}
trades = []  # list of executed trades

ORDER_MATCHING_ENGINE_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), '../OrderMatchingEngine'))


def match_order(side, price, qty):
    """Very small matching routine: match against opposite side by price/time priority."""
    remaining = qty
    executed = []
    if side == 'buy':
        # match with lowest asks
        order_book['asks'].sort(key=lambda x: x['price'])
        i = 0
        while i < len(order_book['asks']) and remaining > 0:
            ask = order_book['asks'][i]
            if ask['price'] <= price:
                take = min(ask['qty'], remaining)
                executed.append({'price': ask['price'], 'qty': take, 'side': 'buy'})
                ask['qty'] -= take
                remaining -= take
                if ask['qty'] == 0:
                    order_book['asks'].pop(i)
                    continue
            i += 1
    else:
        # sell: match with highest bids
        order_book['bids'].sort(key=lambda x: -x['price'])
        i = 0
        while i < len(order_book['bids']) and remaining > 0:
            bid = order_book['bids'][i]
            if bid['price'] >= price:
                take = min(bid['qty'], remaining)
                executed.append({'price': bid['price'], 'qty': take, 'side': 'sell'})
                bid['qty'] -= take
                remaining -= take
                if bid['qty'] == 0:
                    order_book['bids'].pop(i)
                    continue
            i += 1

    return executed, remaining


@app.route('/api/run_demo', methods=['GET'])
def run_demo():
    """Attempt to run OrderMatchingEngine/demo if it's an executable; otherwise return helpful message."""
    demo_path = os.path.join(ORDER_MATCHING_ENGINE_PATH, 'demo')
    if not os.path.exists(demo_path):
        return jsonify({'error': 'Demo executable not found', 'path': demo_path}), 404
    try:
        # run in subprocess and capture output
        result = subprocess.run([demo_path], capture_output=True, text=True, timeout=10)
        return jsonify({'output': result.stdout, 'error': result.stderr, 'returncode': result.returncode})
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/order_book', methods=['GET'])
def get_order_book():
    return jsonify({'order_book': order_book})


@app.route('/api/submit_order', methods=['POST'])
def submit_order():
    data = request.json or {}
    side = data.get('side')
    price = float(data.get('price', 0))
    qty = int(data.get('qty', 0))
    if side not in ('buy', 'sell') or price <= 0 or qty <= 0:
        return jsonify({'error': 'Invalid order data', 'data': data}), 400

    executed, remaining = match_order(side, price, qty)
    # record trades
    for e in executed:
        trades.append({'price': e['price'], 'qty': e['qty'], 'side': side, 'time': time.time()})

    # if remaining > 0, add to book
    if remaining > 0:
        entry = {'price': price, 'qty': remaining}
        if side == 'buy':
            order_book['bids'].append(entry)
        else:
            order_book['asks'].append(entry)

    return jsonify({'status': 'ok', 'executed': executed, 'remaining': remaining})


@app.route('/api/trades', methods=['GET'])
def get_trades():
    return jsonify({'trades': trades})


@app.route('/', defaults={'path': ''})
@app.route('/<path:path>')
def serve_frontend(path):
    # Serve production build if exists
    if app.static_folder and os.path.exists(os.path.join(app.static_folder, 'index.html')):
        if path != '' and os.path.exists(os.path.join(app.static_folder, path)):
            return send_from_directory(app.static_folder, path)
        return send_from_directory(app.static_folder, 'index.html')
    return jsonify({'message': 'Frontend not built. Run frontend dev server (npm start) or build frontend.'})


if __name__ == '__main__':
    # Use threaded=True so demo subprocess calls and Flask both remain responsive during development
    app.run(debug=True, threaded=True)
