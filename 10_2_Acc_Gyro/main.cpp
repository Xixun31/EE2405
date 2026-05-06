#include "mbed.h"
#include "accelerometer.h"
#include "gyro.h"

#include "erpc_simple_server.hpp"
#include "erpc_basic_codec.hpp"
#include "erpc_crc16.hpp"
#include "UARTTransport.h"
#include "DynamicMessageBufferFactory.h"
#include "acc_gyro_server.h"

/****** erpc declarations *******/

Accelerometer accelerometer;
Gyro gyroscope;

void GetAccelerometer(double *acce_x, double *acce_y, double *acce_z) {
    double rawAccelerationData[3] = {0};
    double calibratedAccelerationData[3] = {0};
    // Read raw accelerometer data
    accelerometer.GetAcceleromterSensor(rawAccelerationData);
    // Calibrate accelerometer data
    accelerometer.GetAcceleromterCalibratedData(calibratedAccelerationData);

    *acce_x = calibratedAccelerationData[0];
    *acce_y = calibratedAccelerationData[1];
    *acce_z = calibratedAccelerationData[2];
}

void GetGyro(double *gyro_x, double *gyro_y, double *gyro_z) {
    double rawGyroData[3] = {0};
    double calibratedGyroData[3] = {0};
    // Read raw gyroscope data
    gyroscope.GetGyroSensor(rawGyroData);
    // Calibrate gyroscope data
    gyroscope.GetGyroCalibratedData(calibratedGyroData);

    *gyro_x = calibratedGyroData[0];
    *gyro_y = calibratedGyroData[1];
    *gyro_z = calibratedGyroData[2];
}

/** erpc infrastructure */
ep::UARTTransport uart_transport(D1, D0, 9600);
ep::DynamicMessageBufferFactory dynamic_mbf;
erpc::BasicCodecFactory basic_cf;
erpc::Crc16 crc16;
erpc::SimpleServer rpc_server;

acc_gyroService_service acc_gyro_service;

int main() {
    // Initialize the rpc server
    uart_transport.setCrc16(&crc16);

    printf("Initializing server.\n");
    rpc_server.setTransport(&uart_transport);
    rpc_server.setCodecFactory(&basic_cf);
    rpc_server.setMessageBufferFactory(&dynamic_mbf);

    // Add the led service to the server
    printf("Adding server.\n");
    rpc_server.addService(&acc_gyro_service);

    // Run the server. This should never exit
    printf("Running server.\n");
    rpc_server.run();
}