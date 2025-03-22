#data from dht11 sensor local hooked to esp32 and sends data to lightning studio

from flask import Flask, request
from datetime import datetime

app = Flask(__name__)

# In-memory store for sensor readings (for demo purposes)
sensor_data = []

@app.route('/', methods=['GET'])
def index():
    # Build an HTML page to display the sensor data in a table
    html = """
    <!doctype html>
    <html lang="en">
      <head>
        <meta charset="utf-8">
        <title>Sensor Data Dashboard</title>
        <style>
          body { font-family: Arial, sans-serif; margin: 40px; }
          table { border-collapse: collapse; width: 80%; }
          th, td { border: 1px solid #ddd; padding: 8px; text-align: center; }
          th { background-color: #f2f2f2; }
          tr:nth-child(even){background-color: #f9f9f9;}
          h1 { color: #333; }
        </style>
      </head>
      <body>
        <h1>Latest Sensor Data</h1>
        <table>
          <tr>
            <th>Timestamp</th>
            <th>Temperature (°C)</th>
            <th>Humidity (%)</th>
            <th>Device ID</th>
          </tr>
    """
    # Display the last 20 readings (or all if fewer)
    for reading in sensor_data[-20:]:
        html += f"<tr><td>{reading.get('timestamp')}</td><td>{reading.get('temperature')}</td><td>{reading.get('humidity')}</td><td>{reading.get('device_id')}</td></tr>"
    html += """
        </table>
      </body>
    </html>
    """
    return html

@app.route('/', methods=['POST'])
def receive_data():
    data = request.get_json()
    if data:
        # Append a timestamp to the data
        data['timestamp'] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        sensor_data.append(data)
        print("Received data:", data)
        return "Data received", 200
    else:
        return "Invalid data", 400

if __name__ == '__main__':
    # Run the server on all interfaces on port 8000
    app.run(host='0.0.0.0', port=8000)
