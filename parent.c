#include <windows.h>
#include <stdint.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>

int main() {
    HANDLE pipe1[2];  // pipe1[0] - read, pipe1[1] - write
    HANDLE pipe2[2];  // pipe2[0] - read, pipe2[1] - write
    
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    
    if (!CreatePipe(&pipe1[0], &pipe1[1], &sa, 0)) return 1;
    if (!CreatePipe(&pipe2[0], &pipe2[1], &sa, 0)) return 1;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdInput = pipe1[0];   // ребёнок читает из pipe1[0]
    si.hStdOutput = pipe2[1];  // ребёнок пишет в pipe2[1]
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(
        NULL,
        "child.exe",
        NULL, NULL, TRUE,
        0, NULL, NULL,
        &si, &pi
    )) {
        DWORD err = GetLastError();
        char msg[64];
        wsprintf(msg, "error creating process(%lu)\r\n", err);
        DWORD written;
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), msg, strlen(msg), &written, NULL);
        return 1;
    }

    
    CloseHandle(pipe1[0]);  
    CloseHandle(pipe2[1]);  

    char inputBuf[64];
    char response[128];
    DWORD written, read;
    int num;

    while (1) {
        char prompt[] = "enter the num: ";
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), prompt, strlen(prompt), &written, NULL);

        // ввод
        DWORD bytesRead = 0;
        ReadFile(GetStdHandle(STD_INPUT_HANDLE), inputBuf, sizeof(inputBuf) - 1, &bytesRead, NULL);
        inputBuf[bytesRead] = '\0';

        num = atoi(inputBuf);

        
        WriteFile(pipe1[1], inputBuf, strlen(inputBuf), &written, NULL);

        
        if (ReadFile(pipe2[0], response, sizeof(response) - 1, &read, NULL) && read > 0) {
            response[read] = '\0';
            WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), response, strlen(response), &written, NULL);

            if (strncmp(response, "exit", 4) == 0)
                break;
        } else {
            char err[] = "error reading\r\n";
            WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), err, strlen(err), &written, NULL);
            break;
        }
    }

    CloseHandle(pipe1[1]);  // закрываем конец для записи в pipe1
    CloseHandle(pipe2[0]);  // закрываем конец для чтения из pipe2

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
