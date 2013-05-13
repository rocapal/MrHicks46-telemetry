#!/usr/bin/env python
# -*- coding: utf-8 -*- 

#  Copyright (C) 2013 Roberto Calvo
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see http://www.gnu.org/licenses/.
#
#  Authors : Roberto Calvo <rocapal at gmail dot es>


import os
import csv
import MySQLdb
import re 

db = None
cursor = None
cursor2 = None

def init_bbdd():

	global db, cursor
	# open BBDD
	db=MySQLdb.connect(host='localhost',user='root', passwd='rocapal',db='wtl_mrhicks')
	db.set_character_set('utf8')
	cursor=db.cursor()	
	cursor2 = db.cursor()



def fix_temp():

	global db, cursor
	sql = "SELECT * from data";
	sql_update =  "UPDATE data SET temperature=%.2f WHERE datetime='%s'"

	cursor.execute(sql)
	rows = cursor.fetchall()

	datetime = None
	c = 0
	for r in rows:

		if (datetime != None and r[5]!= 85.00):
			cursor.execute(sql_update % (float(r[5])- float(0.10), datetime))		
			c = c + 1	


		if (r[5]==85.00):
			datetime=r[0]
		else:
			datetime = None

	db.commit()
	

	print  "%d rows were updated with the correct temp!" % (c)
	return c


init_bbdd()
while (True):
	res = fix_temp()
	if (res == 0):
		break

db.close()


