import sqlite3
import httplib, urllib
import time


conn = sqlite3.connect('energon.sl3')

c = conn.cursor()

localtime = int(time.time())
interval = (str(localtime - (localtime % 900)),)

for row in c.execute('SELECT * FROM energylog WHERE timestamp=?', interval):
	if row[1] == 'AWATTHR':
		garage = row[2]
	elif row[1] == 'BWATTHR':
		load2 = row[2]
	elif row[1] == 'CWATTHR':
		load3 = row[2]
	elif row[1] == 'DWATTHR':
		load4 = row[2]
	elif row[1] == 'EWATTHR':
		load5 = row[2]
	elif row[1] == 'FWATTHR':
		load6 = row[2]

params = urllib.urlencode({'field1': interval,'field2': garage,'field3': load2,'field4': load3,'field5': load5,'field6': load5,'field7': load6, 'key':'N71BQXDNFTABINT7'})     # use your API key generated in the thingspeak channels for the value of 'key'

headers = {"Content-typZZe": "application/x-www-form-urlencoded","Accept": "text/plain"}

conn = httplib.HTTPConnection("api.thingspeak.com:80")                

try:
        conn.request("POST", "/update", params, headers)
        response = conn.getresponse()
        print response.status, response.reason
        data = response.read()
        conn.close()
except:
        print "connection failed"