import serial
import json
import time

# Update to match your hardware connection: 
# /dev/ttyUSB0 (if using a USB-to-Serial adapter)
# /dev/ttyS* (if wired directly to the Orange Pi's GPIO UART pins)
SERIAL_PORT = '/dev/ttyS2' 
BAUD_RATE = 115200

def main():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Listening on {SERIAL_PORT}...")
        play_bootanimation(ser)
        
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
                            "action": "status",
                            "tick": payload.get("tick"),
                            "status": 0
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

def play_bootanimation(ser):
    payload = {
        "host": "penny",
        "action": "play-bootanimation",
    }
    payload_str = json.dumps(payload) + "\n"
    ser.write(payload_str.encode('utf-8'))
    time.sleep(10)

if __name__ == "__main__":
    main()