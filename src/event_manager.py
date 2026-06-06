import time
from config import CONFIDENCE_THRESHOLD, DEVICE_ID

class EventManager:
    def __init__(self):
        self.last_event = 0

    def should_trigger_event(self, result):
        if result["person_detected"] and result["confidence"] > CONFIDENCE_THRESHOLD:
            if time.time() - self.last_event > 10:
                self.last_event = time.time()
                return True
        return False

    def build_event_payload(self, result, filename, ts):
        return {
            "device_id": DEVICE_ID,
            "event_type": "person_detected",
            "confidence": result["confidence"],
            "snapshot": filename
        }
