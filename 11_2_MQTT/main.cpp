#include "mbed.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "accelerometer.h" // 👈 引入加速度計標頭檔

// GLOBAL VARIABLES
WiFiInterface *wifi;
InterruptIn btn2(BUTTON1);
volatile int message_num = 0;
volatile int arrivedcount = 0;
volatile bool closed = false;

const char* topic = "Mbed"; // 👈 建議把 Topic 改成有意義的名稱

Thread mqtt_thread(osPriorityHigh);
EventQueue mqtt_queue;

// 宣告加速度計物件
Accelerometer accelerometer; 

void messageArrived(MQTT::MessageData& md) {
    MQTT::Message &message = md.message;
    char payload[300];
    sprintf(payload, "Payload %.*s", message.payloadlen, (char*)message.payload);
    printf("Received: %s\r\n", payload);
    ++arrivedcount;
}

void publish_message(MQTT::Client<MQTTNetwork, Countdown>* client) {
    message_num++;
    MQTT::Message message;
    char buff[100];
    
    // 1. 宣告陣列來裝感測器資料
    double rawAccelerationData[3] = {0};
    double calibratedAccelerationData[3] = {0};
    
    // 2. 讀取並校正加速度計資料
    accelerometer.GetAcceleromterSensor(rawAccelerationData);
    accelerometer.GetAcceleromterCalibratedData(calibratedAccelerationData);

    // 3. 將 X, Y, Z 數值格式化成字串 (保留小數位數)
    sprintf(buff, "Accelerometer X: %.2f, Y: %.2f, Z: %.2f", 
            calibratedAccelerationData[0], 
            calibratedAccelerationData[1], 
            calibratedAccelerationData[2]);

    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*) buff;
    message.payloadlen = strlen(buff) + 1;
    
    // 4. 發布 (Publish) 訊息
    int rc = client->publish(topic, message);

    printf("rc: %d\r\n", rc);
    printf("Published message: %s\r\n", buff);
}

void close_mqtt() {
    closed = true;
}

int main() {
    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("ERROR: No WiFiInterface found.\r\n");
        return -1;
    }

    printf("\r\nConnecting to %s...\r\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\r\nConnection error: %d\r\n", ret);
        return -1;
    }

    NetworkInterface* net = wifi;
    MQTTNetwork mqttNetwork(net);
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    // TODO: 這裡維持你電腦的 IP 位址
    const char* host = "192.168.50.171";
    const int port = 1883;
    printf("Connecting to TCP network...\r\n");
    printf("address is %s/%d\r\n", host, port);

    int rc = mqttNetwork.connect(host, port);
    if (rc != 0) {
        printf("\nConnection error: %d\r\n", rc);
        return -1;
    }
    printf("Successfully connected!\r\n");

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "Mbed_Sensor";

    if ((rc = client.connect(data)) != 0){
        printf("Fail to connect MQTT\r\n");
    }
    
    // 雖然主要是發布，但保留訂閱功能也無妨
    if (client.subscribe(topic, MQTT::QOS0, messageArrived) != 0){
        printf("Fail to subscribe\r\n");
    }

    mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));
    
    // 設定按鈕事件：按下按鈕時，觸發 publish_message
    btn2.rise(mqtt_queue.event(&publish_message, &client));

    while (1) {
        if (closed) break;
        client.yield(500);
        ThisThread::sleep_for(1500ms);
    }

    printf("Ready to close MQTT Network......\r\n");
    if ((rc = client.unsubscribe(topic)) != 0) {
        printf("Failed: rc from unsubscribe was %d\r\n", rc);
    }
    if ((rc = client.disconnect()) != 0) {
        printf("Failed: rc from disconnect was %d\r\n", rc);
    }

    mqttNetwork.disconnect();
    printf("Successfully closed!\r\n");

    return 0;
}