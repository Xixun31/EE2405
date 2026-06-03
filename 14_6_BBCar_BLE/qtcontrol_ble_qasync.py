from PySide6.QtCore import Signal, Slot
from PySide6.QtWidgets import QApplication, QMainWindow, QWidget, QGridLayout, QPushButton
from qasync import QEventLoop, asyncClose, asyncSlot

import asyncio
import signal
import sys
from bleak import BleakScanner, BleakClient

uuid_gatt_service = '0000a060-0000-1000-8000-00805f9b34fb'
uuid_one_byte_characteristic = '0000a061-0000-1000-8000-00805f9b34fb'

class MainWindow(QMainWindow):

    start_signal = Signal()
    forward_signal = Signal()
    left_signal = Signal()
    right_signal = Signal()
    back_signal = Signal()
    stop_signal = Signal()

    def __init__(self):
        super(MainWindow, self).__init__()

        self.setWindowTitle("BBCar Control App")
        self.buttonstart = QPushButton(f"Start", self)
        self.buttonstart.setFixedSize(120, 60)
        self.buttonstart.setStyleSheet("QPushButton" "{" "background-color : lightblue;" "}")

        self.buttonforward = QPushButton(f"Forward", self)
        self.buttonforward.setFixedSize(120, 60)
        self.buttonleft = QPushButton(f"Left", self)
        self.buttonleft.setFixedSize(120, 60)
        self.buttonright = QPushButton(f"Right", self)
        self.buttonright.setFixedSize(120, 60)
        self.buttonback = QPushButton(f"Back", self)
        self.buttonback.setFixedSize(120, 60)
        self.buttonstop = QPushButton(f"Stop", self)
        self.buttonstop.setFixedSize(120, 60)

        self.buttonstart.clicked.connect(self.async_start)
        self.buttonforward.clicked.connect(self.async_forward)
        self.buttonleft.clicked.connect(self.async_left)
        self.buttonright.clicked.connect(self.async_right)
        self.buttonback.clicked.connect(self.async_back)
        self.buttonstop.clicked.connect(self.async_stop)

        layout = QGridLayout()

        layout.addWidget(self.buttonstart, 0, 0)
        layout.addWidget(self.buttonforward, 0, 1)
        layout.addWidget(self.buttonleft, 1, 0)
        layout.addWidget(self.buttonright, 1, 2)
        layout.addWidget(self.buttonback, 2, 1)
        layout.addWidget(self.buttonstop, 1, 1)

        widget = QWidget()
        widget.setLayout(layout)
        self.setCentralWidget(widget)

        self.start_signal.connect(self.on_start)
        self.forward_signal.connect(self.on_forward)
        self.left_signal.connect(self.on_left)
        self.right_signal.connect(self.on_right)
        self.back_signal.connect(self.on_back)
        self.stop_signal.connect(self.on_stop)

    def async_start(self):
        self.start_signal.emit()

    def async_forward(self):
        self.forward_signal.emit()

    def async_left(self):
        self.left_signal.emit()

    def async_right(self):
        self.right_signal.emit()

    def async_back(self):
        self.back_signal.emit()

    def async_stop(self):
        self.stop_signal.emit()

    def on_start(self):
        asyncio.ensure_future(self.InitBLE())

    def on_forward(self):
        asyncio.ensure_future(self.onClickforward())

    def on_left(self):
        asyncio.ensure_future(self.onClickleft())

    def on_right(self):
        asyncio.ensure_future(self.onClickright())

    def on_back(self):
        asyncio.ensure_future(self.onClickback())

    def on_stop(self):
        asyncio.ensure_future(self.onClickstop())

    @asyncSlot()
    async def InitBLE(self):
        #Scan and connect to BLE service
        print("Discovering GattServer.")
        self.devices = await BleakScanner.discover()
        self.device_dict = {}
        global my_device
        for d in self.devices:
            if d.name != "Unknown" and d.name is not None:
                if(d.name=="GattServer60"):
                    my_device=d
                self.device_dict[d.name] = d
        print(self.device_dict)

        self.address=my_device.address
        global client
        print("Create client object.")
        client=BleakClient(my_device)
        print("Connect to GattServer")
        await client.connect(timeout=60)
        print("Connect to service")
        #print("Send initial byte i")
        #await client.write_gatt_char(uuid_one_byte_characteristic, b'i', response=False)

    @asyncSlot()
    async def onClickforward(self):
        print("Forward clicked.")
        await client.write_gatt_char(uuid_one_byte_characteristic, b'f', response=False)

    @asyncSlot()
    async def onClickleft(self):
        print("Left clicked.")
        await client.write_gatt_char(uuid_one_byte_characteristic, b'l', response=False)

    @asyncSlot()
    async def onClickright(self):
        print("Right clicked.")
        await client.write_gatt_char(uuid_one_byte_characteristic, b'r', response=False)

    @asyncSlot()
    async def onClickback(self):
        print("Back clicked.")
        await client.write_gatt_char(uuid_one_byte_characteristic, b'b', response=False)

    @asyncSlot()
    async def onClickstop(self):
        print("Stop clicked.")
        await client.write_gatt_char(uuid_one_byte_characteristic, b's', response=False)

app = QApplication(sys.argv)

event_loop = QEventLoop(app)
asyncio.set_event_loop(event_loop)

app_close_event = asyncio.Event()
app.aboutToQuit.connect(app_close_event.set)

main_window = MainWindow()
main_window.show()

#event_loop.create_task(main_window.boot())
event_loop.run_until_complete(app_close_event.wait())
event_loop.close()