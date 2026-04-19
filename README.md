# Multi-process TCP Server (IE2102 Assignment)

This project is a high-performance, secure Multi-process TCP Server developed as part of the Network Programming (IE2102) module. The server is written in C and handles concurrent client connections using the `fork()` system call.

**Student Name:** Nimesh Alwis  
**Student ID:** IT23381018  
**University:** Sri Lanka Institute of Information Technology (SLIIT)

---

## 🚀 Key Features

- **Multi-processing Architecture:** Uses `fork()` to handle multiple client connections concurrently.
- **Secure Authentication:** Implements salted password hashing to store user credentials securely.
- **Brute-Force Protection:** Automatically locks user accounts after 3 consecutive failed login attempts for 5 minutes.
- **Rate Limiting:** Implements a 2-second delay for each command to prevent rapid request flooding.
- **Input Validation:** Prevents common attacks by validating usernames and command lengths.
- **Activity Logging:** Records all successful `REGISTER` and `LOGIN` events with timestamps, IP addresses, and PIDs in a dedicated system path.
- **Child Process Management:** Uses `SIGCHLD` signals to clean up zombie processes efficiently.

---

## 📂 Project Structure

- `server_1018.c`: The core server implementation in C.
- `client_1018.py`: A Python-based client script for testing server commands.
- `Makefile_1018`: Build automation script for compiling the server.
- `server_IT23381018.log`: Activity log file (Stored in `/srv/ie2102/IT23381018/`).
- `users.txt`: Encrypted user credentials database.

---

## 🛠️ Setup and Installation

### 1. Environment Configuration
Ensure you have the required directory and permissions for data storage:
```bash
sudo mkdir -p /srv/ie2102/IT23381018/
sudo chmod 777 /srv/ie2102/IT23381018/
