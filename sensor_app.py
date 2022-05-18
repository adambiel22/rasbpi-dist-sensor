from flask import Flask, render_template
from threading import Timer
import fake_sensor as sensor

app = Flask(__name__)
DIST = 0
MEASURE = False
DATA = {"dist": 0, "state": "idle", "watching_dist": -1, "dist_error": 1}
IDLE = 'idle'
MEASURE = 'measure'
WATCH = 'watch'
IDLE_WATCH = 'idle-watch'

def update_data(interval):
    global DATA
    if DATA['state'] != IDLE:
        Timer(interval, update_data, [interval]).start()
        print('measuring')
        DATA['dist'] = sensor.distance()
    if DATA['state'] == WATCH and abs(DATA['dist'] - DATA['watching_dist']) > DATA['dist_error']:
        print('ALARM')


@app.route('/')
def root():
    return render_template('sensor.html')

@app.route('/data')
def data():
    return DATA

@app.route('/start', methods=['POST'])
def start():
    global DATA
    if DATA['state'] == IDLE:
        DATA['state'] = MEASURE
        update_data(1)
    return DATA

@app.route('/stop', methods=['POST'])
def stop():
    global DATA
    if DATA['state'] == MEASURE:
        DATA['state'] = IDLE
    elif DATA['state'] == WATCH:
        DATA['state'] = IDLE
    return DATA

@app.route('/watch', methods=['POST'])
def watch():
    global DATA
    if DATA['state'] == MEASURE:
        DATA['state'] = WATCH
        DATA['watching_dist'] = DATA['dist']
    return DATA

@app.route('/stop-watch', methods = ['POST'])
def stop_watch():
    global DATA
    if DATA['state'] == WATCH:
        DATA['state'] = MEASURE
        DATA['watching_dist'] = -1
    return DATA

if __name__=="__main__":
    app.run(host='localhost', port=8888, debug=True)