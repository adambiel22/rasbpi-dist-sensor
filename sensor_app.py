from http import HTTPStatus
from flask import Flask, make_response, render_template, Response
from threading import Timer
import fake_sensor as sensor

app = Flask(__name__)
DATA = {
    "dist": 0,
    "measure": False,
    "watch": False,
    "alarm": False,
    "watching_dist": -1,
    "dist_error": 1,
}


def update_data(interval):
    global DATA
    if DATA["measure"]:
        Timer(interval, update_data, [interval]).start()
        print("measuring")
        DATA["dist"] = sensor.distance()
    if DATA["watch"] and abs(DATA["dist"] - DATA["watching_dist"]) > DATA["dist_error"]:
        DATA["alarm"] = True
        print("ALARM")


@app.route("/")
def root():
    return render_template("sensor.html")


@app.route("/data")
def data():
    return DATA


@app.route("/start-measure", methods=["POST"])
def start():
    global DATA
    if not DATA["measure"]:
        DATA["measure"] = True
        update_data(1)
    return DATA


@app.route("/stop-measure", methods=["POST"])
def stop():
    global DATA
    if DATA["measure"]:
        DATA["measure"] = False
    return DATA


@app.route("/start-watch", methods=["POST"])
def watch():
    global DATA
    if not DATA["measure"]:
        return make_response("First start measuring, then start watching", 400)
        # Response(
        #     status=HTTPStatus.BAD_REQUEST,
        #     response="First start measuring, then start watching",
        # )
    else:
        DATA["watch"] = True
        DATA["watching_dist"] = DATA["dist"]
    return DATA


@app.route("/stop-watch", methods=["POST"])
def stop_watch():
    global DATA
    if DATA["watch"]:
        DATA["watch"] = False
        DATA["watching_dist"] = -1
    return DATA


if __name__ == "__main__":
    app.run(host="localhost", port=8888, debug=True)
