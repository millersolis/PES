import os
import sqlite3
from sqlite3 import Error
import datetime as dt
import pandas as pd

OUTPUTPATH = '/Users/spencerpauls/Documents/School/capstone_code/database/sqlite_dbs'
SQLITE_FILENAME = 'test_sqlite_db.db'

sql_create_user_table = """
CREATE TABLE IF NOT EXISTS user (
    id integer PRIMARY KEY,
    first_name text NOT NULL,
    last_name text NOT NULL,
    seat_height text,
    handlebar_height text,
    creation_date text,
    update_date text,
    update_by text
);
"""

sql_create_access_keys_table = """
CREATE TABLE IF NOT EXISTS access_keys (
    id integer PRIMARY KEY,
    user_id integer NOT NULL,
    private_key text NOT NULL,
    creation_date text,
    update_date text,
    update_by text,
    FOREIGN KEY (user_id) REFERENCES user (id)
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

    sql = ''' INSERT INTO user(first_name, last_name, creation_date)
              VALUES(?, ?, ?) '''

    cur = conn.cursor()

    cur.execute(sql, user)

    conn.commit()

    return cur.lastrowid


def insert_into_access_keys_table(conn, keys):

    sql = ''' INSERT INTO access_keys(private_key, user_id, creation_date)
              VALUES(?, ?, ?) '''

    cur = conn.cursor()

    cur.execute(sql, keys)

    conn.commit()

    return cur.lastrowid


def update_user(conn, user):

    sql = ''' UPDATE user
              SET first_name = ?,
                  last_name = ?,
                  seat_height = ?,
                  handlebar_height = ?,
                  update_date = ?,
                  update_by = ?
              WHERE id = ?'''

    cur = conn.cursor()

    cur.execute(sql, user)

    conn.commit()


def update_access_keys(conn, access_keys):

    sql = ''' UPDATE access_keys
              SET private_key = ?,
                  user_id = ? ,
                  update_date = ?,
                  update_by = ?
              WHERE id = ?'''

    cur = conn.cursor()

    cur.execute(sql, access_keys)

    conn.commit()


def delete_user(conn, id):

    sql = 'DELETE FROM user WHERE id=?'

    cur = conn.cursor()

    cur.execute(sql, (id,))

    conn.commit()


def delete_all_users(conn):

    sql = 'DELETE FROM user'

    cur = conn.cursor()

    cur.execute(sql)

    conn.commit()


def delete_access_keys(conn, id):

    sql = 'DELETE FROM access_keys WHERE id=?'

    cur = conn.cursor()

    cur.execute(sql, (id,))

    conn.commit()


def delete_all_access_keys(conn):

    sql = 'DELETE FROM access_keys'

    cur = conn.cursor()

    cur.execute(sql)

    conn.commit()


def select_all_users(conn):

    df = pd.read_sql_query("SELECT * FROM user", conn)

    return(df)


def select_all_access_keys(conn):

    df = pd.read_sql_query("SELECT * FROM access_keys", conn)

    return(df)


def sample_creating(sqlite_db_path):
    current_date = dt.datetime.now()
    current_date_str = current_date.strftime('%Y-%m-%d %H:%M:%S')

    conn = create_connection(sqlite_db_path)

    if conn:

        # CREATE TABLES
        create_table(conn, sql_create_user_table)
        create_table(conn, sql_create_access_keys_table)

        with conn:

            # CREATE USER ENTRY
            user_entry = ('Shawn', 'Baltar', current_date_str)
            user_id = insert_into_user_table(conn, user_entry)

            # CREATE KEYS ENTRY
            access_keys_entry = ('this_is_a_private_key',
                                 user_id, current_date_str)
            access_keys_id = insert_into_access_keys_table(
                conn, access_keys_entry)


def sample_updating(sqlite_db_path):
    current_date = dt.datetime.now()
    current_date_str = current_date.strftime('%Y-%m-%d %H:%M:%S')

    conn = create_connection(sqlite_db_path)

    with conn:

        # UPDATE USERS
        user_update = ('Spencer', 'Pauls', '10',
                       '13', current_date_str, 'SPauls', 1)
        update_user(conn, user_update)

        # UPDATE ACCESS_KEYS
        access_keys_update = ('this_is_a_NEW_private_key', 1, current_date_str, 'SPauls', 1)
        update_access_keys(conn, access_keys_update)

        # # DELETE USERS
        # delete_user(conn, 1)

        # # DELETE ALL USERS
        # delete_all_users(conn)

        # # DELETE ACCESS KEYS
        # delete_access_keys(conn, 1)

        # # DELETE ALL ACCESS KEYS
        # delete_all_access_keys(conn)

        # # QUERY ALL USERS
        # users_df = select_all_users(conn)

        # # QUERY ALL ACCESS KEYS
        # access_keys_df = select_all_access_keys(conn)


if __name__ == "__main__":
    start_time = dt.datetime.now()
    start_time_str = start_time.strftime('%d-%b-%Y_%H-%M')
    print(f"Start time: {start_time.strftime('%d-%b-%Y %H:%M:%S')}")

    sqlite_file = os.path.join(OUTPUTPATH, SQLITE_FILENAME)

    sample_creating(sqlite_file)
    # sample_updating(sqlite_file)

    end_time = dt.datetime.now()
    elapsed_time = end_time - start_time
    print(f"\nFull excution time: {elapsed_time.seconds} seconds")
    print(f"End time: {end_time.strftime('%d-%b-%Y %H:%M:%S')}")