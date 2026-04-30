#!/usr/bin/env python3
import sys
from PyQt5.QtCore import Qt, QTimer, QDateTime
from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout
from PyQt5.QtGui import QFont


def ensure_platform():
    if "-platform" in sys.argv:
        return
    if any(k.startswith("QT_QPA_PLATFORM") for k in __import__("os").environ):
        return
    sys.argv += ["-platform", "linuxfb:fb=/dev/fb0,size=320x170"]


class HelloWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setFixedSize(320, 170)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setStyleSheet("background-color: black;")

        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 8, 8, 8)
        layout.setSpacing(6)

        title = QLabel("Hello CardputerZero", self)
        tf = QFont(title.font())
        tf.setPointSize(14)
        tf.setBold(True)
        title.setFont(tf)
        title.setStyleSheet("color: white;")
        title.setAlignment(Qt.AlignCenter)

        self.clock = QLabel(self)
        cf = QFont(self.clock.font())
        cf.setPointSize(10)
        self.clock.setFont(cf)
        self.clock.setStyleSheet("color: #00ff88;")
        self.clock.setAlignment(Qt.AlignCenter)

        hint = QLabel("ESC / Q to exit", self)
        hf = QFont(hint.font())
        hf.setPointSize(8)
        hint.setFont(hf)
        hint.setStyleSheet("color: #888;")
        hint.setAlignment(Qt.AlignCenter)

        layout.addStretch()
        layout.addWidget(title)
        layout.addWidget(self.clock)
        layout.addStretch()
        layout.addWidget(hint)

        timer = QTimer(self)
        timer.timeout.connect(self.tick)
        timer.start(1000)
        self.tick()

    def tick(self):
        self.clock.setText(QDateTime.currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))

    def keyPressEvent(self, e):
        if e.key() in (Qt.Key_Escape, Qt.Key_Q):
            QApplication.instance().quit()
            return
        super().keyPressEvent(e)


def main():
    print("[hello_pyqt5] launching on 320x170 linuxfb", file=sys.stderr)
    ensure_platform()
    app = QApplication(sys.argv)
    w = HelloWindow()
    w.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
