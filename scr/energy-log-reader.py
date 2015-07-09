import sqlite3
import time
import requests

utc_time = int(time.time())
interval = utc_time - (utc_time % 900)

print interval

conn = sqlite3.connect('energon.sl3')
c = conn.cursor()

for row in c.execute('SELECT * FROM energylog WHERE timestamp=?', (str(interval),)):
	if row[1] == 'AWATTHR':
		garage = '{"id":"andrew.gillan@gmail.com/Garage", "values":[{"d":%.1f, "t":%d}]}' % (row[2], interval*1000)
	elif row[1] == 'BWATTHR':
		heatPump = '{"id":"andrew.gillan@gmail.com/Heat Pump", "values":[{"d":%.1f, "t":%d}]}' % (row[2], interval*1000)
	elif row[1] == 'CWATTHR':
		washer = '{"id":"andrew.gillan@gmail.com/Washer", "values":[{"d":%.1f, "t":%d}]}' % (row[2], interval*1000)
	elif row[1] == 'DWATTHR':
		oven = '{"id":"andrew.gillan@gmail.com/Oven", "values":[{"d":%.1f, "t":%d}]}' % (row[2], interval*1000)
	elif row[1] == 'EWATTHR':
		waterHeater = '{"id":"andrew.gillan@gmail.com/Water Heater", "values":[{"d":%.1f, "t":%d}]}' % (row[2], interval*1000)
	elif row[1] == 'FWATTHR':
		dryer = '{"id":"andrew.gillan@gmail.com/Dryer", "values":[{"d":%.1f, "t":%d}]}' % (row[2], interval*1000)

series = '[%s, %s, %s, %s, %s, %s]' % (garage, heatPump, washer, oven, waterHeater, dryer)
payload = {"email":"andrew.gillan@gmail.com", "token":"u1nfAFmevThpNIsHlQcY", "json":series}

try:
	r = requests.post("http://cloud.nimbits.com/service/v2/series", params=payload)
	print(r.url)
	print(r.text)
	print(r.encoding)
except:
        print "post failed"