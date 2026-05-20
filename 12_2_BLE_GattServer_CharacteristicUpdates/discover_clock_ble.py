import asyncio
from bleak import BleakScanner, BleakClient
import time

uuid_clock_service = '0000b600-0000-1000-8000-00805f9b34fb'
uuid_hour_characteristic = '0000b601-0000-1000-8000-00805f9b34fb'
uuid_min_characteristic = '0000b602-0000-1000-8000-00805f9b34fb'
uuid_second_characteristic = '0000b603-0000-1000-8000-00805f9b34fb'

async def main():
    devices = await BleakScanner.discover()
    device_dict = {}
    for d in devices:
        if d.name != "Unknown" and d.name is not None:
            if(d.name=="GattServer60"):
                my_device=d
            device_dict[d.name] = d
    print(device_dict)

    address=my_device.address
    async with BleakClient(address) as client:
        svcs = await client.get_services()
        while True:
            hour_byte = await client.read_gatt_char(uuid_hour_characteristic, response=False)
            min_byte = await client.read_gatt_char(uuid_min_characteristic, response=False)
            second_byte = await client.read_gatt_char(uuid_second_characteristic, response=False)
            hour=int.from_bytes(hour_byte)
            min=int.from_bytes(min_byte)
            second=int.from_bytes(second_byte)
            print(f"hour:min:second: {hour}:{min}:{second}")
            time.sleep(1)

asyncio.run(main())