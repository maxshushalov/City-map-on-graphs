from flask import Flask, jsonify, request, send_from_directory
import subprocess
import json
import os

app = Flask(__name__)

# Пути к файлам
PROJECT_ROOT = "/Users/maxshushalov/CPP_projects/Tiger/Laba_3"
API_PATHFINDER = os.path.join(PROJECT_ROOT, "cmake-build-debug", "api_pathfinder")
WEB_DIR = os.path.join(PROJECT_ROOT, "web")
RESPONSE_FILE = os.path.join(WEB_DIR, "response.json")

def call_cpp_api(action, **params):

    args = [API_PATHFINDER, f'action={action}']
    for key, value in params.items():
        args.append(f'{key}={value}')

    try:
        result = subprocess.run(args, capture_output=True, text=True, timeout=10)

        with open(RESPONSE_FILE, 'r', encoding='utf-8') as f:
            data = json.load(f)

        return data

    except subprocess.TimeoutExpired:
        return {"error": "C++ процесс завис (timeout)"}, 500
    except FileNotFoundError:
        return {"error": "response.json не найден (C++ не успел записать?)"}, 500
    except json.JSONDecodeError as e:
        return {"error": f"Невалидный JSON в response.json: {str(e)}"}, 500
    except Exception as e:
        return {"error": f"Ошибка: {str(e)}"}, 500


@app.route('/')
def index():
    return send_from_directory(WEB_DIR, 'index.html')

@app.route('/<path:filename>')
def serve_static(filename):
    return send_from_directory(WEB_DIR, filename)

@app.route('/api/find')
def find_path():
    start = request.args.get('start', '0')
    end = request.args.get('end', '0')
    data = call_cpp_api('find', start=start, end=end)
    return jsonify(data)

@app.route('/api/delivery')
def delivery():
    data = call_cpp_api('delivery')
    return jsonify(data)


@app.route('/api/generate')
def generate():
    hour = request.args.get('hour', '9')

    data = call_cpp_api('generate', hour=hour)
    return jsonify(data)

@app.route('/api/edit_edge')
def edit_edge():
    from_v = request.args.get('from')
    to_v = request.args.get('to')
    mode = request.args.get('mode', 'green')

    if from_v is None or to_v is None:
        return jsonify({"status": "error", "message": "Параметры from/to обязательны"}), 400

    # Используем **{'from': ...} потому что 'from' — зарезервированное слово в Python
    data = call_cpp_api('edit_edge', **{'from': from_v, 'to': to_v, 'mode': mode})
    return jsonify(data)

@app.route('/api/regenerate')
def regenerate():
    num = request.args.get('num', '5')

    data = call_cpp_api('regenerate', num=num)
    return jsonify(data)

@app.route('/api/set_main')
def set_main():
    vertex = request.args.get('vertex', '-1')

    data = call_cpp_api('set_main', vertex=vertex)
    return jsonify(data)

@app.route('/api/add_delivery')
def add_delivery():
    vertex = request.args.get('vertex', '-1')

    data = call_cpp_api('add_delivery', vertex=vertex)
    return jsonify(data)

@app.route('/api/remove_delivery')
def remove_delivery():
    vertex = request.args.get('vertex', '-1')

    data = call_cpp_api('remove_delivery', vertex=vertex)
    return jsonify(data)



if __name__ == '__main__':
    print("Server started: http://localhost:8000")
    app.run(host='0.0.0.0', port=8000, debug=False)