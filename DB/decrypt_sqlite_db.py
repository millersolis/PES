from logging import exception
import sqlite3
import hashlib
import datetime as dt
from sqlite3 import Error
from Crypto.Cipher import AES
from Crypto.Random import get_random_bytes
from Crypto.Protocol.KDF import scrypt
import io

OUTPUTPATH = '/Users/spencerpauls/Documents/School/capstone_code/database/sqlite_dbs/echo_db_unencrypted.db'
INPUTPATH = '/Users/spencerpauls/Documents/School/capstone_code/database/sqlite_dbs/echo_db_encrypted.db'


sqlite_insert_size = 5000

## encryption values
PASSWORD = "THIS IS AN ENCRYPTION PASSWORD"
PRIVATE_KEY = bytes()

BUFFER_SIZE = 1024 * 1024


def generate_private_key(password, salt):

    key = scrypt(password, salt, key_len = 32, N = 2**17, r = 8, p = 1)

    return(key)


def decrypt(encrypted, password):

    file_in = io.BytesIO(encrypted)
    file_out = io.BytesIO()

    salt = file_in.read(32)

    global PRIVATE_KEY

    if not PRIVATE_KEY:

        PRIVATE_KEY = generate_private_key(password, salt)

    nonce = file_in.read(16)
    cipher = AES.new(PRIVATE_KEY, AES.MODE_GCM, nonce=nonce)

    file_in_size = file_in.getbuffer().nbytes
    encrypted_data_size = file_in_size - 32 - 16 - 16

    for _ in range(int(encrypted_data_size / BUFFER_SIZE)):

        data = file_in.read(BUFFER_SIZE)
        decrypted_data = cipher.decrypt(data)
        file_out.write(decrypted_data)

    data = file_in.read(int(encrypted_data_size % BUFFER_SIZE))
    decrypted_data = cipher.decrypt(data)
    file_out.write(decrypted_data)

    tag = file_in.read(16)
    try:
        cipher.verify(tag)
    except ValueError as e:
       print('Tag error, potential tampering, unable to decrypt')
       return()

    return_data = file_out.getvalue()

    return_data_decoded = return_data.decode()

    return(return_data_decoded)


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


def create_sqlite_file(conn, encrypted_sqlite_file):

    current_date = dt.datetime.now()
    current_date_str = current_date.strftime('%Y-%m-%d %H:%M:%S')

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

    user_data = get_user_data(encrypted_sqlite_file)
    RID_data = get_RID_data(encrypted_sqlite_file)
    PDM_data = get_PDM_data(encrypted_sqlite_file)

    RID_table_list = []
    for doc in RID_data:

        private_key_encrypted = doc['private_RID_key']
        public_key = doc['public_RID_key']
        public_PDM_key = doc['public_PDM_key']
        user_id = doc['user_id']

        private_key_decrypted = decrypt(private_key_encrypted, PASSWORD)

        RID_table = (public_key, private_key_decrypted, public_PDM_key, user_id, current_date_str)

        if (len(RID_table_list) < sqlite_insert_size):

            RID_table_list.append(RID_table)

        else:
            insert_multiple_into_RID_table(conn, RID_table_list)
            RID_table_list.clear()

    insert_multiple_into_RID_table(conn, RID_table_list)

    user_table_list = []
    for doc in user_data:

        first_name_encrypted = doc['first_name']
        last_name_encrypted = doc['last_name']
        blood_type_encrypted = doc['blood_type']
        seat_height_encrypted = doc['seat_height']
        handlebar_height_encrypted = doc['handlebar_height']
        user_id = doc['user_id']

        first_name_decrypted = decrypt(first_name_encrypted, PASSWORD)
        last_name_decrypted = decrypt(last_name_encrypted, PASSWORD)
        blood_type_decrypted = decrypt(blood_type_encrypted, PASSWORD)
        seat_height_decrypted = decrypt(seat_height_encrypted, PASSWORD)
        handlebar_height_decrypted = decrypt(handlebar_height_encrypted, PASSWORD)

        user_table = (user_id, first_name_decrypted, last_name_decrypted, blood_type_decrypted, seat_height_decrypted, handlebar_height_decrypted, current_date_str)

        if (len(user_table_list) < sqlite_insert_size):

            user_table_list.append(user_table)

        else:
            insert_multiple_into_users_table(conn, user_table_list)
            user_table_list.clear()

    insert_multiple_into_users_table(conn, user_table_list)

    
    ## PDM ACTUALLY DOESN'T NEED TO DECRYPT ANYTHING
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

    encrypted_sqlite_file = INPUTPATH
    decrypted_sqlite_output_file = OUTPUTPATH

    print(f'Encrypted Input File Path: {encrypted_sqlite_file}')
    print(f'Decrypted Output File Path: {decrypted_sqlite_output_file}')

    conn = create_connection(decrypted_sqlite_output_file)
    create_sqlite_file(conn, encrypted_sqlite_file)

    conn.commit()
    conn.close()

    end_time = dt.datetime.now()
    elapsed_time = end_time - start_time
    print(f"Full execution time: {elapsed_time.seconds} seconds")
    print(f"End time: {end_time.strftime('%d-%b-%Y %H:%M:%S')}")