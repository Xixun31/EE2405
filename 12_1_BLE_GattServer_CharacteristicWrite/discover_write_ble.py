import asyncio
from bleak import BleakScanner, BleakClient

uuid_gatt_service = '0000a600-0000-1000-8000-00805f9b34fb'
uuid_one_byte_characteristic = '0000a601-0000-1000-8000-00805f9b34fb'

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
        charset=[b'5', b'6', b'7', b'8']
        for c in charset:
            battery_level = await client.write_gatt_char(uuid_one_byte_characteristic, c, response=False)
            #print(int.from_bytes(battery_level,byteorder='big'))

asyncio.run(main())