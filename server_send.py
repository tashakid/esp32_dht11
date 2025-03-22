#The command data is to be sent from lightning studio to esp32

from flask import Flask, jsonify

app = Flask(__name__)

@app.route('/command', methods=['GET'])
def send_command():
    # Return a JSON command with multiple messages
    command_data = {
        "command": "display",
        "messages": [
            "Welcome to IoT!",
            "ESP32 + I2C LCD",
            "Have a nice day!"
        ]
    }
    return jsonify(command_data), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
