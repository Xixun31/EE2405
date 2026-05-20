import paho.mqtt.client as paho
import time

# MQTT broker hosted on local machine
mqttc = paho.Client()

# Settings for connection
# 這裡的 IP 必須與你 Mbed C++ 程式中的 host 保持一致
host = "192.168.50.171"

# 注意：這裡的 topic 必須與 Mbed C++ 端的 const char* topic 完全一樣！
# 如果你在 C++ 沒改，那這裡就維持 "Mbed"
topic = "Mbed"

# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    # 【關鍵修改】: 將收到的 msg.payload (位元組格式) 解碼成正常的字串 (utf-8)
    # 這樣印出來才不會有 b'Accelerometer X...' 的醜醜格式
    decoded_payload = msg.payload.decode('utf-8')
    print(f"[Received] Topic: {msg.topic}, Message: {decoded_payload}\n")

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)

# Publish messages from Python
# 這裡會先傳送 5 次訊息給開發板，開發板的終端機應該要看到這 5 筆訊息
num = 0
while num != 5:
    ret = mqttc.publish(topic, "Message from Python!\n", qos=0)
    if (ret[0] != 0):
        print("Publish failed")
    mqttc.loop()
    time.sleep(1.5)
    num += 1

print("Ready to receive Accelerometer data from Mbed. Please press BUTTON1 on your board...")

# Loop forever, receiving messages
# 程式會停在這裡等。當你按下開發板按鈕，就會觸發上面的 on_message 把加速度計數值印出來
mqttc.loop_forever()