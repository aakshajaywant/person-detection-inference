import psutil
import time
from config import DEVICE_ID

class HealthMonitor:
    def get_health_payload(self):
        return {
            "device_id": DEVICE_ID,
            "cpu": psutil.cpu_percent(),
            "memory": psutil.virtual_memory().percent,
            "timestamp": int(time.time())
        }
