#import all the necessary libraries
import numpy as np
from wsgiref.simple_server import make_server
from pyramid.config import Configurator
from pyramid.response import Response, FileResponse
import mysql.connector as mysql
from dotenv import load_dotenv
import os
import time
import math
import cv2

load_dotenv('credentials.env')

''' Environment Variables '''
db_host = os.environ['MYSQL_HOST']
db_user = os.environ['MYSQL_USER']
db_pass = os.environ['MYSQL_PASSWORD']
db_name = os.environ['MYSQL_DATABASE']

objects = [
{"id" : 1, "object" : "Apple" , "count" : 0},
{"id" : 2, "object" : "Banana", "count" : 0},
{"id" : 3, "object" : "Pear", "count" : 0}]


def search(req):
    print("searching")
    id = int(req.matchdict['object'])

    # connect to our database, get lower and upper HSV range
    db = mysql.connect(host=db_host, user=db_user, passwd=db_pass, database=db_name)
    cursor = db.cursor()

    update1 = "UPDATE objects SET searching = 1 WHERE id = {};".format(id)
    update2 = "UPDATE objects SET searching = 0 WHERE id != {};".format(id)
    #print(update1)
    # print(update2)
    cursor.execute(update1)
    cursor.execute(update2)
    db.commit()
    

    select = "SELECT * FROM objects WHERE id = {};".format(id)
    cursor.execute(select)
    record = cursor.fetchone()
    
    response = {'found' : record[4], 'coordinates' : record[5]}
    print(response)
    return response



def store_coordinates(req):
    id = int(req.matchdict['object'])-1

    # connect to our database
    db = mysql.connect(host=db_host, user=db_user, passwd=db_pass, database=db_name)
    cursor = db.cursor()

    select = "SELECT coordinates FROM objects WHERE id = {}".format(id+1)
    cursor.execute(select)
    address = cursor.fetchone()[0]
    
    if address == "" :
      return 0
    
    if objects[id]['count'] == 0 :
      theObject = objects[id]["object"]
      objects[id]['count'] += 1
      

    else:
      theObject = objects[id]['object'] + str(objects[id]['count'])
      objects[id]['count'] += 1
    print(address)
    print(theObject)
    store_object = "INSERT INTO found_objects (object_name, address) Value('{}','{}');".format(theObject, address)

    cursor.execute(store_object)
    db.commit()
    db.close()







def index_page(req):
   return FileResponse("index.html")


if __name__ == '__main__':

    with Configurator() as config:

       config.add_route('home', '/')
       config.add_view(index_page, route_name='home')

       config.add_route('store', '/store/{object}')
       config.add_route('search', '/search/{object}')

       config.add_view(search, route_name='search', renderer='json')
       config.add_view(store_coordinates, route_name='store', renderer='json')

       config.add_static_view(name='/', path='./public', cache_max_age=3600)

app = config.make_wsgi_app()
server = make_server('0.0.0.0', 6543, app)
server.serve_forever()
