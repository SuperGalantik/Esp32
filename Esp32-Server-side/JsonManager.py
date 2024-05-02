# Gianluca Galanti 4C-IN
# Json manager class
import json
from datetime import datetime


class JsonManager(object):
    file_name = "dati.json"
    file = None
    data = {"device_id": "", "date_time": "", "temperature": [], "humidity": [], "heat_index": [], "light": []}

    def __init__(self):
        self.open_file("w")
        self.file.write("[")
        self.close()

    def write_data(self, data):
        self.open_file("r+")

        #self.dump_data(data)

        #json.dump(self.data, self.file)
        #self.close()
        file_data = json.load(self.file)
        file_data["data"].append(self.dump_data(data))
        json.dump(file_data, self.file)
        self.close()
        return

    def dump_data(self, data):
        self.data["device_id"] = data["device_id"]
        self.data["date_time"] = datetime.now().strftime("%d/%m/%Y %H:%M:%S")

        for i in range(3):
            self.data["temperature"].append(data["temperature"][i])
            self.data["humidity"].append(data["humidity"][i])
            self.data["heat_index"].append(data["heat_index"][i])
            self.data["light"].append(data["light"][i])

        return self.data

    def read_data(self):
        self.open_file("r")
        data = json.load(self.file)
        return data

    def open_file(self, mode):
        if self.file is not None:
            self.close()

        try:
            self.file = open(self.file_name, mode)
        except IOError as e:
            print("Error while opening the file.\nClose the file before opening it")
            print(e)

    def remove_data_from_file(self):
        self.open_file("w")

    def close(self):
        self.file.close()
        self.file = None
        return
