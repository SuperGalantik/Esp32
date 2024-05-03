import pymysql

conn = pymysql.connect(
    host="sql11.freesqldatabase.com",
    database="sql11703848",
    user="sql11703848",
    password="Q6t1y9zbE5",
    charset="utf8mb4",
    cursorclass=pymysql.cursors.DictCursor
)

cursor = conn.cursor()
sql_query = """ CREATE TABLE data (
        id integer PRIMARY KEY AUTO_INCREMENT,
        device_id text NOT NULL,
        date_time text NOT NULL,
        temperature text NOT NULL,
        humidity text NOT NULL,
        heat_index text NOT NULL,
        light text NOT NULL
    )
"""
cursor.execute(sql_query)
conn.close()
