"""
    @Author: Gianluca Galanti
    @Version: 7/05/2024
"""

import requests
from flask import *
import pymysql
from datetime import datetime
import time
from flask_cors import CORS

import JsonManager

app = Flask(__name__)
cors = CORS(app, resources={r"/*": {"origins": "*"}})

jsonManager = JsonManager.JsonManager()

'''
db = SQLAlchemy()
app.config["SQLALCHEMY_DATABASE_URI"] = "sqlite:///database.db"
app.config["SECRET_KEY"] = "DCm06.ak"
db.init_app(app)


# Data class
class Data(db.Model, UserMixin):
    # __tablename__ = "ESP32Data"
    id = db.Column(db.Integer, primary_key=True)
    device_id = db.Column(db.String(100), nullable=False)
    date_time = db.Column(db.String(10), nullable=False)

    # fare lo storage dei dati effettivi usando notazione csv
    temperature = db.Column(db.String(50), nullable=False)
    humidity = db.Column(db.String(50), nullable=False)
    heat_index = db.Column(db.String(50), nullable=False)
    light = db.Column(db.String(50), nullable=False)


with app.app_context():
    db.create_all()
'''

start: [float] = time.time()
count: [int] = 0
actual_datas: [dict]


def db_connection():
    in_conn = pymysql.connect(
        host="sql11.freesqldatabase.com",
        database="sql11703848",
        user="sql11703848",
        password="Q6t1y9zbE5",
        charset="utf8mb4",
        cursorclass=pymysql.cursors.DictCursor
    )
    return in_conn


# POST method
@app.route("/postListener", methods=['POST'])
def postListener():
    if request.method == "POST":
        conn = db_connection()
        cursor = conn.cursor()
        current_data = request.get_json()
        print(current_data)
        # jsonManager.write_data(current_data)

        # data structure:
        # data = {"device_id": "", "date_time": "", "temperature": [], "humidity": [], "heat_index": [], "light": []}
        data = jsonManager.dump_data(current_data)

        temperature = str(data["temperature"][0]) + ";" + str(data["temperature"][1]) + ";" + str(
            data["temperature"][2])
        humidity = str(data["humidity"][0]) + ";" + str(data["humidity"][1]) + ";" + str(data["humidity"][2])
        heat_index = str(data["heat_index"][0]) + ";" + str(data["heat_index"][1]) + ";" + str(data["heat_index"][2])
        light = str(data["light"][0]) + ";" + str(data["light"][1]) + ";" + str(data["light"][2])

        sql_query = """ INSERT INTO data (device_id, date_time, temperature, humidity, heat_index, light) 
                        VALUES (%s, %s, %s, %s, %s, %s)"""

        cursor.execute(sql_query, (data["device_id"], data["date_time"], temperature, humidity, heat_index, light))
        conn.commit()
        cursor.close()
        conn.close()

        """new_data = Data(device_id=data["device_id"], date_time=data["date_time"], temperature=temperature,
                        humidity=humidity, heat_index=heat_index, light=light)

        db.session.add(new_data)
        db.session.commit()

        print(Data.query.all())"""

    return "HTTP/1.1 OK", 200


# GET method
@app.route("/get_datas", methods=['GET'])
def getData():
    if request.method == "GET":
        conn = db_connection()
        cursor = conn.cursor()

        cursor.execute("SELECT * FROM data")

        datas = [
            dict(id=row["id"], device_id=row["device_id"], date_time=row["date_time"], temperature=row["temperature"],
                 humidity=row["humidity"], heat_index=row["heat_index"], light=row["light"])
            for row in cursor.fetchall()
        ]

        for j in range(len(datas)):
            temperaturec = datas[j]["temperature"].split(";")
            humidityc = datas[j]["humidity"].split(";")
            heat_indexc = datas[j]["heat_index"].split(";")
            lightc = datas[j]["light"].split(";")

            for i in range(3):
                temperaturec[i] = float(temperaturec[i])
                humidityc[i] = float(humidityc[i])
                heat_indexc[i] = float(heat_indexc[i])
                lightc[i] = float(lightc[i])

            datas[j]["temperature"] = temperaturec
            datas[j]["humidity"] = humidityc
            datas[j]["heat_index"] = heat_indexc
            datas[j]["light"] = lightc

        cursor.close()
        conn.close()

        if datas is not None:
            try:
                jsonify(datas)
            except:
                print("-------------------------Something went wrong--------------------------")
        return datas


@app.route("/get_one/<int:id>", methods=['GET', 'DELETE'])
def get_one(id):
    if request.method == 'GET':

        conn = db_connection()
        cursor = conn.cursor()

        cursor.execute('SELECT * FROM data WHERE id=%s', str(id))
        rows = cursor.fetchall()

        for r in rows:
            print(r)
            datas = r
        print(datas)

        temperaturec = datas["temperature"].split(";")
        humidityc = datas["humidity"].split(";")
        heat_indexc = datas["heat_index"].split(";")
        lightc = datas["light"].split(";")

        for i in range(3):
            temperaturec[i] = float(temperaturec[i])
            humidityc[i] = float(humidityc[i])
            heat_indexc[i] = float(heat_indexc[i])
            lightc[i] = float(lightc[i])

        datas["temperature"] = temperaturec
        datas["humidity"] = humidityc
        datas["heat_index"] = heat_indexc
        datas["light"] = lightc

        cursor.close()
        conn.close()

        if datas is not None:
            return jsonify(datas), 200
        else:
            return "ID not found or error with db"

    elif request.method == 'DELETE':
        conn = db_connection()
        cursor = conn.cursor()

        cursor.execute('DELETE FROM data WHERE id=%s', str(id))
        conn.commit()

        cursor.close()
        conn.close()

        return "HTTP/1.1 OK", 200


@app.route("/get_actuals", methods=['GET'])
def get_actuals():
    global actual_datas
    global start
    global count

    if request.method == "GET":
        esp32_url = 'http://192.168.1.9/get_actuals'

        if count == 0 or (time.time() - start) > 600.0:
            start = time.time()
            count += 1
            response = requests.get(esp32_url)
            if response.status_code == 200:
                actual_datas = response.json()
                actual_datas["date_time"] = datetime.now().strftime("%d/%m/%Y %H:%M:%S")

        if actual_datas is not None:
            return actual_datas, 200, {"Content-Type": "application/json"}
        else:
            return "Impossible to retrieve actual datas", 400


@app.route("/get_interval", methods=["GET"])
def get_interval(start_date, end_date):
    if request.method == "GET":
        start_date = request.args.get["start_date"]
        end_date = request.args.get["end_date"]

        if start_date is None or end_date is None:
            return "Date passed are not valid", 500


if __name__ == '__main__':
    Flask.run(app, host="0.0.0.0", port=10000, debug=True)
