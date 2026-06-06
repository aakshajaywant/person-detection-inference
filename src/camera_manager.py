import cv2
from config import CAMERA_INDEX

class CameraManager:
    def __init__(self):
        self.cap = cv2.VideoCapture(CAMERA_INDEX)

    def get_frame(self):
        ret, frame = self.cap.read()
        return frame, 0
