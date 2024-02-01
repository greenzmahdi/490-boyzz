from flask import Flask, render_template

app = Flask(__name__)

# This will store the LED state
led_color = "turquoise"  # Start with turquoise

@app.route('/')
def index():
    return render_template('index.html', led_color=led_color)

@app.route('/toggle_led')
def toggle_led():
    global led_color
    led_color = "purple" if led_color == "turquoise" else "turquoise"
    return led_color

@app.route('/status')
def status():
    return led_color

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')
