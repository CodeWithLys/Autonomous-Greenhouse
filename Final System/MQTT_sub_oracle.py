# ==============================================================================
# MQTT to Oracle APEX Bridge
#
# This script subscribes to an MQTT topic, listens for incoming JSON data
# from the ESP32 greenhouse monitor, and forwards it to an Oracle APEX
# RESTful Service endpoint via an HTTP POST request.
# ==============================================================================

import paho.mqtt.client as mqtt
import requests
import json
import time

# --- CONFIGURATION - YOU MUST EDIT THESE VALUES ---

# MQTT Broker Details (should match your ESP32 code)
MQTT_BROKER_HOST = "172.17.131.50"
MQTT_BROKER_PORT = 1883
MQTT_TOPIC = "greenhouse/sensor_data"

# Oracle APEX RESTful Service Details
# !!! IMPORTANT !!! You MUST replace this with the URL of your APEX REST endpoint.
# It will look something like: 'https://<your_server>/ords/<your_schema>/<module>/<template>'
APEX_URL = "https://oracleapex.com/ords/at2/greenhouse/sensor"

# --- END OF CONFIGURATION ---


def send_to_apex(payload_dict):
    """
    Sends the received sensor data payload to the Oracle APEX endpoint.

    Args:
        payload_dict (dict): A dictionary containing the parsed sensor data.
    """
    if not APEX_URL or "your-apex-instance" in APEX_URL:
        print("❌ ERROR: APEX_URL is not configured. Please edit the script.")
        return

    headers = {
        'Content-Type': 'application/json',
        'Accept': 'application/json',
        'User-Agent': 'Mozilla/5.0'
    }

    try:
        # The 'json' parameter in requests automatically serializes the dict
        # and sets the correct headers.
        response = requests.post(APEX_URL, json=payload_dict, headers=headers, timeout=10)

        # Check the response from the server
        if 200 <= response.status_code < 300:
            print(f"✅ Successfully sent data to APEX. Status Code: {response.status_code}")
        else:
            print(f"❌ Failed to send data to APEX. Status Code: {response.status_code}")
            print(f"   Response Body: {response.text}")

    except requests.exceptions.RequestException as e:
        print(f"❌ An error occurred while trying to connect to APEX: {e}")


def on_connect(client, userdata, flags, rc):
    """The callback for when the client receives a CONNACK response from the server."""
    if rc == 0:
        print("✓ Connected to MQTT Broker!")
        print(f"  Subscribing to topic: {MQTT_TOPIC}\n")
        client.subscribe(MQTT_TOPIC)
    else:
        print(f"❌ Failed to connect to MQTT Broker, return code {rc}\n")


def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    print(f"--- Message Received on Topic: {msg.topic} ---")
    
    try:
        # Decode the payload from bytes to a string
        payload_str = msg.payload.decode("utf-8")
        print(f"Payload: {payload_str}")

        # Parse the JSON string into a Python dictionary
        data_dict = json.loads(payload_str)

        # Forward the parsed data to Oracle APEX
        send_to_apex(data_dict)

    except json.JSONDecodeError:
        print("❌ Received payload is not valid JSON. Skipping.")
    except UnicodeDecodeError:
        print("❌ Could not decode payload as UTF-8. Skipping.")
    finally:
        print("------------------------------------------------\n")


def main():
    """Main function to set up and run the MQTT client."""
    client = mqtt.Client(client_id="apex-bridge-client-001")
    client.on_connect = on_connect
    client.on_message = on_message

    print("Attempting to connect to MQTT broker...")
    try:
        client.connect(MQTT_BROKER_HOST, MQTT_BROKER_PORT, 60)
    except ConnectionRefusedError:
        print("\n❌ CONNECTION REFUSED: Check that the MQTT broker is running and accessible.")
        return
    except OSError as e:
        print(f"\n❌ OS ERROR: Could not connect to {MQTT_BROKER_HOST}. Check the IP address. Error: {e}")
        return

    # Blocking call that processes network traffic, dispatches callbacks,
    # and handles reconnecting.
    client.loop_forever()


if __name__ == '__main__':
    main()
