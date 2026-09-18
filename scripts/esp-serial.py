import serial
import json
import time

# Update to match your hardware connection: 
# /dev/ttyUSB0 (if using a USB-to-Serial adapter)
# /dev/ttyS* (if wired directly to the Orange Pi's GPIO UART pins)
SERIAL_PORT = '/dev/ttyUSB0' 
BAUD_RATE = 115200

def main():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Listening on {SERIAL_PORT}...")
    except serial.SerialException as e:
        print(f"Failed to open port: {e}")
        return

    while True:
        try:
            # Read until newline
            line = ser.readline().decode('utf-8').strip()
            
            if line:
                try:
                    payload = json.loads(line)
                    print(f"Received from ESP: {payload}")
                    
                    # Check action and respond
                    if payload.get("action") == "ping":
                        response = {
                            "host": "penny",
                            "status": "ack",
                            "received_tick": payload.get("tick")
                        }
                        
                        resp_str = json.dumps(response) + "\n"
                        ser.write(resp_str.encode('utf-8'))
                        print(f"Sent reply: {response}")
                        
                except json.JSONDecodeError:
                    print(f"Malformed JSON dropped: {line}")
                    
        except KeyboardInterrupt:
            print("\nShutting down.")
            ser.close()
            break

if __name__ == "__main__":
    main()