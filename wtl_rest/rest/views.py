# -*- coding: utf-8 -*- 
import sys
from django.http import HttpResponse
from django.utils import simplejson
import MySQLdb
import collections
import os
import datetime

from wtl_rest.config import database, server

JSON_MIMETYPE="application/json"
XML_MIMETYPE ="application/xml"

db = None
cursor = None

## Database functions 
def init_DB():

	global db, cursor
	# open BBDD
	db=MySQLdb.connect(host=database.HOST,user=database.USER, passwd=database.PASSWORD,db=database.NAME)
	db.set_character_set('utf8')
	cursor=db.cursor()

def close_DB():
	global db
	db.commit()
	db.close()	

def get_simple_sql (sql):

	global db, cursor
	cursor.execute(sql)
	rows = cursor.fetchall()
	
	return str(rows[0][0])
	

## Database functions 

def version(request):
	data = {'code' : '200',
		'name': 'WorldTripLogger',
		'version' : '0.1'}

	return HttpResponse(simplejson.dumps(data), JSON_MIMETYPE)


def dates(request):
	global db, cursor

	"""
	This function will return a JSON with all the dates in database with information.
	It will in order.

	@param request: Object that contains information about the petition


	@return: HTTPResponse with JSON
	"""
	init_DB()

	sql = "select DISTINCT(DATE(datetime)) from %s ORDER BY DATE(datetime) DESC;" % (database.TABLE)
	cursor.execute(sql)
	rows = cursor.fetchall()
	
	objects_list = []
	for r in rows:
		t = str(r[0])
		objects_list.append(t)


	close_DB()
	
	return HttpResponse(simplejson.dumps(objects_list), JSON_MIMETYPE)



def traces_georss(request, date):

	global db, cursor

	# Interval in seconds
	interval = -1
	
	if ("interval" in request.GET):
		interval = int(request.GET['interval']) * 60

	georss = "<feed xmlns='http://www.w3.org/2005/Atom' xmlns:dc='http://purl.org/dc/elements/1.1/' xmlns:geo='http://www.w3.org/2003/01/geo/wgs84_pos#' xmlns:georss='http://www.georss.org/georss' xmlns:woe='http://where.yahooapis.com/v1/schema.rng' xmlns:flickr='urn:flickr:user' xmlns:media='http://search.yahoo.com/mrss/'>"
	georss += "<title> WorldTripLogger </title>"
	georss += "<subtitle>MrHicks46 - " + date + "</subtitle>"

	init_DB()
	sql = "SELECT * FROM %s WHERE DATE(datetime)='%s';" % (database.TABLE, date)
	cursor.execute(sql)
	rows = cursor.fetchall()

	last_datetime = None
	for r in rows:

		if (interval != -1) and (last_datetime != None):
			d1 = datetime.datetime.strptime(last_datetime, '%Y-%m-%d %H:%M:%S')
			d2 = datetime.datetime.strptime(str(r[0]), '%Y-%m-%d %H:%M:%S')
			
			if ((d2-d1).seconds < interval):
				continue
				

		last_datetime = str(r[0])

		georss += "<entry>"
		georss += "<title>" + str(r[0]) + "</title>"

		georss += "<content type='html'>"
	
		url_image = "http://" + server.NAME + "/" + server.DATA_REST + "/" + str(r[6]) 
		georss += "&lt;p&gt;&lt;a href=&quot;" + url_image + "&quot; &gt;&lt;img src=&quot;" + url_image + "&quot; width=&quot;240&quot; height=&quot;179&quot; /&gt;&lt;/a&gt;&lt;/p&gt;"
		
		georss += "&lt;p&gt; Temp: " + str(r[5]).replace(".",",") + " C &lt;/p&gt;"
		georss += "&lt;p&gt; Altitude: " + str(r[3]).replace(".",",") + " m. &lt;/p&gt;"
		georss += "&lt;p&gt; Speed: " + str(r[4]).replace(".",",") + " Km/h &lt;/p&gt;"
		georss += "</content>"

		georss += "<geo:lat>" + str(r[1]) + "</geo:lat>"
		georss += "<geo:long>" + str(r[2]) + "</geo:long>"
		georss += "</entry>"

		

	georss += "</feed>"

	close_DB()

	return HttpResponse(georss, XML_MIMETYPE)

def traces(request, date):

	global db, cursor

	"""
	This function will return a JSON with all the traces of one day

	@param request: Object that contains information about the petition

	@return: HTTPResponse with JSON
	"""
	init_DB()

	sql = "SELECT * FROM %s WHERE DATE(datetime)='%s';" % (database.TABLE, date)
	cursor.execute(sql)
	rows = cursor.fetchall()

	if (len(rows) == 0):
		return HttpResponse(simplejson.dumps(""), JSON_MIMETYPE)

	traces_list = []
	for r in rows:
		d = collections.defaultdict()
		d['datetime'] = str(r[0])
		d['latitude'] = r[1]
		d['longitude'] = r[2]
		d['altitude'] = r[3]
		d['speed'] = r[4]
		d['temperature'] = r[5]
		d['image'] = (str(r[6]))

		traces_list.append(d)

	

	r_data = collections.defaultdict()
	r_data['num_traces'] = int(get_simple_sql ("SELECT COUNT(*) FROM %s WHERE DATE(datetime)='%s';" % (database.TABLE, date)))
	r_data['max_temp'] = float(get_simple_sql ("SELECT MAX(temperature) FROM %s WHERE DATE(datetime)='%s';" % (database.TABLE, date)))
	r_data['min_temp'] = float(get_simple_sql ("SELECT MIN(temperature) FROM %s WHERE DATE(datetime)='%s';" % (database.TABLE, date)))
	r_data['max_speed'] = float(get_simple_sql ("SELECT MAX(speed) FROM %s WHERE DATE(datetime)='%s';" % (database.TABLE, date)))
	r_data['max_altitude'] = float(get_simple_sql ("SELECT MAX(altitude) FROM %s WHERE DATE(datetime)='%s';" % (database.TABLE, date)))
	r_data['acc_altitude'] = float(rows[len(rows)-1][3] - rows[0][3])
	#r_data['traces'] = traces_list	

	close_DB()

	return HttpResponse(simplejson.dumps(r_data), JSON_MIMETYPE)










