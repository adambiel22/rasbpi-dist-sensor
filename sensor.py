

#Libraries
import RPi.GPIO as GPIO
import time
 
#GPIO Mode (BOARD / BCM)
GPIO.setmode(GPIO.BCM)
 
#set GPIO Pins
GPIO_TRIGGER = 6
GPIO_ECHO = 5
GPIO_CHANGE = 10
GPIO_ALARM = 27
 
#set GPIO direction (IN / OUT)
GPIO.setup(GPIO_TRIGGER, GPIO.OUT)
GPIO.setup(GPIO_ECHO, GPIO.IN)
GPIO.setup(GPIO_CHANGE, GPIO.IN)
GPIO.setup(GPIO_ALARM, GPIO.OUT)
 
def distance():
    # set Trigger to HIGH
    GPIO.output(GPIO_TRIGGER, True)
 
    # set Trigger after 0.01ms to LOW
    time.sleep(0.00001)
    GPIO.output(GPIO_TRIGGER, False)
 
    StartTime = time.time()
    StopTime = time.time()
 
    # save StartTime
    while GPIO.input(GPIO_ECHO) == 0:
        StartTime = time.time()
 
    # save time of arrival
    while GPIO.input(GPIO_ECHO) == 1:
        StopTime = time.time()
 
    # time difference between start and arrival
    TimeElapsed = StopTime - StartTime
    # multiply with the sonic speed (34300 cm/s)
    # and divide by 2, because there and back
    distance = (TimeElapsed * 34300) / 2
 
    return distance

# define sensor states
NORMAL_STATE = 0
WATCH_STATE = 1
ALARM_STATE = 2

if __name__ == '__main__':
    try:
        state = NORMAL_STATE
        while True:
            dist = distance()
            if state == NORMAL_STATE and GPIO.input(GPIO_CHANGE) == 0:
                state = WATCH_STATE
                watch_dist = dist
                print ("Start watching Distance = %.1f cm" % dist)
            elif state == WATCH_STATE and GPIO.input(GPIO_CHANGE) == 0:
                state = NORMAL_STATE
                print ("Stop watching Distance = %.1f cm" % dist)
            elif state == WATCH_STATE and abs(dist - watch_dist) <= 2:
                print ("Watching Distance = %.1f cm" % dist)
            elif state == WATCH_STATE and abs(dist - watch_dist) > 2:
                print("ALARM")
            else:
                print ("Measured Distance = %.1f cm" % dist)
            time.sleep(1)
 
        # Reset by pressing CTRL + C
    except KeyboardInterrupt:
        print("Measurement stopped by User")
        GPIO.cleanup()