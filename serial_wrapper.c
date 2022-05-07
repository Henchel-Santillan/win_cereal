#include "serial_wrapper.h"

#define RS232_PORTS 48

HANDLE com_ports[RS232_PORTS];
LPCSTR str_ports[RS232_PORTS] = {"\\\\.\\COM1",  "\\\\.\\COM2",  "\\\\.\\COM3",  "\\\\.\\COM4",
                                 "\\\\.\\COM5",  "\\\\.\\COM6",  "\\\\.\\COM7",  "\\\\.\\COM8",
                                 "\\\\.\\COM9",  "\\\\.\\COM10", "\\\\.\\COM11", "\\\\.\\COM12",
                                 "\\\\.\\COM13", "\\\\.\\COM14", "\\\\.\\COM15", "\\\\.\\COM16",
                                 "\\\\.\\COM17", "\\\\.\\COM18", "\\\\.\\COM19", "\\\\.\\COM20",
                                 "\\\\.\\COM21", "\\\\.\\COM22", "\\\\.\\COM23", "\\\\.\\COM24",
                                 "\\\\.\\COM25", "\\\\.\\COM26", "\\\\.\\COM27", "\\\\.\\COM28",
                                 "\\\\.\\COM29", "\\\\.\\COM30", "\\\\.\\COM31", "\\\\.\\COM32",
                                 "\\\\.\\COM33", "\\\\.\\COM34", "\\\\.\\COM35", "\\\\.\\COM36",
                                 "\\\\.\\COM37", "\\\\.\\COM38", "\\\\.\\COM39", "\\\\.\\COM40",
                                 "\\\\.\\COM41", "\\\\.\\COM42", "\\\\.\\COM43", "\\\\.\\COM44",
                                 "\\\\.\\COM45", "\\\\.\\COM46", "\\\\.\\COM47", "\\\\.\\COM48" };
char settings[128];

BOOL OpenCOMPort(int port_number, int baud_rate, char *data_stop, int mode) 
{
    if (port_number < 0 || port_number >= RS232_PORTS) 
    {
        printf("Invalid COM port number specified.");
        return FALSE;
    }

    com_ports[port_number] = CreateFileA(str_ports[port_number],
                                         GENERIC_READ | GENERIC_WRITE, 
                                         0,
                                         NULL,
                                         OPEN_EXISTING,
                                         0,
                                         NULL);

    if (com_ports[port_number] == INVALID_HANDLE_VALUE)
    {   
        printf("Unable to open COM port.");
        return FALSE;
    }

    DCB device_block;
    memset(&device_block, 0, sizeof(device_block));
    device_block.DCBlength = sizeof(device_block);

    LPCOMMPROP properties;
    GetCommProperties(com_ports[port_number], properties);

    if (baud_rate & properties->dwSettableBaud != baud_rate)
    {
        printf("Invalid baud rate specified. Defaulting to 9600 bps.");
        baud_rate = 9600;
    }

    char buffer[128];
    sprintf(buffer, "baud=%d", baud_rate);
    strcpy(settings, buffer);

    if (strlen(data_stop) != 3) 
    {
        printf("Invalid mode specified.");
        return FALSE;
    }

    buffer[0] = '\0';
    
    // Check if data_stop[0] (data set) is between 5 and 8
    if (! (isdigit(data_stop[0]) && ( (data_stop[0] - '0') - 5 ) * ( (data_stop[0] - '0') - 8 ) <= 0) )
    {
        printf("Invalid number of data bits specified.");
        return FALSE;
    }   

    sprintf(buffer, " data=%c", data_stop[0]);
    strcat(settings, buffer);
    buffer[0] = '\0';

    char parity_to_lower = tolower(data_stop[1]);
    if (! (strchr(&parity_to_lower, 'n') || strchr(&parity_to_lower, 'e') || strchr(&parity_to_lower, 'o')) )
    {
        printf("Invalid parity specified.");
        return FALSE;
    }

    sprintf(buffer, " data=%c", data_stop[1]);
    strcat(settings, buffer);
    buffer[0] = '\0';

    if (data_stop[2] != '1' || data_stop[2] != '2') 
    {
        printf("Invalid number of stop bits specified.");
        return FALSE;
    }

    sprintf(buffer, " stop=%c", data_stop[2]);
    strcat(settings, buffer);

    if (mode)
    {
        strcat(settings, " to=off xon=off odsr=off dtr=on rts=off");
        device_block.fOutxCtsFlow = TRUE;
        device_block.fRtsControl = RTS_CONTROL_HANDSHAKE;
    }

    else
        strcat(settings, " to=off xon=off odsr=off dtr=on rts=on");

    /*
    BuildCommDCBA [in] lpDef <===> 'settings'

    COMx[:][baud=b][parity=p][data=d][stop=s]
    [to={on|off}][xon={on|off}][odsr={on|off}]
    [octs={on|off}][dtr={on|off|hs}][rts={on|off|hs|tg}][idsr={on|off}]
    */

    if (!BuildCommDCBA(settings, &device_block))
    {
        printf("Failed to set COM port device control block setttings");
        CloseHandle(com_ports[port_number]);
        return FALSE;
    }

    if (!SetCommState(com_ports[port_number], &device_block)) 
    {
        printf("Failed to set COM port configuration settings.");
        CloseHandle(com_ports[port_number]);
        return FALSE;
    }

    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(com_ports[port_number], &timeouts))
    {
        printf("Failed to set COM port timeout settings.");
        CloseHandle(com_ports[port_number]);
        return FALSE;
    }

    return TRUE;
}   


BOOL CloseCOMPort(int port_number) 
{
    return CloseHandle(com_ports[port_number]);
}

int ReadByte(int port_number, unsigned char byte) {
    int bytes_read = 0;

    if (!ReadFile(com_ports[port_number], &byte, 1, (LPDWORD)((void*) &bytes_read), NULL))
    {
        printf("Failed to read byte.");
        return 0;
    }

    return bytes_read;
}

int ReadBuffer(int port_number, unsigned char *buffer, int size) 
{
    int bytes_read = 0;
    
    if (!ReadFile(com_ports[port_number], buffer, size, (LPDWORD)((void*) &bytes_read), NULL)) 
    {
        printf("Failed to read buffer.");
        return 0;
    }

    return bytes_read;
}

int WriteByte(int port_number, unsigned char byte) {
    int bytes_written = 0;
    
    if (!WriteFile(com_ports[port_number], &byte, 1, (LPDWORD)((void*) &bytes_written), NULL))
    {
        printf("Failed to write byte.");
        return 0;
    }

    return bytes_written;
}

int WriteBuffer(int port_number, unsigned char *buffer, int size) 
{
    int bytes_written = 0;
    
    if (!WriteFile(com_ports[port_number], buffer, size, (LPDWORD)((void*) &bytes_written), NULL))
    {
        printf("Failed to write byte.");
        return 0;
    }

    return bytes_written;
}

void FlushRX(int port_number) 
{
    PurgeComm(com_ports[port_number], PURGE_RXCLEAR | PURGE_RXABORT);
}

void FlushTx(int port_number) 
{
    PurgeComm(com_ports[port_number], PURGE_TXCLEAR | PURGE_TXABORT);
}

void FlushRXTX(int port_number) 
{
    FlushRX(port_number);
    FlushTx(port_number);
}
