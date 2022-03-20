import mysql.connector as mysql
import os
import datetime
from dotenv import load_dotenv #only required if using dotenv for creds
import time
import cv2
import numpy as np
import RPi.GPIO as GPIO
import time
from RpiMotorLib import RpiMotorLib
import GPS




load_dotenv('credentials.env')
db_host = os.environ['MYSQL_HOST']
db_user = os.environ['MYSQL_USER']
db_pass = os.environ['MYSQL_PASSWORD']

db = mysql.connect(user=db_user, password=db_pass, host=db_host)
cursor = db.cursor()

cursor.execute("CREATE DATABASE IF NOT EXISTS Lab8;")
cursor.execute("USE Lab8")
#cursor.execute("GRANT ALL PRIVILEGES ON Lab8.* TO 'pi'@'localhost';")
#cursor.execute("FLUSH PRIVILEGES;")
cursor.execute("DROP TABLE IF EXISTS objects;")
cursor.execute("DROP TABLE IF EXISTS found_objects;")
cursor.execute("SET time_zone='-08:00';")



try:
    cursor.execute("""
    CREATE TABLE found_objects (
    id           integer  AUTO_INCREMENT PRIMARY KEY,
    object_name  VARCHAR(50) NOT NULL,
    address      VARCHAR(50) NOT NULL,
    time         TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );
    """)

    cursor.execute("""
    CREATE TABLE objects (
    id           integer  AUTO_INCREMENT PRIMARY KEY,
    object       VARCHAR(50) NOT NULL,
    color_lower  integer NOT NULL,
    color_upper  integer NOT NULL,
    found        VARCHAR(50) NOT NULL,
    coordinates  VARCHAR(50) NOT NULL,
    searching    BIT
    );
    """)
    ### put objects data in
    # cursor.execute("INSERT INTO objects (object, color_lower, color_upper, found, coordinates,searching) Value('none',0,0,'not found','',0);")
    # db.commit()
    cursor.execute("INSERT INTO objects (object, color_lower, color_upper, found, coordinates,searching) Value('Apple',0,10,'not found','',0);")
    db.commit()
    cursor.execute("INSERT INTO objects (object, color_lower, color_upper, found, coordinates,searching) Value('Banana',15,25,'not found','',0);")
    db.commit()
    cursor.execute("INSERT INTO objects (object, color_lower, color_upper, found, coordinates,searching) Value('Pear',25,75,'not found','',0);")
    db.commit()

    db.close()

    cap=cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    frames = 0

    apple_index = 0
    banana_index = 0
    pear_index = 0
    theObject_index = 0
    coordinate1,coordinate2 = 0,0
    
    #Stepper Motor Setup
    GpioPins = [18, 23, 24, 25]

    # Declare a named instance of class pass a name and motor type
    mymotortest = RpiMotorLib.BYJMotor("MyMotorOne", "28BYJ") #min time between motor steps (ie max speed)
    step_time = .002

    #PID Gain Values (these are just starter values)
    Kp = 0.003
    Kd = 0.0001
    Ki = 0.0001

    #error values
    d_error = 0
    last_error = 0
    sum_error = 0
    counter = 0
    head = True

    while(1):
        db = mysql.connect(user=db_user, password=db_pass, host=db_host)
        cursor = db.cursor()
        cursor.execute("USE Lab8")
        cursor.execute("SELECT * FROM objects")
        record = cursor.fetchall()
        print(record)
        searching = False
        low = 0
        upp = 0
        theObject_index = 0
        
        for item in record:
            
            if item[6] == 1:
                low = int(item[2])
                upp = int(item[3])
                searching = True
                theObject_index = item[0]
                

                break
        # print(item[6])
        #print(searching," ",low," ",upp)

        # Start live feed
        if(searching):
            # Read with the USB camera
            _,frame=cap.read()
            #frames += 1
            # Convert to hsv deals better with lighting
            hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

            # Range for color filter
            lower = np.array([low,100,100])
            upper = np.array([upp,255,255])

            # Mask input image with upper and lower ranges
            color_range = cv2.inRange(hsv, lower, upper)

            # Mask for kernel opening
            mask=np.ones((5,5),np.uint8)

            # Opening operation on color only for denoising
            opening=cv2.morphologyEx(color_range, cv2.MORPH_OPEN, mask)

            # Run connected components algo to return all objects it sees.
            num_labels, labels, stats, centroids = cv2.connectedComponentsWithStats(opening,4, cv2.CV_32S)

            # Matrix showing labels for each pixel in the image
            b = np.matrix(labels)

            if num_labels > 1:
                print("number of labels : " ,num_labels)
                cursor.execute("UPDATE objects SET found = 'found' where id = {};".format(theObject_index))
                GPS_coordinate = GPS.getGPS()
                cursor.execute("UPDATE objects SET coordinates = '{}' where id = {};".format(GPS_coordinate,theObject_index))
                db.commit()
                start = time.time()
                # Extracts the label of the largest none background component
                # and displays distance from center and image.
                max_label, max_size = max([(i, stats[i, cv2.CC_STAT_AREA]) for i in range(1, num_labels)], key = lambda x: x[1])
                '''
                # Get only pixels with max_label as high (1), rest zero
                seg = (b == max_label)

                # Convert data to binary image
                seg = np.uint8(seg)
                seg[seg > 0] = 255

                if max_size > 1200:
                    # Get distance from center
                    print('distance from center:', -1 * (320 - centroids[max_label][0]))
                    found_object()
                    # 32.8555148,-177.2341130


                # Log images for debugging
                #cv2.imshow(f"./Frames/data_{frames}.png", frame)
                #cv2.imwrite(f"./Frames/seg_{frames}.png", seg)

            #else:
                #print("no object in view")
            '''
                Obj = b == max_label
                Obj = np.uint8(Obj)
                Obj[Obj > 0] = 255
                cv2.imshow('largest object', Obj)

                #calculate error from center column of masked image
                error = -1 * (320 - centroids[max_label][0])

                #speed gain calculated from PID gain values
                speed = Kp * error + Ki * sum_error + Kd * d_error
                #if negative speed change direction
                if speed < 0:
                    direction = False
                else:
                    direction = True
                #inverse speed set for multiplying step time (lower step time = faster speed)
                speed_inv = abs(1/(speed))
                #get delta time between loops
                delta_t = time.time() - start
                #calculate derivative error
                d_error = (error - last_error)/delta_t
                #integrated error
                sum_error += (error * delta_t)
                last_error = error
                #buffer of 20 only runs within 20
                if abs(error) > 20:
                    mymotortest.motor_run(GpioPins , speed_inv * step_time, 1, direction, False, "full", .05)
                else:
                    #run 0 steps if within an error of 20
                    mymotortest.motor_run(GpioPins , step_time, 0, direction, False, "full", .05)
            else:
                mymotortest.motor_run(GpioPins , .002, 5, head, False, "full", .05)
                counter = counter + 1
                print("Searching... ",counter," ",head)
                if counter == 20:
                    counter = 0
                    if head == True:
                        head = False
                    else:
                        head = True
                #print('no object in view')
        db.close()

except RuntimeError as err:
    print("runtime error: {0}".format(err))


def found_object():
    coordinate1,coordinate2 = 32.8555148,-177.2341130
    coordinates = str(coordinate1) + str(coordinate2)
    update1 = "UPDATE objects SET found = 'found' WHERE id = {};".format(theObject_index)
    update2 = "UPDATE objects SET coordinates = {} WHERE id = {};".format(coordinates, theObject_index)
    cursor.execute(update1)
    cursor.execute(update2)
    db.commit()
    
     

