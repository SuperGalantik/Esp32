from flask import *
from flask_sqlalchemy import SQLAlchemy
from flask_login import UserMixin

db = SQLAlchemy()
app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///database.db'
app.config['SECRET_KEY'] = 'superSecretKey'
db.init_app(app)


class User(db.Model, UserMixin):
    id = db.Column(db.Integer, primary_key=True)
    email = db.Column(db.String(100), nullable=False, unique=True)
    password = db.Column(db.String(100), nullable=False)
    name = db.Column(db.String(100), nullable=True)
    surname = db.Column(db.String(100), nullable=True)
    birth_date = db.Column(db.String(50), nullable=True)

with app.app_context():
    db.create_all()

#add a row
new_user = User(email=email, password=password, name=name, surname=surname, birth_date=birth_date)
db.session.add(new_user)
db.session.commit()

#make a query
user = User.query.filter_by(email=email).first()
if user:
    return "User already exists"