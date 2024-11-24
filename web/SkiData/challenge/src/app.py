import random
import json
import os
import re

from flask import (Flask, flash, redirect, render_template, request, session,
                   url_for)
from flask_login import (LoginManager, UserMixin, current_user, login_required,
                         login_user, logout_user)
from flask_sqlalchemy import SQLAlchemy
from pycel import ExcelCompiler
from redis import Redis
from rq import Queue
from werkzeug.security import check_password_hash, generate_password_hash
from werkzeug.utils import secure_filename

from bot import visit

app = Flask(__name__)
app.config['SECRET_KEY'] = os.urandom(32).hex()
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///ski_race.db'
app.config['UPLOAD_FOLDER'] = 'uploads'

# Initialize Database
db = SQLAlchemy(app)

# Initialize Flask-Login
login_manager = LoginManager()
login_manager.init_app(app)
login_manager.login_view = 'login'


@app.after_request
def add_security_headers(response):
    response.headers[
        'Content-Security-Policy'] = "script-src 'self' 'unsafe-inline' https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/; frame-ancestors 'none'; object-src 'none';"
    response.headers['X-Content-Type-Options'] = 'nosniff'
    return response

redis_host = os.environ.get('REDIS_HOST', 'localhost')

q = Queue(connection=Redis.from_url(f'redis://{redis_host}:6379'))


def style(rank):
    return {f"rank-{rank}": "1", "src": f"/static/rank-{rank}.png", "width": "25px", "height": "25px"}


app.jinja_env.globals.update(style=style)


# Models
class User(UserMixin, db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    password = db.Column(db.String(200), nullable=False)
    races = db.relationship('Race', backref='user', lazy=True)
    is_admin = db.Column(db.Boolean, default=False)


class Race(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    race_name = db.Column(db.String(50), nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)
    comment = db.Column(db.String(200))
    results = db.relationship('RaceResult', backref='race', lazy=True)


class RaceResult(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    race_id = db.Column(db.Integer, db.ForeignKey('race.id'), nullable=False)
    name = db.Column(db.String(50), nullable=False)
    time = db.Column(db.String(10), nullable=False)
    rank = db.Column(db.String, nullable=False)
    country = db.Column(db.String(3), nullable=False)


if not os.path.exists(app.config['UPLOAD_FOLDER']):
    os.makedirs(app.config['UPLOAD_FOLDER'])

with app.app_context():
    db.create_all()

with open("adj.txt", "r") as f:
    adj = f.read().splitlines() 

with open("noun.txt", "r") as f:
    noun = f.read().splitlines() 


@login_manager.user_loader
def load_user(user_id):
    return User.query.get(int(user_id))


# Routes
@app.route('/')
def index():
    if current_user.is_authenticated:
        return redirect(url_for('my_races'))
    return render_template('index.html')


@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        username = f"{random.choice(adj)}{random.choice(noun)}{random.randrange(10, 99, 1)}"

        password_pattern = "^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?\W).{8,}$"
        if not re.match(password_pattern, request.form['password']):
            flash('Password must be at least 8 characters long, contain at least one uppercase letter, one lowercase letter, one number, and one special character.')
            return redirect(url_for('register'))

        password = generate_password_hash(request.form['password'], method='scrypt')

        while User.query.filter_by(username=username).first():
            username = f"{random.choice(adj)}{random.choice(noun)}{random.randrange(10, 99, 1)}"

        new_user = User()
        new_user.username = username
        new_user.password = password
        db.session.add(new_user)
        db.session.commit()
        flash(f'Account created successfully! Please log in as: {username}')
        return redirect(url_for('login'))
    return render_template('register.html')


@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        user = User.query.filter_by(username=username).first()

        if user and check_password_hash(user.password, password):
            login_user(user)
            flash('Login successful!')
            return redirect(url_for('my_races'))
        else:
            flash('Invalid username or password')
    return render_template('login.html')


@app.route('/logout')
@login_required
def logout():
    logout_user()  # Log out the user
    flash('You have been logged out.')
    return redirect(url_for('index'))


def allowed_file(filename):
    if filename is None:
        return False
    return ('.' in filename) and (filename.rsplit('.', 1)[1].lower() == "xlsx")


def allowed_mime(content_type):
    if content_type is None:
        return False
    return content_type == ("application/vnd.openxmlformats-officedocument." \
                            "spreadsheetml.sheet")


@app.route('/my_races', methods=['GET', 'POST'])
@login_required
def my_races():
    user = current_user
    if request.method == 'POST':
        if 'file' not in request.files:
            flash('No file part')
            return redirect(request.url)

        file = request.files['file']
        if file.filename == '':
            flash('No selected file')
            return redirect(request.url)

        if file:
            filename = secure_filename(file.filename)

            if not allowed_file(filename):
                flash("The file extension is not allowed.")
                return redirect(request.url)

            if not allowed_mime(file.content_type):
                flash(f"The file mimetype {file.content_type} is not allowed.")
                return redirect(request.url)

            filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            file.save(filepath)

            try:
                excel = ExcelCompiler(filepath)
                race_results = []

                for row in range(2, 12):
                    try:
                        if excel.evaluate(f'Sheet1!A{row}') is None:
                            flash(f"Sheet1!A{row}, Name cannot be empty")
                            return redirect(request.url)

                        if excel.evaluate(f'Sheet1!B{row}') is None:
                            flash(f"Sheet1!B{row}, Time missing")
                            return redirect(request.url)

                        if excel.evaluate(f'Sheet1!C{row}') is None:
                            flash(f"Sheet1!C{row}, Rank missing")
                            return redirect(request.url)

                        if excel.evaluate(f'Sheet1!D{row}') is None:
                            flash(f"Sheet1!D{row}, Country missing")
                            return redirect(request.url)

                        if type(excel.evaluate(f'Sheet1!C{row}')) is not int:
                            flash(f"Sheet1!C{row}, Rank must be an integer")
                            return redirect(request.url)

                        excel.evaluate(f'Sheet1!E{row}')
                        excel.set_value(f'Sheet1!E{row}', "Imported")

                        name = excel.evaluate(f'Sheet1!A{row}')
                        time = excel.evaluate(f'Sheet1!B{row}')
                        rank = excel.evaluate(f'Sheet1!C{row}')
                        country = excel.evaluate(f'Sheet1!D{row}')

                        race_results.append({
                            'name': name,
                            'time': time,
                            'rank': rank,
                            'country': country
                        })
                    except Exception as e:
                        flash(f"Error processing row {row}: {str(e)}")
                        break

                race_name = request.form.get('race_name', 'Unnamed Race')

                new_race = Race(race_name=race_name, user_id=user.id, comment=request.form.get('comment', ''))
                db.session.add(new_race)
                db.session.commit()

                for result in race_results:
                    race_result = RaceResult(
                        race_id=new_race.id,
                        name=result['name'],
                        time=result['time'],
                        rank=result['rank'],
                        country=result['country']
                    )
                    db.session.add(race_result)

                db.session.commit()
                flash('Race data uploaded and processed successfully!')
            except Exception as e:
                flash('Error processing the file: ' + str(e))
                return redirect(request.url)

    races = Race.query.filter_by(user_id=user.id).all()
    return render_template('my_races.html', races=races)


@app.route('/race/<int:race_id>')
@login_required
def race_detail(race_id):
    user = current_user
    race = Race.query.get_or_404(race_id)

    if race.user_id == user.id or user.is_admin:
        return render_template('race_detail.html', race=race)

    flash('Not your race!')
    return redirect(url_for("my_races"))


@app.route('/race/<int:race_id>/report')
@login_required
def race_report(race_id):
    user = current_user
    race = Race.query.get_or_404(race_id)

    if race.user_id == user.id or user.is_admin:

        q.enqueue(visit, url_for('race_detail', race_id=race_id),
                  os.environ.get("ADMIN_USER", "FAKEUSER"),
                  os.environ.get("ADMIN_PASSWORD", "FAKEPW"),
                  os.environ.get("WEB_URL", "localhost"))
        flash('The Admin has been notified!')
        return redirect(url_for("my_races"))

    flash('Not your race!')
    return redirect(url_for("my_races"))


if __name__ == '__main__':
    app.run(host="0.0.0.0", debug=False)
