#include "mbed.h"
#include "drivers/DigitalOut.h"

#include <events/mbed_events.h>
#include "../mbed-os/connectivity/FEATURE_BLE/include/ble/BLE.h"
#include "gatt_server_process.h"
#include "mbed-trace/mbed_trace.h"

// Uncomment for actual BB Car operations
// #include "bbcar.h"

// Ticker servo_ticker;
// Ticker servo_feedback_ticker;

// PwmIn servo0_f(D9), servo1_f(D10);
// PwmOut servo0_c(D11), servo1_c(D12);
// BBCar car(servo0_c, servo0_f, servo1_c, servo1_f, servo_ticker, servo_feedback_ticker);

mbed::DigitalOut led1(LED1, 1);
mbed::DigitalOut led2(LED2, 1);
mbed::DigitalOut led3(LED3, 1);
mbed::DigitalOut *leds[] = {&led1, &led2, &led3};
// Uncomment for actual BB Car operations
// BBCar* cars[] = {&car}; //Control only one car

void stop(uint8_t car)
{
  if (car == 1)
  { // there is only one car
          *leds[car - 1] = 0;
          // Uncomment for actual BB Car operations
          // (*cars[car -1]).stop();
          printf("Car %d stop.\r\n", car);
  }
}

void goStraight(uint8_t car, int32_t speed)
{
  if (car == 1)
  { // there is only one car
          *leds[car - 1] = 0;
          // Uncomment for actual BB Car operations
          // (*cars[car -1]).goStraight(speed);
          printf("Car %d go straight at speed %d.\r\n", car, speed);
  }
}

void turn(uint8_t car, int32_t speed, double factor)
{
  if (car == 1)
  { // there is only one car
          *leds[car - 1] = 0;
          // Uncomment for actual BB Car operations
          // (*cars[car -1]).turn(speed, factor);
          printf("Car %d turn at speed %d with a factor of %f.\r\n", car, speed, factor);
  }
}

static EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);

class GattServerDemo : ble::GattServer::EventHandler {

    const static uint16_t EXAMPLE_SERVICE_UUID         = 0xA060;
    const static uint16_t WRITABLE_CHARACTERISTIC_UUID = 0xA061;

public:
    GattServerDemo()
    {
        const UUID uuid = WRITABLE_CHARACTERISTIC_UUID;
        _writable_characteristic = new ReadWriteGattCharacteristic<uint8_t> (uuid, &_characteristic_value);

        if (!_writable_characteristic) {
            printf("Allocation of ReadWriteGattCharacteristic failed\r\n");
        }
    }

    ~GattServerDemo()
    {
    }

    void start(BLE &ble, events::EventQueue &event_queue)
    {
        const UUID uuid = EXAMPLE_SERVICE_UUID;
        GattCharacteristic* charTable[] = { _writable_characteristic };
        GattService example_service(uuid, charTable, 1);

        ble.gattServer().addService(example_service);

        ble.gattServer().setEventHandler(this);

        printf("Example service UUID is 0xA060\r\n");
        printf("Writable characteristic UUID is 0xA061\r\n");
    }

private:
    /**
     * This callback allows the LEDService to receive updates to the ledState Characteristic.
     *
     * @param[in] params Information about the characteristic being updated.
     */
    virtual void onDataWritten(const GattWriteCallbackParams &params)
    {
        if ((params.handle == _writable_characteristic->getValueHandle()) && (params.len == 1)) {
            printf("New characteristic value written: %x\r\n", *(params.data));
                  char command=*(params.data);
                  switch(command) {
                          case 0x69: //i
                                  printf("Initialization received\r\n");
                                  break;
                          case 0x66: //f
                                  goStraight(1, 100);
                                  break;
                          case 0x6C: //l
                                  turn(1, 100, 0.3);
                                  break;
                          case 0x72: //r
                                  turn(1, 100, -0.3);
                                  break;
                          case 0x62: //b
                                  goStraight(1, -100);
                                  break;
                          case 0x73: //s
                                  stop(1);
                                  break;
                          default:
                                  break;
                  }
        }
    }

private:
    ReadWriteGattCharacteristic<uint8_t> *_writable_characteristic = nullptr;
    uint8_t _characteristic_value = 0;
};

int main()
{
    mbed_trace_init();

    BLE &ble = BLE::Instance();

    printf("GattServer for BB Car.\r\n");

    GattServerDemo demo;

    /* this process will handle basic setup and advertising for us */
    GattServerProcess ble_process(event_queue, ble);

    /* once it's done it will let us continue with our demo*/
    ble_process.on_init(callback(&demo, &GattServerDemo::start));

    ble_process.start();

    return 0;
}
