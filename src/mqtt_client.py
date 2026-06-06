import paho.mqtt.client as mqtt
import json
from config import MQTT_BROKER, MQTT_PORT, TOPIC_EVENTS, TOPIC_HEALTH

class MQTTClient:
    def __init__(self):
        self.client = mqtt.Client()

    def connect(self):
        self.client.connect(MQTT_BROKER, MQTT_PORT, 60)

    def publish_event(self, payload):
        self.client.publish(TOPIC_EVENTS, json.dumps(payload))

    def publish_health(self, payload):
        self.client.publish(TOPIC_HEALTH, json.dumps(payload))
