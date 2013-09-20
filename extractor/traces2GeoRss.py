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
import datetime
import MySQLdb
import re 
import database

db = None
cursor = None

def init_bbdd():

	global db, cursor
	# open BBDD
	db=MySQLdb.connect(host=database.HOST,user=database.USER, passwd=database.PASSWORD,db=database.NAME)
	db.set_character_set('utf8')
	cursor=db.cursor()	
	


def insert(original, new, pos):
	return original[:pos] + new + original[pos:]


# Convert to GPS coordinate to decimal degress
def gps_to_dd (coordinate):


	pos = coordinate.index(".")

	if (pos >= 3):
		degree = coordinate[0:pos-2]
		minute = coordinate[pos-2:len(coordinate)-1]
	else:
		degree = 0.0
		minute = coordinate[0:len(coordinate)-1]
	
	orientation = coordinate[len(coordinate)-1]
	sign = ""
	if orientation == 'N' or orientation == 'E':
		sign = "+"
	else:
		sign = "-"	

	return sign + str((float(degree)*1.0) + float(minute)/60.0)



def save_data (datetime, latitude, longitude, altitude, speed, temperature, image_path):

	global db, cursor
	sql_application = "INSERT INTO %s (datetime, latitude, longitude, altitude, speed, temperature, image) VALUES ('%s', %.4f, %.4f, %.3f, %.3f, %.2f, '%s');"

	print sql_application % (database.TABLE, datetime, latitude, longitude, altitude, speed, temperature, image_path) 

	try:
		cursor.execute(sql_application % (database.TABLE, datetime, latitude, longitude, altitude, speed, temperature, os.path.basename(image_path)) )
		db.commit()
	except MySQLdb.IntegrityError:
		print "-> Trace already save in database!"

	

init_bbdd()
#dir = "/home/rocapal/mrhicks/trazas/navacerrada/"
#dir = "/home/rocapal/Dropbox/MrHicks46-WTL/"
dir = "/home/rocapal/rest.worldtriplogger.com/public/data/"

for file in os.listdir(dir):

	if ( file[len(file)-1] == 'D' ):

		print "Parsing " + dir + file
		# convert the DateTime
		minute = ( "00" , file[9:11]) [int(file[9:11]) < 60]
		date_time_str = "20%s-%s-%s.%s:%s" % (file[0:2], file[2:4], file[4:6], file[6:8], minute) 
				

		try:
			date = datetime.datetime.strptime(date_time_str, '%Y-%m-%d.%H:%M')
			# read the file
			array = list( csv.reader( open( dir + file ) ) )
			temperature = array[0][0]
			latitude = gps_to_dd(array[1][0])
			longitude = gps_to_dd(array[2][0])
			altitude = array[3][0]
			speed = float(array[4][0])*1.854
			image = dir + file[0:len(file)-1]+"I"

			print date
			print array[1][0] + " , " + array[2][0] 
			print latitude + " , " + longitude
			print altitude
			print speed
			print temperature
			print "Image: " + image
			print ""

			save_data (date, float(latitude), float(longitude) ,float(altitude), float(speed), float(temperature), image)

		except IndexError:
			print "Error (IndexError)"
		except ValueError:
			print "Error (ValueError)"

db.close()


