import os
import sqlite3
from sqlite3 import Error
import datetime as dt
import pandas as pd
import uuid
import json

OUTPUTPATH = './sqlite_dbs'
SQLITE_FILENAME = 'echo_db.db'

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


def insert_into_user_table(conn, user):

    sql = ''' INSERT INTO user(user_id, first_name, last_name, creation_date)
              VALUES(?, ?, ?, ?) '''

    cur = conn.cursor()

    cur.execute(sql, user)

    conn.commit()

    return cur.lastrowid


def insert_into_RID_table(conn, keys):

    sql = ''' INSERT INTO RID(public_RID_key, private_RID_key, public_PDM_key, user_id, creation_date)
              VALUES(?, ?, ?, ?, ?) '''

    cur = conn.cursor()

    cur.execute(sql, keys)

    conn.commit()

    return cur.lastrowid


def insert_into_PDM_table(conn, keys):

    sql = ''' INSERT INTO PDM(public_PDM_key, creation_date)
              VALUES(?, ?) '''

    cur = conn.cursor()

    cur.execute(sql, keys)

    conn.commit()

    return cur.lastrowid


def update_user(conn, user):

    sql = ''' UPDATE user
            SET first_name = ?,
                last_name = ?,
                blood_type = ?,
                seat_height = ?,
                handlebar_height = ?,
                update_date = ?,
                update_by = ?
            WHERE user_id = ?'''

    cur = conn.cursor()

    cur.execute(sql, user)

    conn.commit()


def update_RID(conn, RID):

    sql = ''' UPDATE RID
            SET private_RID_key = ?,
                public_PDM_key = ?,
                user_id = ? ,
                update_date = ?,
                update_by = ?
            WHERE public_RID_key = ?'''

    cur = conn.cursor()

    cur.execute(sql, RID)

    conn.commit()


## NOTE: THIS REALLY DOESN"T WORK JUST OF THE NATURE OF THE TABLE STRUCTURE, KEEPING IT HERE ANYWAYS
def update_PDM(conn, PDM):

    sql = ''' UPDATE PDM
            SET public_PDM_key = ?,
                update_date = ?,
                update_by = ?
            WHERE public_PDM_key = ?'''

    cur = conn.cursor()

    cur.execute(sql, PDM)

    conn.commit()


def delete_user(conn, user_id):

    sql = 'DELETE FROM user WHERE user_id=?'

    cur = conn.cursor()

    cur.execute(sql, (user_id,))

    conn.commit()


def delete_all_users(conn):

    sql = 'DELETE FROM user'

    cur = conn.cursor()

    cur.execute(sql)

    conn.commit()


def delete_RID(conn, public_RID_key):

    sql = 'DELETE FROM RID WHERE public_RID_key=?'

    cur = conn.cursor()

    cur.execute(sql, (public_RID_key,))

    conn.commit()


def delete_all_RID(conn):

    sql = 'DELETE FROM RID'

    cur = conn.cursor()

    cur.execute(sql)

    conn.commit()


def delete_PDM(conn, public_PDM_key):

    sql = 'DELETE FROM PDM WHERE public_PDM_key=?'

    cur = conn.cursor()

    cur.execute(sql, (public_PDM_key,))

    conn.commit()


def delete_all_PDM(conn):

    sql = 'DELETE FROM PDM'

    cur = conn.cursor()

    cur.execute(sql)

    conn.commit()


def select_all(conn):

    query = """
        SELECT U.user_id,
            U.first_name,
            U.last_name,
            U.blood_type,
            U.seat_height,
            U.handlebar_height,
            R.public_RID_key,
            R.private_RID_key,
            P.public_PDM_key
        FROM user U
        JOIN RID R on U.user_id = R.user_id
        JOIN PDM P on R.public_PDM_key = P.public_PDM_key
    """

    df = pd.read_sql_query(query, conn)

    return(df.to_json(orient="records"))


def select_all_where_user_id(conn, user_id):

    params = (user_id,)

    query = """
        SELECT U.user_id,
            U.first_name,
            U.last_name,
            U.blood_type,
            U.seat_height,
            U.handlebar_height,
            R.public_RID_key,
            R.private_RID_key,
            P.public_PDM_key
        FROM user U
        JOIN RID R on U.user_id = R.user_id
        JOIN PDM P on R.public_PDM_key = P.public_PDM_key
        WHERE U.user_id = ?
    """

    df = pd.read_sql_query(query, conn, params = params)

    return(df.to_json(orient="records"))

def get_enrollment_table(conn, pdm_pub_key):

    params = (pdm_pub_key,)

    query = """
        SELECT R.public_RID_key,
            R.private_RID_key
        FROM RID R
        WHERE R.public_PDM_key = ?
    """

    df = pd.read_sql_query(query, conn, params = params)

    return(df.to_json(orient="records"))

def select_all_users(conn):

    df = pd.read_sql_query("SELECT * FROM user", conn)

    return(df.to_json(orient="records"))


def select_all_RID(conn):

    df = pd.read_sql_query("SELECT * FROM RID", conn)

    return(df.to_json(orient="records"))


def select_all_PDM(conn):

    df = pd.read_sql_query("SELECT * FROM PDM", conn)

    return(df.to_json(orient="records"))


def sample_creating(sqlite_db_path):
    current_date = dt.datetime.now()
    current_date_str = current_date.strftime('%Y-%m-%d %H:%M:%S')

    conn = create_connection(sqlite_db_path)

    if conn:

        # CREATE TABLES
        create_table(conn, sql_create_user_table)
        create_table(conn, sql_create_RID_table)
        create_table(conn, sql_create_PDM_table)

        with conn:

            # CREATE USER ENTRY
            user_id = str(uuid.uuid4())
            user_entry = (user_id, 'Shawn', 'Baltar', current_date_str)
            user_entry_row_number = insert_into_user_table(conn, user_entry)

            # CREATE PDM ENTRY
            PDM_public_key = str(uuid.uuid4())
            PDM_entry = (PDM_public_key, current_date_str)
            PDM_entry_row_number = insert_into_PDM_table(conn, PDM_entry)

            # CREATE RID ENTRY
            RID_private_key = str(uuid.uuid4())
            RID_public_key = str(uuid.uuid4())
            RID_entry = (RID_private_key, RID_public_key, PDM_public_key, user_id, current_date_str)
            RID_entry_row_number = insert_into_RID_table(conn, RID_entry)


def sample_updating(sqlite_db_path):
    current_date = dt.datetime.now()
    current_date_str = current_date.strftime('%Y-%m-%d %H:%M:%S')

    conn = create_connection(sqlite_db_path)

    with conn:

        # UPDATE USERS
        user_id_to_match = '62071e57-aa68-45a9-a3ed-39a40337bac3'
        user_update = ('Spencer', 'Pauls', 'O negative', '10', '13', current_date_str, 'SPauls', user_id_to_match)
        update_user(conn, user_update)

        # UPDATE PDM
        PDM_public_key_to_match = 'c798f601-04bc-4324-9938-11b8aedde97f'
        new_PDM_public_key = str(uuid.uuid4())
        PDM_update = (new_PDM_public_key, current_date_str, 'SPauls', PDM_public_key_to_match)
        update_PDM(conn, PDM_update)

        # UPDATE RID
        RID_public_key_to_match = '1d68123c-6858-4a9d-b40e-e3cc1a660ff0'
        new_RID_public_key = str(uuid.uuid4())
        new_PDM_public_key = 'ba0148c7-85d4-42c6-932d-41e9abed0ae3'
        new_user_id = '62071e57-aa68-45a9-a3ed-39a40337bac3'
        RID_update = (new_RID_public_key, new_PDM_public_key, new_user_id, current_date_str, 'SPauls', RID_public_key_to_match)
        update_RID(conn, RID_update)


## this should go in a new package/folder so the simulated ECU can get it's data, just keeping here for now
def sample_return_data(sqlite_db_path):

    current_date = dt.datetime.now()
    current_date_str = current_date.strftime('%Y-%m-%d %H:%M:%S')

    conn = create_connection(sqlite_db_path)

    with conn:

        ## QUERY USERS
        user_id_to_match = '62071e57-aa68-45a9-a3ed-39a40337bac3'

        json_1 = select_all(conn)
        json_2 = select_all_where_user_id(conn, user_id_to_match)

        parsed_json_1 = json.loads(json_1)
        parsed_json_2 = json.loads(json_2)

        print(json.dumps(parsed_json_1, indent=4))
        print(json.dumps(parsed_json_2, indent=4), '\n')

def sample_get_enrollment_table_response(sqlite_db_path, motorcycle_pdm):
    current_date = dt.datetime.now()
    current_date_str = current_date.strftime('%Y-%m-%d %H:%M:%S')

    conn = create_connection(sqlite_db_path)

    with conn:

        json_enrollment_table = get_enrollment_table(conn, motorcycle_pdm)

        parsed_json_enrollment_table = json.loads(json_enrollment_table)

        print(json.dumps(parsed_json_enrollment_table, indent=4), '\n')


if __name__ == "__main__":
    start_time = dt.datetime.now()
    start_time_str = start_time.strftime('%d-%b-%Y_%H-%M')
    print(f"Start time: {start_time.strftime('%d-%b-%Y %H:%M:%S')}", '\n')

    sqlite_file = os.path.join(OUTPUTPATH, SQLITE_FILENAME)

    # sample_creating(sqlite_file)
    # sample_updating(sqlite_file)
    # sample_return_data(sqlite_file)


    # Assumme PDM public key is b06188d2-65d6-4ffb-b9b1-578b0e35e002
    MOTORCYCLE_PDM = 'b06188d2-65d6-4ffb-b9b1-578b0e35e002'
    sample_get_enrollment_table_response(sqlite_file, MOTORCYCLE_PDM)


    end_time = dt.datetime.now()
    elapsed_time = end_time - start_time
    print(f"\nFull excution time: {elapsed_time.seconds} seconds")
    print(f"End time: {end_time.strftime('%d-%b-%Y %H:%M:%S')}")