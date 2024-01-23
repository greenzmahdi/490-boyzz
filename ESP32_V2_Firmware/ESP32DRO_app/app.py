# from flask import Flask, render_template
# from flask_socketio import SocketIO

# app = Flask(__name__)

# @app.route('/')
# def home():
#     return render_template('index.html')

# if __name__ == '__main__':
#     app.run(debug=True)


from flask import Flask, render_template

app = Flask(__name__)

@app.route('/')
def home():
    return render_template('index.html')

# This variable represents the LED state in your Flask app
led_state = "off"

@app.route('/toggle_led')
def toggle_led():
    global led_state
    led_state = "on" if led_state == "off" else "off"
    return led_state

@app.route('/get_led_state')
def get_led_state():
    return led_state

@app.route('/test')
def test_connection():
    return 'Connection successful!'

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)

