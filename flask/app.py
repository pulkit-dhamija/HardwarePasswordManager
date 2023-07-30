from flask import Flask, render_template
import serial

app = Flask(__name__)
statusMsg = ""

@app.route("/")
def hello_world():
    return render_template('index.html')

@app.route("/backup")
def backup():
    try:
        #read serial input from arduino using pyserial library
        ser = serial.Serial('COM4', 115200)
        while(ser.in_waiting == 0):
            pass
        msg = ser.readline().decode('utf-8')
        print(msg)
        f = open("./backup.txt", "w")
        f.write(msg)
        statusMsg = "Backup Successful"
        return statusMsg
    except:
        statusMsg = "Backup Failed"
        return statusMsg

@app.route("/restore")
def restore():
    try:
        f = open("./backup.txt", "r")
        msg = f.readline()
        ser = serial.Serial('COM4', 115200)
        ser.write(msg.encode('utf-8'))  
        statusMsg = "Restore Successful" 
        return statusMsg  
    except:
        statusMsg = "Restore Failed" 
        return statusMsg
          

app.run(host='0.0.0.0', port=8000)