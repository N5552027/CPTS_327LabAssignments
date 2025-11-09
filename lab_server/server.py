from flask import Flask, request, jsonify, render_template, session, redirect, url_for
import sqlite3
import hashlib
import os

app = Flask(__name__)
app.secret_key = "super_secret_key"  # Needed for session management
DATABASE = 'sqllab_users.db'

def get_db_connection():
    """Establish a connection to the SQLite database."""
    conn = sqlite3.connect(DATABASE)
    conn.row_factory = sqlite3.Row
    return conn

def init_db():
    """Initialize the database and create the credential table if it does not exist.
       Insert sample data with SHA1 hashed passwords."""
    db_exists = os.path.exists(DATABASE)
    conn = get_db_connection()
    cursor = conn.cursor()
    cursor.execute('''CREATE TABLE IF NOT EXISTS credential (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        eid TEXT,
        salary INTEGER,
        birth TEXT,
        ssn TEXT,
        nickname TEXT,
        email TEXT,
        address TEXT,
        Password TEXT,
        PhoneNumber TEXT
    );''')
    if not db_exists:
        # Insert sample data; passwords are stored as SHA1 hash
        sample_data = [
            ('admin', '99999', 400000, '3/5', '43254314', 'admin', 'admin@example.com', 'admin address', hashlib.sha1('seedadmin'.encode()).hexdigest(), '123456789'),
            ('alice', '10000', 20000, '9/20', '10211002', 'alice', 'alice@example.com', 'alice address', hashlib.sha1('seedalice'.encode()).hexdigest(), '123456789'),
            ('boby', '20000', 50000, '4/20', '10213352', 'boby', 'boby@example.com', 'boby address', hashlib.sha1('seedboby'.encode()).hexdigest(), '123456789'),
            ('ryan', '30000', 90000, '4/10', '32193525', 'ryan', 'ryan@example.com', 'ryan address', hashlib.sha1('seedryan'.encode()).hexdigest(), '123456789'),
            ('samy', '40000', 40000, '1/11', '32111111', 'samy', 'samy@example.com', 'samy address', hashlib.sha1('seedsamy'.encode()).hexdigest(), '123456789'),
            ('ted', '50000', 110000, '11/3', '24343244', 'ted', 'ted@example.com', 'ted address', hashlib.sha1('seedted'.encode()).hexdigest(), '123456789'),
        ]
        cursor.executemany(
            'INSERT INTO credential (name, eid, salary, birth, ssn, nickname, email, address, Password, PhoneNumber) VALUES (?,?,?,?,?,?,?,?,?,?)',
            sample_data
        )
    conn.commit()
    conn.close()

@app.route('/', methods=['GET', 'POST'])
def index():
    """
    Home page.
    - On GET: Displays the page with the current student username (or 'Guest' if not set)
      and a table of all users from the SQL database.
    - On POST: Accepts a student username input and saves it to the session.
    """
    if request.method == "POST":
        session['student_username'] = request.form.get('student_username', 'Guest')
        return redirect(url_for('index'))

    # Fetch all user data from the 'credential' table
    conn = get_db_connection()
    cursor = conn.cursor()
    cursor.execute("SELECT id, name, eid, salary, birth, ssn, address, email, nickname, PhoneNumber FROM credential")
    users = cursor.fetchall()
    conn.close()

    student_username = session.get('student_username', 'Guest')
    return render_template('index.html', users=users, student_username=student_username)

@app.route('/login', methods=['GET'])
def login():
    """
    Login endpoint.
    Expects 'username' and 'Password' as URL parameters.
    This version is intentionally vulnerable to SQL injection.
    """
    username = request.args.get('username', '')
    password = request.args.get('Password', '')
    hashed_pwd = hashlib.sha1(password.encode()).hexdigest()

    query = "SELECT id, name, eid, salary, birth, ssn, address, email, nickname, Password FROM credential WHERE name = '{}' and Password = '{}'".format(username, hashed_pwd)
    
    print("Executing SQL: " + query)  # Debug log
    
    conn = get_db_connection()
    cursor = conn.cursor()
    try:
        cursor.execute(query)
        row = cursor.fetchone()
    except Exception as e:
        conn.close()
        return jsonify({'error': str(e)})
    conn.close()

    if row:
        return jsonify(dict(row))
    else:
        return jsonify({'message': 'Authentication failed'})

@app.route('/update_profile', methods=['POST'])
def update_profile():
    """
    Update profile endpoint.
    Expects form data: 'id', 'nickname', 'email', 'address', 'Password', and 'PhoneNumber'.
    The new password is hashed with SHA1.
    Note: This SQL query is built using insecure string concatenation and is vulnerable to SQL injection.
    """
    user_id = request.form.get('id', '')
    nickname = request.form.get('nickname', '')
    email = request.form.get('email', '')
    address = request.form.get('address', '')
    password = request.form.get('Password', '')
    phone_number = request.form.get('PhoneNumber', '')
    
    hashed_pwd = hashlib.sha1(password.encode()).hexdigest()

    query = ("UPDATE credential SET nickname='{}', email='{}', address='{}', Password='{}', PhoneNumber='{}' "
             "WHERE id={}").format(nickname, email, address, hashed_pwd, phone_number, user_id)
    
    print("Executing SQL: " + query)  # Debug log
    conn = get_db_connection()
    cursor = conn.cursor()
    try:
        cursor.execute(query)
        conn.commit()
    except Exception as e:
        conn.close()
        return jsonify({'error': str(e)})
    conn.close()

    return jsonify({'message': 'Profile updated successfully'})

@app.route('/login_page')
def login_page():
    """Render the English login page, displaying the current student username."""
    student_username = session.get('student_username', 'Guest')
    return render_template('login.html', student_username=student_username)

@app.route('/update_page')
def update_page():
    """Render the English update profile page, displaying the current student username."""
    student_username = session.get('student_username', 'Guest')
    return render_template('update.html', student_username=student_username)

@app.route('/logout')
def logout():
    """Clear the current student username and redirect to the home page."""
    session.pop('student_username', None)
    return redirect(url_for('index'))

if __name__ == '__main__':
    init_db()
    app.run(host='0.0.0.0', port=5000, debug=True)
    print("Server is running on http://localhost:5000")
