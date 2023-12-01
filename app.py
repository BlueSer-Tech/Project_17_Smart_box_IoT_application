from flask import Flask, render_template, Response, request, jsonify, redirect, url_for, flash, session
import cv2
import socket
import gspread
from oauth2client.service_account import ServiceAccountCredentials
import json
import cv2
import numpy as np
import pyzbar.pyzbar as pyzbar
import urllib.request
import gspread
from oauth2client.service_account import ServiceAccountCredentials
import json

import threading
scopes = [
'https://www.googleapis.com/auth/spreadsheets',
'https://www.googleapis.com/auth/drive'
]
font = cv2.FONT_HERSHEY_PLAIN
cap = cv2.VideoCapture(0)
# url = 'http://192.168.1.7/'
# cv2.namedWindow("live transmission", cv2.WINDOW_AUTOSIZE)

def send_data(data):
    credentials = ServiceAccountCredentials.from_json_keyfile_name("json_file.json", scopes) #access the json key you downloaded earlier
    file = gspread.authorize(credentials) # authenticate the JSON key with gspread
    sheet = file.open("Data") #open sheet
    sheet = sheet.sheet1 #replace sheet_name with the name that corresponds to yours, e.g, it can be sheet1
    value = str(data)
    sheet.update_acell('D1', value)
def ipadd():
    hostname=socket.gethostname()
    IPAddr=socket.gethostbyname(hostname)
    return IPAddr
app = Flask(__name__)
vc = cv2.VideoCapture(1)
@app.route('/', methods=['POST', 'GET'])
def main_page():
    return render_template("index.html")
def gen1():
    while True:
        rval, frame = cap.read()
        barcodes = pyzbar.decode(frame)
        for barcode in barcodes:
            (x, y, w, h) = barcode.rect
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 0, 255), 2)
            barcodeData = barcode.data.decode("utf-8")
            text = "{}".format(barcodeData)
            cv2.putText(frame, text, (x, y - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
            t2 = threading.Thread(target=send_data, args=(text,)).start()
        cv2.imwrite('t.jpg', frame)
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + open('t.jpg', 'rb').read() + b'\r\n')
@app.route('/video_feed')
def video_feed():
    return Response(gen1(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    app.run(host=f'{ipadd()}', debug=True, threaded=True)