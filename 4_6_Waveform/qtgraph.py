from PySide6.QtWidgets import QApplication, QMainWindow
from PySide6.QtCore import QTimer
import pyqtgraph as pg
import sys
import serial
from random import randint


class MainWindow(QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()

        self.graphWidget = pg.PlotWidget()
        self.setCentralWidget(self.graphWidget)

        self.x = list(range(100))  # 100 time points
        self.y1 = [0.0 for _ in range(50)]  # 100 data points of 0.0
        self.y2 = [5.0 for _ in range(50)]  # 100 data points of 5.0
        self.y = self.y1+self.y2  # Combine all 100 data points similar to a step function

        self.graphWidget.setBackground("w")

        pen = pg.mkPen(color=(255, 0, 0))
        self.data_line = self.graphWidget.plot(self.x, self.y, pen=pen)

        self.timer = QTimer()
        self.timer.setInterval(100)
        self.timer.timeout.connect(self.update_plot_data)
        self.timer.start()

        serdev = "/dev/ttyACM0"
        self.s = serial.Serial(serdev , 115200)

    def update_plot_data(self):
        # Only update if there is input from serial
        if self.s.inWaiting() > 0:
            # read the bytes and convert from binary array to ASCII
            data_str = self.s.read(self.s.inWaiting()).decode("ascii")
            if(data_str!='\r\n'):
                ynew = float(data_str)
                print('data_str='+repr(data_str)+' ynew='+repr(ynew))

                self.x = self.x[1:]  # Remove the first y element.
                self.x.append(self.x[-1] + 1)  # Add a new value 1 higher than the last.

                self.y = self.y[1:]  # Remove the first
                self.y.append(ynew)  # Add a new random value.

                self.data_line.setData(self.x, self.y)  # Update the data.

app = QApplication(sys.argv)
main = MainWindow()
main.show()
app.exec()