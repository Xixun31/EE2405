#include "mbed.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

// GLOBAL VARIABLES
EthInterface *ethernet;
InterruptIn btn2(BUTTON1);
//InterruptIn btn3(SW3);
volatile int message_num = 0;
volatile int arrivedcount = 0;
volatile bool closed = false;

const char* topic = "Mbed";

Thread mqtt_thread(osPriorityHigh);
EventQueue mqtt_queue;

void messageArrived(MQTT::MessageData& md) {
    MQTT::Message &message = md.message;
    char msg[300];
    sprintf(msg, "Message arrived: QoS%d, retained %d, dup %d, packetID %d", message.qos, message.retained, message.dup, message.id);
    printf("%s\r\n", msg);
    ThisThread::sleep_for(2000ms);
    char payload[300];
    sprintf(payload, "Payload %.*s", message.payloadlen, (char*)message.payload);
    printf("%s\r\n", payload);
    ++arrivedcount;
}

void publish_message(MQTT::Client<MQTTNetwork, Countdown>* client) {
    message_num++;
    MQTT::Message message;
    char buff[100];
    sprintf(buff, "QoS0 Hello, Python! #%d", message_num);
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*) buff;
    message.payloadlen = strlen(buff) + 1;
    int rc = client->publish(topic, message);

    printf("rc:  %d\r\n", rc);
    printf("Publish message: %s\r\n", buff);
}

void close_mqtt() {
    closed = true;
}

int main() {

    ethernet = EthInterface::get_default_instance();
    if (!ethernet) {
        printf("ERROR: No EthInterface found.\r\n");
        return -1;
    }


    printf("\r\nConnecting ...\r\n");
    int ret = ethernet->connect();
    if (ret != 0) {
        printf("\r\nConnection error: %d\r\n", ret);
        return -1;
    }


    NetworkInterface* net = ethernet;
    MQTTNetwork mqttNetwork(net);
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    //TODO: revise host to your IP
    const char* host = "192.168.50.171";
    const int port=1883;
    printf("Connecting to TCP network...\r\n");
    printf("address is %s/%d\r\n", host, port);

    int rc = mqttNetwork.connect(host, port);//(host, 1883);
    if (rc != 0) {
        printf("\nConnection error: %d\r\n", rc);
        return -1;
    }
    printf("Successfully connected!\r\n");

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "Mbed";

    if ((rc = client.connect(data)) != 0){
        printf("Fail to connect MQTT\r\n");
    }
    if (client.subscribe(topic, MQTT::QOS0, messageArrived) != 0){
        printf("Fail to subscribe\r\n");
    }

    mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));
    btn2.rise(mqtt_queue.event(&publish_message, &client));
    //btn3.rise(&close_mqtt);

    int num = 0;
    while (num != 5) {
        client.yield(100);
        ++num;
    }

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
