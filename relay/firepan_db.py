#!/usr/bin/python

import sqlite3 as lite
import sys

DB_PATH = "sensors.db"

def check_tables(cur):
	cur.execute('''CREATE TABLE IF NOT EXISTS actors_meta 
			(id integer, key text, value text)''')
	cur.execute('''CREATE TABLE IF NOT EXISTS sensors_data
			(actor_id integer, sensor_id integer, type text, date real, data real)''')

def set_meta(actor_id, key, value):
	conn = lite.connect(DB_PATH)

	with conn:
		cur = conn.cursor()
		check_tables(cur)
		cur.execute("SELECT * FROM actors_meta WHERE id=? AND key=?", (actor_id, key))
		
		if cur.fetchone():
			cur.execute("UPDATE actors_meta SET value=? WHERE id=? AND key=?", (value, actor_id, key))
		else:
			cur.execute("INSERT INTO actors_meta VALUES (?,?,?)", (actor_id, key, value))
		
		conn.commit()

def add(actor_id, sensor_id, sensor_type, value):
	import time

	values = (actor_id, sensor_id, sensor_type, time.time(), value)
	
	conn = lite.connect(DB_PATH)

	with conn:
		cur = conn.cursor()
		check_tables(cur)
		cur.execute("INSERT INTO sensors_data VALUES (?,?,?,?,?)", values)
		conn.commit()

def list():
	conn = lite.connect(DB_PATH)

	with conn:
		cur = conn.cursor()
		check_tables(cur)
		
		print("ACTORS META")
		print("===============")
		for entry in cur.execute("SELECT * FROM actors_meta"):
			print(entry)
		
		print("")
		
		print("SENSORS DATA")
		print("===============")
		for entry in cur.execute("SELECT * FROM sensors_data"):
			print(entry)
		
def clear():
	conn = lite.connect(DB_PATH)
	
	with conn:
		cur = conn.cursor()
		cur.execute("DELETE FROM actors_meta")
		cur.execute("DELETE FROM sensors_data")

		conn.commit()

#
# Usage : $SCRIPT_NAME primitive_id sensor_id sensor_type value
#

if len(sys.argv) < 2:
	sys.exit(1)

command = sys.argv[1]

if command == "add":
	if len(sys.argv) < 6:
		sys.exit(1)
	
	add(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
elif command == "list":
	list()
elif command == "clear":
	clear()
elif command == "set-location":
	if len(sys.argv) < 4:
		sys.exit(1)
	
	set_meta(sys.argv[2], "location", sys.argv[3])


