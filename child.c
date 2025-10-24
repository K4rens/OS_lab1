#include <windows.h>
#include <stdlib.h>

int is_prime(int n) {
    if (n < 2) return 0;
    for (int i = 2; i * i <= n; i++)
        if (n % i == 0)
            return 0;
    return 1;
}

int main() {
    HANDLE hFile = CreateFile(
        "composite.txt",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        char err[] = "error open file\r\n";
        DWORD written;
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), err, strlen(err), &written, NULL);
        return 1;
    }

    char buffer[64];
    DWORD read, written;
    int num;

    while (ReadFile(GetStdHandle(STD_INPUT_HANDLE), buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
        buffer[read] = '\0';
        num = atoi(buffer);

        if (num < 0 || is_prime(num)) {
            char msg[] = "exit\r\n";
            WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), msg, strlen(msg), &written, NULL);
            break;
        } else {
            char msg[64];
            wsprintf(msg, "Composite num: %d\r\n", num);
            WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), msg, strlen(msg), &written, NULL);

            char fmsg[32];
            wsprintf(fmsg, "%d\r\n", num);
            WriteFile(hFile, fmsg, strlen(fmsg), &written, NULL);
        }
    }

    CloseHandle(hFile);
    return 0;
}
