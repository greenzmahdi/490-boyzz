from flask import Flask, jsonify, render_template, request
import json 
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

@app.route('/getposition', methods=['POST'])
def getposition():
    global value1
    global value2
    data = json.loads(request.data)

    value1 = data['value1']
    value2 = data['value2']

    #global data
    #data = request.data.decode('utf-8')
    
    return 'Sucessfully Received'

@app.route('/please')
def please():
    temp = jsonify(data=value1)
    temp2 = jsonify(data=value2)
    something = temp.get_json()['data']
    something2 = temp2.get_json()['data']
    return render_template('index.html', something=something, something2=something2)
    #print("hey", data)
    #return jsonify(data=data)
    #return render_template('index.html', data=data)


if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')
