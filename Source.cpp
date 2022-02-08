#include <stdio.h> /* Standard I/O */
#include <windows.h> /* Win32 API */
#include <winscard.h> /* SmartCard API */

BYTE ErrorCheck(
    char* string,
    LONG retval
)
{
    if (SCARD_S_SUCCESS != retval)
    {
        printf("Failed ");
        printf("%s: 0x%08X\n", string, retval);
        return 1;
    }
    return 0;
}

void PrintResponse(
    BYTE* bReponse,
    DWORD dwLen
)
{
    printf("Response: ");
    for (int i = 0; i < dwLen; i++)
        printf("%02X ", bReponse[i]);
    printf("\n");
}

int main(void)
{
    SCARDCONTEXT hSC = 0;
    SCARDHANDLE hCardHandle = 0;
    SCARD_IO_REQUEST pioSendPci;

    LPCTSTR pmszReaders = NULL;
    LPTSTR          pReader;
    LONG            lReturn, lReturn2;
    DWORD cch = SCARD_AUTOALLOCATE;
    DWORD dwActiveProtocol;

    BYTE pbRecv[258];
    DWORD dwRecv;


    // GET UID
    BYTE GET_UID[] = { 0xFF,0xCA,0x00,0x00,0x00 };

    // DUMMY MESSAGE Code Format
    //BYTE DATA[] = {0xFF, 0xD6 , 0x00, Address of the Block, Size of Payload, Payload_1, Payload_2, Payload_3,Payload_4};

    BYTE DUMMY1[] = { 0xFF, 0xD6 , 0x00, 0x04, 0x04, 0x03, 0x32, 0x92, 0x06  };
    BYTE DUMMY2[] = { 0xFF, 0xD6 , 0x00, 0x05, 0x04, 0x10, 0x44, 0x45, 0x56  };
    BYTE DUMMY3[] = { 0xFF, 0xD6 , 0x00, 0x06, 0x04, 0x45, 0x55, 0x49, 0x31  };
    BYTE DUMMY4[] = { 0xFF, 0xD6 , 0x00, 0x07, 0x04, 0x31, 0x32, 0x32, 0x33  };
    BYTE DUMMY5[] = { 0xFF, 0xD6 , 0x00, 0x08, 0x04, 0x33, 0x34, 0x34, 0x35  };
    BYTE DUMMY6[] = { 0xFF, 0xD6 , 0x00, 0x09, 0x04, 0x35, 0x36, 0x36, 0x37  };
    BYTE DUMMY7[] = { 0xFF, 0xD6 , 0x00, 0x0A, 0x04, 0x37, 0x38, 0x38, 0x52  };
    BYTE DUMMY8[] = { 0xFF, 0xD6 , 0x00, 0x0B, 0x04, 0x06, 0x10, 0x41, 0x50  };
    BYTE DUMMY9[] = { 0xFF, 0xD6 , 0x00, 0x0C, 0x04, 0x50, 0x45, 0x55, 0x49  };
    BYTE DUMMY10[] = { 0xFF, 0xD6 , 0x00, 0x0D, 0x04, 0x31, 0x31, 0x32, 0x32 };
    BYTE DUMMY11[] = { 0xFF, 0xD6 , 0x00, 0x0E, 0x04, 0x33, 0x33, 0x34, 0x34 };
    BYTE DUMMY12[] = { 0xFF, 0xD6 , 0x00, 0x0F, 0x04, 0x35, 0x35, 0x36, 0x36 };
    BYTE DUMMY13[] = { 0xFF, 0xD6 , 0x00, 0x10, 0x04, 0x37, 0x37, 0x38, 0x38 };
    BYTE DUMMY14[] = { 0xFF, 0xD6 , 0x00, 0x11, 0x04, 0xFF, 0x00, 0x00, 0x00 };

    // Read Data Back Code Format
    //BYTE DATA[] = {0xFF, 0xB0 , 0x00, Address of the Block, Size of Payload};

    // BYTE READ_BLOCK[] = { 0xFF, 0xB0, 0x00, 0x04, 0x04 };
    // BYTE READ_BLOCK2[] = { 0xFF, 0xB0, 0x00, 0x05, 0x04 };
    // BYTE READ_BLOCK3[] = { 0xFF, 0xB0, 0x00, 0x06, 0x04 };

    // Establish the context.
    lReturn = SCardEstablishContext(
        SCARD_SCOPE_USER,
        NULL,
        NULL,
        &hSC);
    if (ErrorCheck("SCardEstablishContext", lReturn))
        return -1;

    // Retrieve the list the readers.
    lReturn = SCardListReaders(hSC,
        NULL,
        (LPTSTR)&pmszReaders,
        &cch);

    switch (lReturn)
    {
    case SCARD_E_NO_READERS_AVAILABLE:
        printf("Reader is not in groups.\n");
        break;

    case SCARD_S_SUCCESS:
        // Display print reader.
        pReader = LPTSTR(pmszReaders);
        while ('\0' != *pReader)
        {
            // Display the value.
            printf("Reader: %S\n", pReader);
            // Advance to the next value.
            pReader = pReader + wcslen((wchar_t*)pReader) + 1;
        }
        break;

    default:
        printf("Failed SCardListReaders\n");
        return -1;
    }

    lReturn = SCardConnect(hSC,
        (LPCTSTR)pmszReaders,
        SCARD_SHARE_SHARED,
        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
        &hCardHandle,
        &dwActiveProtocol);
    if (ErrorCheck("SCardConnect", lReturn))
        return -1;

    // Use the connection.
    // Display the active protocol and define protocol.
    switch (dwActiveProtocol)
    {
    case SCARD_PROTOCOL_T0:
        printf("Active protocol T0\n");
        pioSendPci = *SCARD_PCI_T0;
        break;

    case SCARD_PROTOCOL_T1:
        printf("Active protocol T1\n");
        pioSendPci = *SCARD_PCI_T1;
        break;

    case SCARD_PROTOCOL_UNDEFINED:
    default:
        printf("Active protocol unnegotiated or unknown\n");
        return -1;
    }


    printf("GET UID.\n");
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        GET_UID,
        sizeof(GET_UID),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv-2); // Last two bytes 0x90 and 0x00 are for confirmation, remove it
 
 

    // Write Dummy Message1
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY1,
        sizeof(DUMMY1),
        NULL,
		pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv); 

    // Write Dummy Message2
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY2,
        sizeof(DUMMY2),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Write Dummy Message3
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY3,
        sizeof(DUMMY3),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Write Dummy Message4
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY4,
        sizeof(DUMMY4),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Write Dummy Message5
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY5,
        sizeof(DUMMY5),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Write Dummy Message6
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY6,
        sizeof(DUMMY6),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Write Dummy Message7
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY7,
        sizeof(DUMMY7),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Write Dummy Message8
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY8,
        sizeof(DUMMY8),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Write Dummy Message9
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY9,
        sizeof(DUMMY9),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Write Dummy Message10
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY10,
        sizeof(DUMMY10),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Write Dummy Message11
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY11,
        sizeof(DUMMY11),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Write Dummy Message12
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY12,
        sizeof(DUMMY12),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Write Dummy Message13
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY13,
        sizeof(DUMMY13),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Write Dummy Message14
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        DUMMY14,
        sizeof(DUMMY14),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv);

    // Free the memory.
    lReturn = SCardFreeMemory(
        hSC,
        pmszReaders
    );
    if (ErrorCheck("SCardFreeMemory", lReturn))
        return -1;

    // Free the context.
    SCardReleaseContext(hSC);
    if (ErrorCheck("SCardReleaseContext", lReturn))
        return -1;

    getchar();
    return 0;
}