from flask import *
from flask_login import UserMixin
from flask_sqlalchemy import SQLAlchemy


import JsonManager

app = Flask(__name__)

jsonManager = JsonManager.JsonManager()

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


# POST method
@app.route("/postListener", methods=['POST'])
def postListener():
    if request.method == "POST":
        current_data = request.get_json()
        print(current_data)
        #jsonManager.write_data(current_data)

        # data structure:
        # data = {"device_id": "", "date_time": "", "temperature": [], "humidity": [], "heat_index": [], "light": []}
        data = jsonManager.dump_data(current_data)

        temperature = str(data["temperature"][0]) + ";" + str(data["temperature"][1]) + ";" + str(data["temperature"][2])
        humidity = str(data["humidity"][0]) + ";" + str(data["humidity"][1]) + ";" + str(data["humidity"][2])
        heat_index = str(data["heat_index"][0]) + ";" + str(data["heat_index"][1]) + ";" + str(data["heat_index"][2])
        light = str(data["light"][0]) + ";" + str(data["light"][1]) + ";" + str(data["light"][2])

        new_data = Data(device_id=data["device_id"], date_time=data["date_time"], temperature=temperature,
                        humidity=humidity, heat_index=heat_index, light=light)

        db.session.add(new_data)
        db.session.commit()

        print(Data.query.all())
    return "ok"


# GET method
@app.route("/getData", methods=['GET'])
def getData():
    if request.method == "GET":
        print(Data.query.all())
        data = Data.query.all()
        return jsonify(data)


if __name__ == '__main__':
    Flask.run(app, host="0.0.0.0", port=10000, debug=True)
