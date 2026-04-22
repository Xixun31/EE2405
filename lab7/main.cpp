#include "mbed.h"

Thread thread_master;
Thread thread_slave;

// Master 定義
SPI spi(D11, D12, D13); // mosi, miso, sclk
DigitalOut cs(D9);

// Slave 定義 (PMOD pins)
SPISlave device(PD_4, PD_3, PD_1, PD_0); 

void slave()
{
    device.format(8, 3);
    // 預先準備好一個值 (例如 0x55)，等 Master 來抓
    device.reply(0x55); 

    while (1) {
        if (device.receive()) {
            // 讀取 Master 傳來的 Dummy byte
            int master_data = device.read(); 
            // 再次準備好下一次要給的值 (例如把收到的值加 1)
            device.reply(0x55); 
        }
    }
}

void master()
{
    spi.format(8, 3);
    spi.frequency(1000000);

    while (1) {
        cs = 0; // 開始傳輸
        // 送出一個 Dummy byte (0x00)，同時交換回 Slave 的值
        int response = spi.write(0x00); 
        cs = 1; // 結束傳輸

        // 助教要求的重點：在 Master 端印出讀到的值
        printf("Master read from Slave: 0x%X\n", response);

        ThisThread::sleep_for(1s);
    }
}

int main()
{
    thread_slave.start(slave);
    thread_master.start(master);
}