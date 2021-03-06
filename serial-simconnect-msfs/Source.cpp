#include <iostream>
#include <Windows.h>
#include <SimConnect.h>
#include "SerialPort.hpp" // https://github.com/manashmandal/SerialPort

using namespace std;

HANDLE hSimConnect = NULL;
HRESULT hr = E_FAIL;

enum EVENT_ID {
	KEY_AP_ALT_VAR_SET_ENGLISH
};

enum GROUP_ID {
	GROUP0,
};

const char* portName = "\\\\.\\COM6";
SerialPort* arduino;
#define DATA_LENGTH 32

int quit;

void sendSerial(string dataString)
{
    if (arduino->isConnected()) {
        dataString.append("\n");
        const char* sendString = dataString.c_str();
        cout << "\n" << sendString << endl;
        bool hasWritten = arduino->writeSerialPort(sendString, DATA_LENGTH);
        if (hasWritten) cout << "\nData Written Successfully" << endl;
        else cerr << "\nData was not written" << endl;
    }
}

// https://www.prepar3d.com/SDKv4/sdk/simconnect_api/c_simconnect_project_tutorial.html
void CALLBACK MyDispatchProc1(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
    switch (pData->dwID)
    {
    case SIMCONNECT_RECV_ID_EVENT:
    {
        SIMCONNECT_RECV_EVENT* evt = (SIMCONNECT_RECV_EVENT*)pData;

        switch (evt->uEventID)
        {
        case KEY_AP_ALT_VAR_SET_ENGLISH:
            printf("\nRef Alt on AP: %d", evt->dwData);
            char alt[32];
            sprintf_s(alt, "%d", evt->dwData);
            sendSerial(alt);
            break;

        default:
            break;
        }
        break;
    }


    case SIMCONNECT_RECV_ID_QUIT:
    {
        quit = 1;
        break;
    }

    default:
        break;
    }
}

void startListening()
{
    hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_AP_ALT_VAR_SET_ENGLISH, "AP_ALT_VAR_SET_ENGLISH");
    hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP0, KEY_AP_ALT_VAR_SET_ENGLISH);
    hr = SimConnect_SetNotificationGroupPriority(hSimConnect, GROUP0, SIMCONNECT_GROUP_PRIORITY_HIGHEST);
    if (hr == S_OK) {
        cout << "listening for events" << endl;
        while (quit == 0)
        {
            SimConnect_CallDispatch(hSimConnect, MyDispatchProc1, NULL);
            Sleep(1);
        }
    }
    else {
        cout << "failed to map client event" << endl;
    }
    hr = SimConnect_Close(hSimConnect);
}

int main() {

    arduino = new SerialPort(portName);
    cout << "arduino connected: " << arduino->isConnected() << endl;

	if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Client Event", NULL, NULL, NULL, NULL))) {
		cout << "sim connected" << endl;
        startListening();
	}
	else {
		cout << "bruh" << endl;
	}

	return 0;
}

