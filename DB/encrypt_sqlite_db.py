import sqlite3
from sqlite3 import Error
import hashlib
import datetime as dt
from Crypto.Cipher import AES
from Crypto.Random import get_random_bytes
from Crypto.Protocol.KDF import scrypt
import io

OUTPUTPATH = '/Users/spencerpauls/Documents/School/capstone_code/database/sqlite_dbs/echo_db_encrypted.db'
INPUTPATH = '/Users/spencerpauls/Documents/School/capstone_code/database/sqlite_dbs/echo_db.db'

sqlite_insert_size = 5000

# encryption values
# terrible security practices but I'm too lazy to store this somewhere else
PASSWORD = "THIS IS AN ENCRYPTION PASSWORD"
SALT = get_random_bytes(32)

BUFFER_SIZE = 1024 * 1024


def generate_private_key(password, salt):

    key = scrypt(password, salt, key_len=32, N=2**17, r=8, p=1)

    return(key)


def encrypt(raw, salt, private_key):

    raw_bytes = raw.encode()

    file_in = io.BytesIO(raw_bytes)
    file_out = io.BytesIO()

    file_out.write(salt)

    cipher = AES.new(private_key, AES.MODE_GCM)
    nonce = cipher.nonce
    file_out.write(nonce)

    data = file_in.read(BUFFER_SIZE)
    while len(data) != 0:
        encrypted_data = cipher.encrypt(data)
        file_out.write(encrypted_data)
        data = file_in.read(BUFFER_SIZE)

    tag = cipher.digest()
    file_out.write(tag)

    return_data = file_out.getvalue()

    file_in.close()
    file_out.close()

    return(return_data)


def create_connection(db_file):
    conn = None

    try:

        conn = sqlite3.connect(db_file)

        return conn

    except Error as e:

        print(e)

    return conn


def create_table(conn, create_table_sql):

    try:

        c = conn.cursor()

        c.execute(create_table_sql)

    except Error as e:

        print(e)


def insert_multiple_into_RID_table(conn, data_list):

    sql = ''' INSERT OR IGNORE INTO RID(public_RID_key, private_RID_key, public_PDM_key, user_id, creation_date)
                VALUES(?, ?, ?, ?, ?) '''

    cur = conn.cursor()

    cur.executemany(sql, data_list)

    conn.commit()


def insert_multiple_into_users_table(conn, data_list):

    sql = ''' INSERT OR IGNORE INTO user(user_id, first_name, last_name, blood_type, seat_height, handlebar_height, creation_date)
                VALUES(?, ?, ?, ?, ?, ?, ?) '''

    cur = conn.cursor()

    cur.executemany(sql, data_list)

    conn.commit()


def insert_multiple_into_PDM_table(conn, data_list):

    sql = ''' INSERT OR IGNORE INTO PDM(public_PDM_key, creation_date)
                VALUES(?, ?) '''

    cur = conn.cursor()

    cur.executemany(sql, data_list)

    conn.commit()


def create_sqlite_file(conn, unencrypted_sqlite_file):

    current_date = dt.datetime.now()
    current_date_str = current_date.strftime('%Y-%m-%d %H:%M:%S')

    PRIVATE_KEY = generate_private_key(PASSWORD, SALT)

    sql_create_user_table = """
    CREATE TABLE IF NOT EXISTS user (
        user_id text PRIMARY KEY,
        first_name text NOT NULL,
        last_name text NOT NULL,
        blood_type text,
        seat_height text,
        handlebar_height text,
        creation_date text,
        update_date text,
        update_by text
    );
    """

    sql_create_RID_table = """
    CREATE TABLE IF NOT EXISTS RID (
        public_RID_key text PRIMARY KEY,
        private_RID_key text NOT NULL,
        public_PDM_key text NOT NULL,
        user_id text NOT NULL,
        creation_date text,
        update_date text,
        update_by text,
        FOREIGN KEY (user_id) REFERENCES user (user_id),
        FOREIGN KEY (public_PDM_key) REFERENCES PDM (public_PDM_key)
    );
    """

    sql_create_PDM_table = """
    CREATE TABLE IF NOT EXISTS PDM (
        public_PDM_key text PRIMARY KEY,
        creation_date text,
        update_date text,
        update_by text
    );
    """

    create_table(conn, sql_create_RID_table)
    create_table(conn, sql_create_user_table)
    create_table(conn, sql_create_PDM_table)

    user_data = get_user_data(unencrypted_sqlite_file)
    RID_data = get_RID_data(unencrypted_sqlite_file)
    PDM_data = get_PDM_data(unencrypted_sqlite_file)

    RID_table_list = []
    for doc in RID_data:

        private_key_unencrypted = doc['private_RID_key']
        public_key = doc['public_RID_key']
        public_PDM_key = doc['public_PDM_key']
        user_id = doc['user_id']

        private_key_encrypted = encrypt(str(private_key_unencrypted), SALT, PRIVATE_KEY)

        RID_table = (public_key, private_key_encrypted, public_PDM_key, user_id, current_date_str)

        if (len(RID_table_list) < sqlite_insert_size):

            RID_table_list.append(RID_table)

        else:
            insert_multiple_into_RID_table(conn, RID_table_list)
            RID_table_list.clear()

    insert_multiple_into_RID_table(conn, RID_table_list)

    user_table_list = []
    for doc in user_data:

        first_name_unencrypted = doc['first_name']
        last_name_unencrypted = doc['last_name']
        blood_type_unencrypted = doc['blood_type']
        seat_height_unencrypted = doc['seat_height']
        handlebar_height_unencrypted = doc['handlebar_height']
        user_id = doc['user_id']

        first_name_encrypted = encrypt(str(first_name_unencrypted), SALT, PRIVATE_KEY)
        last_name_encrypted = encrypt(str(last_name_unencrypted), SALT, PRIVATE_KEY)
        blood_type_encrypted = encrypt(str(blood_type_unencrypted), SALT, PRIVATE_KEY)
        seat_height_encrypted = encrypt(str(seat_height_unencrypted), SALT, PRIVATE_KEY)
        handlebar_height_encrypted = encrypt(str(handlebar_height_unencrypted), SALT, PRIVATE_KEY)

        user_table = (user_id, first_name_encrypted, last_name_encrypted, blood_type_encrypted, seat_height_encrypted, handlebar_height_encrypted, current_date_str)

        if (len(user_table_list) < sqlite_insert_size):

            user_table_list.append(user_table)

        else:
            insert_multiple_into_users_table(conn, user_table_list)
            user_table_list.clear()

    insert_multiple_into_users_table(conn, user_table_list)

    ## PDM ACTUALLY DOESN'T NEED TO ENCRYPT ANYTHING
    PDM_table_list = []
    for doc in PDM_data:

        public_PDM_key = doc['public_PDM_key']

        PDM_table = (public_PDM_key, current_date_str)

        if (len(PDM_table_list) < sqlite_insert_size):

            PDM_table_list.append(PDM_table)

        else:
            insert_multiple_into_PDM_table(conn, PDM_table_list)
            PDM_table_list.clear()

    insert_multiple_into_PDM_table(conn, PDM_table_list)


def get_RID_data(db_file):
    conn = create_connection(db_file)
    with conn:
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()

        query = """
                SELECT public_RID_key
                    , private_RID_key
                    , public_PDM_key
                    , user_id
                    , creation_date
                FROM RID;"""

        cur.execute(query)
        rows = cur.fetchall()

    return rows


def get_PDM_data(db_file):
    conn = create_connection(db_file)
    with conn:
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()

        query = """
                SELECT public_PDM_key
                    , creation_date
                FROM PDM;"""

        cur.execute(query)
        rows = cur.fetchall()

    return rows


def get_user_data(db_file):
    conn = create_connection(db_file)
    with conn:
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()

        query = """
                SELECT user_id
                    , first_name
                    , last_name
                    , blood_type
                    , seat_height
                    , handlebar_height
                    , creation_date
                FROM user;"""

        cur.execute(query)
        rows = cur.fetchall()

    return rows


if __name__ == "__main__":
    start_time = dt.datetime.now()
    start_time_str = start_time.strftime('%d-%b-%Y_%H-%M')
    print(f"Start time: {start_time.strftime('%d-%b-%Y %H:%M:%S')}")

    unencrypted_sqlite_file = INPUTPATH
    encrypted_sqlite_output_file = OUTPUTPATH

    print(f'Unencrypted Input File Path: {unencrypted_sqlite_file}')
    print(f'Encrypted Output File Path: {encrypted_sqlite_output_file}')

    conn = create_connection(encrypted_sqlite_output_file)
    create_sqlite_file(conn, unencrypted_sqlite_file)

    conn.commit()
    conn.close()

    end_time = dt.datetime.now()
    elapsed_time = end_time - start_time
    print(f"Full execution time: {elapsed_time.seconds} seconds")
    print(f"End time: {end_time.strftime('%d-%b-%Y %H:%M:%S')}")