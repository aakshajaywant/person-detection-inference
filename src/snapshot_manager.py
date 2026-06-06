import cv2
import time

class SnapshotManager:
    def save_snapshot(self, frame, ts):
        filename = f"snapshots/event_{int(time.time())}.jpg"
        cv2.imwrite(filename, frame)
        return filename
