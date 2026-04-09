#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <signal.h>   // Zombies අයින් කරන්න ඕන library එක
#include <sys/wait.h> // waitpid() වලට ඕන library එක
#include <ctype.h> // isdigit() function එකට ඕන library එක - C programming වල character handling (අකුරු check/convert කිරීම) සඳහා use කරන library එකක්
#include <time.h> //time date handdle karana library eka 
// Zombie processes අයින් කරන function එක
void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// --- Task 11: Password එක Hash කරන Function එක ---
void hash_password(const char *password, char *hashed) {
    unsigned long hash = 5381; 
    int c;
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c; 
    }
    sprintf(hashed, "%lu", hash); // Hash එක string එකක් විදියට හරවනවා
}
// -------------------------------------------------

// --- Task 16: Username Validation Function ---
// මේකෙන් බලනවා Username එකේ තියෙන්නේ අකුරු සහ ඉලක්කම් විතරද කියලා
int is_valid_username(const char *username) {
    for (int i = 0; username[i] != '\0'; i++) {
        if (!isalnum(username[i])) { // අකුරක් හරි ඉලක්කමක් හරි නෙවෙයි නම්!
            return 0; // වැරදියි (False)
        }
    }
    return 1; // ඔක්කොම හරි (True)
}

// --- Task 17: Log File එකට ලියන Function එක ---
void log_activity(const char *ip, int port, int pid, const char *command, const char *user) {
    FILE *log_file = fopen("server_IT23381018.log", "a"); // 'a' කියන්නේ පරණ ඒවා මකන්නේ නැතුව අගට ලියනවා
    if (log_file != NULL) {
        time_t now;
        time(&now);
        char *date = ctime(&now);
        date[strlen(date) - 1] = '\0'; // අගට එන newline එක අයින් කරනවා

        // [වෙලාව] IP:Port | PID: xxx | CMD: xxx | User: xxx විදියට ලියනවා
        fprintf(log_file, "[%s] IP: %s:%d | PID: %d | CMD: %s | User: %s\n", 
                date, ip, port, pid, command, user);
        fclose(log_file);
    }
}



int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[1024] = {0}; 

    // Zombie processes හදන්නේ නැතුව clean කරන්න signal එක දෙනවා
    signal(SIGCHLD, handle_sigchld);

    server_socket = socket(AF_INET, SOCK_STREAM, 0); 
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(50018); 

    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 10); // දැන් 10 දෙනෙක්ට වුනත් පෝලිමේ ඉන්න පුළුවන්
    
    printf("Server is running continuously on port 50018... Waiting for clients!\n");

    // Server එක දිගටම දුවන්න (Infinite loop)
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            continue; // කවුරුහරි එනකොට අවුලක් ගියොත්, නවතින්නේ නැතුව ඊළඟ කෙනාව ගන්නවා
        }

        printf("\n[+] New Client Connected! (Port: %d)\n", ntohs(client_addr.sin_port));

        // Fork කරනවා (අලුත් Child process එකක් හදනවා)
        pid_t pid = fork();

        if (pid == 0) {// ----- මේක තමයි Child Process එක (Client ගේ වැඩේ කරන්නේ මෙයා) -----
            // --- Task 14: Rate Limiting ---
            sleep(2);
            close(server_socket); // Child ට ප්‍රධාන දොර ඕනේ නෑ
            
            // --- Task 13: Inactivity Timeout කෑල්ල ---
            struct timeval tv;
            tv.tv_sec = 30;  // ටෙස්ට් කරන්න තත්පර 10ක් දෙමු (පස්සේ 300 කරමු)
            tv.tv_usec = 0;
            setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
            // ----------------------------------------

            int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_received > 0) {
                printf("    --> [Child PID: %d] Received:\n    %s\n", getpid(), buffer);

                int expected_len = 0;
                // '\n' (නව පේළිය) තියෙන තැන හොයාගන්නවා
                char *payload_start = strchr(buffer, '\n'); 

                // "LEN: <ගාණ>" කියන එක හරියට තියෙනවද කියලා බලනවා
                if (sscanf(buffer, "LEN: %d", &expected_len) == 1 && payload_start != NULL) {
                    
                    payload_start++; // '\n' එකෙන් පස්සේ තියෙන අකුරට (Payload එකට) යනවා
                    int actual_len = strlen(payload_start); // ඇත්තටම ආපු අකුරු ගාණ

                    // 1. Task 6: 4096 bytes වලට වඩා ලොකුද බලනවා
                    if (expected_len > 4096) {
                        char *err = "ERR 1 SID: 3810 Payload exceeds 4096 bytes!\n";
                        send(client_socket, err, strlen(err), 0);
                        printf("    <-- [Child PID: %d] ERROR 1 Sent.\n", getpid());
                    } 
                    // 2. Task 5: කියපු ගාණයි, ඇත්තටම ආපු ගාණයි අසමානද බලනවා
                    else if (actual_len != expected_len) {
                        char *err = "ERR 2 SID: 3810 Length mismatch!\n";
                        send(client_socket, err, strlen(err), 0);
                        printf("    <-- [Child PID: %d] ERROR 2 Sent.\n", getpid());
                    } 
                    // 3. ඔක්කොම හරි නම් (දිග හරියටම සමාන නම්) මෙතනින් Commands අඳුරගන්නවා
                    else {
                        char command[50] = {0}, username[50] = {0}, password[50] = {0};
                        
                        // Client එවපු message එක කොටස් වලට කඩනවා (Command, User, Pass)
                        int parsed = sscanf(payload_start, "%s %s %s", command, username, password);

                        // --- Task 16: Username එකේ අවුල් අකුරු තියෙනවද බලනවා ---
                        if (parsed >= 2 && !is_valid_username(username)) {
                            printf("    [-] Security Alert: Invalid characters in Username '%s'\n", username);
                            char *err = "ERR 6 SID: 3810 Invalid characters in Username!\n";
                            send(client_socket, err, strlen(err), 0);
                        }
                        
                        // --- REGISTER COMMAND ---
                        else if (strcmp(command, "REGISTER") == 0 && parsed == 3) {
                            printf("    [*] User trying to REGISTER: %s\n", username);
                            
                            char hashed_pwd[100];
                            hash_password(password, hashed_pwd); 

                            FILE *file = fopen("users.txt", "a"); 
                            if (file != NULL) {
                                fprintf(file, "%s %s\n", username, hashed_pwd); 
                                fclose(file);
                                printf("    [*] Password Hashed & User saved to users.txt!\n");
                                log_activity(inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), getpid(), "REGISTER", username);
                            }
                            
                            char *resp = "OK 0 SID: 3810 REGISTER success!\n";
                            send(client_socket, resp, strlen(resp), 0);
                        } 

                        // --- LOGIN COMMAND (Task 15 සහිතව) ---
                        else if (strcmp(command, "LOGIN") == 0 && parsed == 3) {
                            printf("    [*] User trying to LOGIN: %s\n", username);
                            
                            static int failed_attempts = 0; // වැරදුනු වාර ගණන
                            char hashed_pwd[100];
                            hash_password(password, hashed_pwd);

                            // users.txt එක කියවා බැලීම
                            FILE *file = fopen("users.txt", "r");
                            int login_success = 0;
                            if (file != NULL) {
                                char f_user[50], f_hash[100];
                                while (fscanf(file, "%s %s", f_user, f_hash) == 2) {
                                    if (strcmp(username, f_user) == 0 && strcmp(hashed_pwd, f_hash) == 0) {
                                        login_success = 1;
                                        break;
                                    }
                                }
                                fclose(file);
                            }

                            // Task 15: Lockout Logic
                            if (failed_attempts >= 3) {
                                printf("    [!] Security Alert: Brute-force detected! Account locked.\n");
                                char *resp = "ERR 7 SID: 3810 Account Locked! Too many failed attempts.\n";
                                send(client_socket, resp, strlen(resp), 0);
                            } 
                            else if (login_success) {
                                failed_attempts = 0; 
                                int session_token = (rand() % 9000) + 1000; 
                                printf("    [+] LOGIN SUCCESS! Token: %d\n", session_token);
                                log_activity(inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), getpid(), "LOGIN", username);
                                char resp[100];
                                sprintf(resp, "OK 0 SID: 3810 LOGIN SUCCESS. Token: %d\n", session_token);
                                send(client_socket, resp, strlen(resp), 0);
                            } 
                            else {
                                failed_attempts++; 
                                printf("    [-] LOGIN FAILED! (Attempt %d/3)\n", failed_attempts);
                                char *resp = "ERR 5 SID: 3810 Invalid Credentials!\n";
                                send(client_socket, resp, strlen(resp), 0);
                            }
                        }
                        else if (strcmp(command, "LOGOUT") == 0) {
                            printf("    [*] User LOGOUT.\n");
                            char *resp = "OK 0 SID: 3810 LOGOUT success!\n";
                            send(client_socket, resp, strlen(resp), 0);
                            printf("    <-- [Child PID: %d] LOGOUT OK Sent.\n", getpid());
                        }
                        else {
                            char *err = "ERR 4 SID: 3810 Unknown Command or Missing Arguments!\n";
                            send(client_socket, err, strlen(err), 0);
                            printf("    <-- [Child PID: %d] ERROR 4 (Unknown Cmd) Sent.\n", getpid());
                        }
                    }

                } else {
                    // Protocol එකට නැතුව නිකන්ම message එකක් එව්වොත්
                    char *err = "ERR 3 SID: 3810 Invalid Protocol Format!\n";
                    send(client_socket, err, strlen(err), 0);
                    printf("    <-- [Child PID: %d] ERROR 3 Sent.\n", getpid());
                }
            }

            close(client_socket);
            printf("[-] Client Disconnected (Handled by Child PID: %d).\n", getpid());
            exit(0); // Child ගේ වැඩේ ඉවරයි, එයා මැරෙනවා
        } 
        else if (pid > 0) {
            // ----- මේක තමයි Parent Process එක (ප්‍රධාන Server එක) -----
            close(client_socket); // Parent, client එක්ක කතා කරන්නේ නෑ, ඒක Child ට දුන්නා
        } 
        else {
            printf("Fork failed!\n");
        }
    }

    close(server_socket);
    return 0;
}