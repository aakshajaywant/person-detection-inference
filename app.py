import time
from src.camera_manager import CameraManager
from src.inference_engine import InferenceEngine
from src.event_manager import EventManager
from src.snapshot_manager import SnapshotManager
from src.mqtt_client import MQTTClient
from src.health_monitor import HealthMonitor

def main():
    camera = CameraManager()
    inference = InferenceEngine()
    event_mgr = EventManager()
    snapshot_mgr = SnapshotManager()
    mqtt = MQTTClient()
    health = HealthMonitor()

    mqtt.connect()

    last_health = 0

    while True:
        frame, ts = camera.get_frame()
        result = inference.run_inference(frame)

        if event_mgr.should_trigger_event(result):
            filename = snapshot_mgr.save_snapshot(frame, ts)
            payload = event_mgr.build_event_payload(result, filename, ts)
            mqtt.publish_event(payload)

        if time.time() - last_health > 60:
            mqtt.publish_health(health.get_health_payload())
            last_health = time.time()

if __name__ == "__main__":
    main()
