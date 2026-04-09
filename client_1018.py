import socket

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print("Connecting to server on port 50018...")

try:
    client_socket.connect(('127.0.0.1', 50018))
    print("Connected successfully! 🎉")

# 1. යවන්න ඕන Message එක සහ ඒකේ දිග හදාගැනීම
    message = "REGISTER nimesh mypassword123"
    msg_length = len(message)
  #  msg_length = 50
    # 2. Protocol එකට අනුව සම්පූර්ණ message එක හැදීම (LEN: <n> \n <payload>)
    protocol_msg = f"LEN: {msg_length}\n{message}"
    
    # 3. ඒක Server එකට යැවීම
    client_socket.sendall(protocol_msg.encode('utf-8'))
    print(f"--> Sent to Server: {protocol_msg.replace('\n', ' ')}")

    # 4. Server එකෙන් එන Reply එක කියවීම
    response = client_socket.recv(1024).decode('utf-8')
    print(f"<-- Server Response: {response}")

except ConnectionRefusedError:
    print("Connection failed! ❌ Server එක run වෙනවද කියලා බලන්න.")

client_socket.close()