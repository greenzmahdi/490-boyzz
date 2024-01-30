''' 
Set up server : 
    python3 -m venv venv
    source venv/bin/activate
    pip3 install flask
'''

from flask import Flask, jsonify, render_template 

app = Flask(__name__)

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/status')
def status():
    # Example endpoint that ESP32 might request to check the server status
    return jsonify({"status": "Server is running"})

@app.route('/led/on')
def led_on():
    # Code to turn on LED
    return jsonify({"status": "LED turned on"})

@app.route('/led/off')
def led_off():
    # Code to turn off LED
    return jsonify({"status": "LED turned off"})


if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)
