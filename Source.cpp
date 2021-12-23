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

    BYTE DUMMY1[] = { 0xFF, 0xD6 , 0x00, 0x04, 0x04, 0x03, 0x08, 0xD1,0x01 };
    BYTE DUMMY2[] = { 0xFF, 0xD6 , 0x00, 0x05, 0x04, 0x04, 0x54, 0x02,0x65 };
    BYTE DUMMY3[] = { 0xFF, 0xD6 , 0x00, 0x06, 0x04, 0x06E, 0x48, 0xFE,0x00};

    // Read Data Back Code Format
    //BYTE DATA[] = {0xFF, 0xB0 , 0x00, Address of the Block, Size of Payload};

    BYTE READ_BLOCK[] = { 0xFF, 0xB0, 0x00, 0x04, 0x04 };
    BYTE READ_BLOCK2[] = { 0xFF, 0xB0, 0x00, 0x05, 0x04 };
    BYTE READ_BLOCK3[] = { 0xFF, 0xB0, 0x00, 0x06, 0x04 };

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
 
 

    // Write Dummy Message
    printf("Write DATA to Block 4 \n");
    PrintResponse(DUMMY1, sizeof(DUMMY1));
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

    // Write Dummy Message
    printf("Write DATA to Block 5 \n");
    PrintResponse(DUMMY2, sizeof(DUMMY2));
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

    // Write Dummy Message
    printf("Write DATA to Block 6 \n");
    PrintResponse(DUMMY2, sizeof(DUMMY2));
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

    // Read back data present at Block 4
    printf("Read BLOCK 4\n");
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        READ_BLOCK,
        sizeof(READ_BLOCK),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv-2);

    // Read back data present at Block 4
    printf("Read BLOCK 5\n");
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        READ_BLOCK2,
        sizeof(READ_BLOCK2),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv - 2);

    // Read back data present at Block 4
    printf("Read BLOCK 6\n");
    dwRecv = sizeof(pbRecv);
    lReturn = SCardTransmit(
        hCardHandle,
        &pioSendPci,
        READ_BLOCK3,
        sizeof(READ_BLOCK3),
        NULL,
        pbRecv,
        &dwRecv);
    if (ErrorCheck("SCardTransmit", lReturn))
        return -1;
    PrintResponse(pbRecv, dwRecv - 2);

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