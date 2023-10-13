#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <chrono>
#include <vector>
#include <thread>
#include <iomanip>
using namespace std;

// Define the Structure that will be used to store the Key pressed and the Elapse time
struct KeyInfo {
    string key_name;
    double elapsed_time;
};
// Declare Vector of KeyInfo
static vector<KeyInfo>* keylogged;

// Define a flag to indicate whether the program should exit
static bool exitProgram = false;

// Define verbose flag
static bool verbose = true;

// Define monitoring flag. If true, do not keylogged
static bool monitoring = false;

// Define Detection flags
static bool keystroke_speed = false;
static bool keystroke_words = false;
static bool keystroke_pattern = false;
static bool new_keyboard_found = false;

// THREAD Function to reset the detection flags
void resetDetectionFlags(int timer) {
    while (!exitProgram) {
        if (keystroke_pattern || keystroke_speed || keystroke_words) {
            keystroke_speed = false;
            keystroke_words = false;
            keystroke_pattern = false;
            new_keyboard_found = false;
            if (verbose) {
                cout << "[*] Detection flags reset" << endl;
            }
        }
        this_thread::sleep_for(chrono::minutes(timer));
    }

    if (verbose) {
        cout << "[*] Closing resetDetectionFlags thread" << endl;
    }

}

// Function to write to EventVwr Application (Need to modify general message) - (event type EVENTLOG_ERROR_TYPE, EVENTLOG_WARNING_TYPE, EVENTLOG_INFORMATION_TYPE)
BOOL report_event(const wchar_t* eventmessage, WORD eventtype) {

    HANDLE hLog = RegisterEventSourceW(0, L"Rubber-Duck-Hunt");
    if (!hLog)
    {
        cerr << "RegisterEventSourceW() failed with " << GetLastError() << endl;
        return 1;
    }

    BOOL res = ReportEventW(hLog, eventtype, 0, 1337, 0, 1, 0, &eventmessage, 0);

    if (!res)
    {
        cerr << "ReportEventW() failed with " << GetLastError() << endl;
        CloseEventLog(hLog);
        return 1;
    }

    CloseEventLog(hLog);
    return 0;
}

// Function to verify sequence of key in Vector
bool checkForKeySequence(const vector<KeyInfo>& data, const vector<string>& sequence) {
    size_t dataIndex = 0;
    size_t sequenceIndex = 0;

    while (dataIndex < data.size()) {
        if (data[dataIndex].key_name == sequence[sequenceIndex]) {
            sequenceIndex++;
            if (sequenceIndex == sequence.size()) {
                return true;
            }
        }
        else {
            sequenceIndex = 0;  // Reset sequence index if the key does not match
        }
        dataIndex++;
    }

    return false;
}

// THREAD Function to detect keystroke patterns
void detection_keystroke_pattern() {
    vector<vector<string>> keySequences = {
        {"Left Windows", "R"},
        {"WINDOWS GAUCHE", "R"},
        {"Left Windows", "DOWN"},
        {"ALT", "F4"}
        };
    while (!exitProgram) {
        while (!keystroke_pattern) {
            //cout << "[*] Keystroke Pattern Check Engaged";
            for (const auto& sequence : keySequences) {
                if (checkForKeySequence(*keylogged, sequence)) {
                    cout << "[*] Keystroke Pattern Detected: ";
                    for (const auto& key : sequence) {
                        cout << "[" << key << "]";
                    }
                    cout << endl;
                    // Set Pattern detection flag to true
                    keystroke_pattern = true;

                    // Report to eventvwr
                    const wchar_t* eventmessage = L"Abnormal keystroke pattern detected!";
                    report_event(eventmessage, EVENTLOG_WARNING_TYPE);
                }
            }

            this_thread::sleep_for(chrono::seconds(1));
        }
    this_thread::sleep_for(chrono::seconds(1));
    }
}

// Function to verify words in Vector
bool checkForWord(const vector<KeyInfo>& data, const string& word) {
    string concatenatedKeys;
    for (const auto& keyInfo : data) {
        concatenatedKeys += keyInfo.key_name;
    }
    return concatenatedKeys.find(word) != string::npos;
}

// THREAD function to detect keywords
void detection_keystroke_words() {
    
    vector<string> keyWords = {
        "POWERSHELL",
        "CMD.EXE",
        "USER",
        "NEW-OBJECT"
        };

    while (!exitProgram) {
        //cout << "[*] Keystroke Words Check Engaged" << endl;
        while (!keystroke_words) {
            for (const auto& word : keyWords) {
                if (checkForWord(*keylogged, word)) {
                    cout << "[*] Key Words Detected: [" << word << "]" << endl;
                    
                    // Set Keystroke flag to true if a word is found
                    keystroke_words = true;

                    // Report to eventvwr
                    const wchar_t* eventmessage = L"Abnormal keystroke Word detected!";
                    report_event(eventmessage, EVENTLOG_WARNING_TYPE);
                }
            }
            this_thread::sleep_for(chrono::seconds(1));
        }
    this_thread::sleep_for(chrono::seconds(1));
    }
}

// Function to Get String of Keylogged
string get_keylogged(const vector<KeyInfo>& key_buffer) {
    string klogged;
    for (const KeyInfo& key_info : key_buffer) {
        klogged += "[";
        klogged += key_info.key_name;
        klogged += "]";
    }
    return klogged;
}

// Print the Key_buffer content
void print_keylogged(const vector<KeyInfo>& key_buffer) {
    // Print buffer information
    cout << "Key buffer:" << endl;
    for (const KeyInfo& key_info : key_buffer) {
        cout << "Key: " << key_info.key_name.c_str() << ", Elapsed time : " << key_info.elapsed_time << endl;
    }
    
    cout << "Key_captured: " << endl;
    for (const KeyInfo& key_info : key_buffer) {
        cout << "[" << key_info.key_name.c_str() << "]";
    }
}

// Function to Detect Keystroke Speed
bool detection_keystroke_speed(double keylogged_average) {
    // limit to 1 detection
    if (!keystroke_speed) {
        // Verify if speed of keystroke is normal
        if (keylogged_average < 0.026 && keylogged_average != -1) {
            cout << "[*] Abnormal keystroke speed detected!" << endl;
            keystroke_speed = true;
            const wchar_t* eventmessage = L"Abnormal keystroke speed detected!";

            // Could return the Keylogged in the Event but could be dangerous to leak sensitive input

            report_event(eventmessage, EVENTLOG_WARNING_TYPE);
            return true;
        }
        else {
            return false;
        }
    }
}

// THREAD Function to list keyboards (Generated with chatGPT)
void detect_new_keyboards() {
    UINT numDevices;
    GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
    PRAWINPUTDEVICELIST pOldRawInputDeviceList = new RAWINPUTDEVICELIST[numDevices];
    GetRawInputDeviceList(pOldRawInputDeviceList, &numDevices, sizeof(RAWINPUTDEVICELIST));

    //list Actual Keyboards
    vector<wstring> oldDevices;
    for (UINT i = 0; i < numDevices; i++) {
        if (RIM_TYPEKEYBOARD == pOldRawInputDeviceList[i].dwType) {
            UINT cbSize = 0;
            GetRawInputDeviceInfo(pOldRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, NULL, &cbSize);
            if (cbSize > 0) {
                LPWSTR deviceName = new WCHAR[cbSize];
                if (GetRawInputDeviceInfo(pOldRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, deviceName, &cbSize) > 0) {
                    wstring device(deviceName);
                    oldDevices.push_back(device);
                }
                delete[] deviceName;
            }
        }
    }
    // Show Actual Keyboard
    if (verbose) {
        wcout << "[*] Keyboard found : " << oldDevices.size() << endl;
        for (const auto& device : oldDevices) {
            wcout << device << endl;
        }
    }

    // Loop to check for NEW keyboards
    while (!exitProgram) {
        PRAWINPUTDEVICELIST pNewRawInputDeviceList;
        UINT numNewDevices;
        GetRawInputDeviceList(NULL, &numNewDevices, sizeof(RAWINPUTDEVICELIST));
        pNewRawInputDeviceList = new RAWINPUTDEVICELIST[numNewDevices];
        GetRawInputDeviceList(pNewRawInputDeviceList, &numNewDevices, sizeof(RAWINPUTDEVICELIST));
        vector<wstring> newDevices;
        for (UINT i = 0; i < numNewDevices; i++) {
            if (RIM_TYPEKEYBOARD == pNewRawInputDeviceList[i].dwType) {
                UINT cbSize = 0;
                GetRawInputDeviceInfo(pNewRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, NULL, &cbSize);
                if (cbSize > 0) {
                    LPWSTR deviceName = new WCHAR[cbSize];
                    if (GetRawInputDeviceInfo(pNewRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, deviceName, &cbSize) > 0) {
                        wstring device(deviceName);
                        newDevices.push_back(device);
                    }
                    delete[] deviceName;
                }
            }
        }
        delete[] pNewRawInputDeviceList;

        // Loop to detection new device
        for (const auto& newDevice : newDevices) {
            if (find(oldDevices.begin(), oldDevices.end(), newDevice) == oldDevices.end()) {
                oldDevices.push_back(newDevice);
                new_keyboard_found = true;

                // send Event
                wstring message = L"New keyboard device detected: ";
                message += newDevice;
                report_event(message.c_str(), EVENTLOG_WARNING_TYPE);

                if (verbose) {
                    wcout << L"[*] New keyboard device detected: " << newDevice << endl;
                    cout << "[*] Keyboard detected : " << newDevices.size() << endl;
                }
            }
        }
        // Loop to detecte Keyboard removed
        for (const auto& oldDevice : oldDevices) {
            if (find(newDevices.begin(), newDevices.end(), oldDevice) == newDevices.end()) {
                if (verbose) {
                    wcout << L"[*] Keyboard device removed: " << oldDevice << endl;
                }
                // send Event
                wstring message = L"Keyboard device removed: ";
                message += oldDevice;
                report_event(message.c_str(), EVENTLOG_WARNING_TYPE);

                // remove the removed device from oldDevices vector
                oldDevices.erase(std::remove(oldDevices.begin(), oldDevices.end(), oldDevice), oldDevices.end());
                //new_keyboard_found = false;
            }
        }
        this_thread::sleep_for(chrono::seconds(1));
    }
    if (verbose) {
        cout << endl << "[*] Closing detect_new_keyboards thread" << endl;
    }
}

// Function to calculate the Average Elapsed time from the last 32 entry on the vector
double calculateAverageElapsedTime(int vectorsize) {
    // Make sure there are at least 32 elements in the vector
    if (keylogged->size() < vectorsize) {
        // Return -1 if there are less than 32 elements
        return -1.0;
    }
    double sum = 0.0;
    // Iterate over the last 32 elements of the vector
    for (int i = keylogged->size() - vectorsize; i < keylogged->size(); i++) {
        sum += keylogged->at(i).elapsed_time;
    }
    // Calculate and return the average elapsed time
    return sum / vectorsize;
}

// Function to transform Scankeys to Strings https://stackoverflow.com/questions/38100667/windows-virtual-key-codes
string VirtualKeyCodeToString(UCHAR virtualKey)
{

    UINT scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);

    CHAR szName[128];
    int result = 0;
    switch (virtualKey)
    {
    case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
    case VK_RCONTROL: case VK_RMENU:
    case VK_LWIN: case VK_RWIN: case VK_APPS:
    case VK_PRIOR: case VK_NEXT:
    case VK_END: case VK_HOME:
    case VK_INSERT: case VK_DELETE:
    case VK_DIVIDE:
    case VK_NUMLOCK:
        scanCode |= KF_EXTENDED;
    default:
        result = GetKeyNameTextA(scanCode << 16, szName, 128);
    }
    return szName;
}




// HOOK Define the hook callback function
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // Set Time_point
    static chrono::time_point<chrono::system_clock> previousTime;

    // Check if the hook is called for a keydown even
    if (nCode == HC_ACTION && (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)) {
        // Get the keycode of the key that was pressed
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
        DWORD vkCode = p->vkCode;

        //DWORD flag = p->flags; // if it's a special key
        //DWORD ctime = p->time; // May also be used to calculate timestamps... but Crono is more accurate
        //DWORD deviceData = p->dwExtraInfo; // Doesn't seem to have anything interesting


        // Escape Killswitch, to remove before Prod
        if (vkCode == VK_ESCAPE) {
            cout << "[*] Quitting application" << endl;
            cout << "average time of last 32 : " << calculateAverageElapsedTime(32) << endl;
            exitProgram = true;
            PostQuitMessage(0);
        }

        // Calculate elapsed time
        chrono::time_point<chrono::system_clock> currentTime = chrono::system_clock::now();
        chrono::duration<double> elapsedSeconds = currentTime - previousTime;
        previousTime = currentTime;

        // Transform virtualkey scancode to string
        string keyname = VirtualKeyCodeToString(vkCode);


        double elapsedtime = elapsedSeconds.count();
        double keylogged_average = calculateAverageElapsedTime(32);

        //verify if speed is normal
        detection_keystroke_speed(keylogged_average);

        // PRINT REPORT and DEBUGGING DATA
        if (verbose) {
            cout << "[*] Key : " << keyname << setw(30 - keyname.length());
            cout << "| Elapsed time : " << elapsedtime << "s  ";
            cout << "\t| Average time of last 32 : " << keylogged_average << "s";
            cout << "\t| S[" << keystroke_speed << "] P[" << keystroke_pattern << "] W[" << keystroke_words << "] K[" << new_keyboard_found << "]" << endl;
        }

        // If Monitoring mode, do not store keynames
        if (monitoring) {
            keyname = "";
        }

        KeyInfo keyInfo = { keyname, elapsedtime };
        keylogged->push_back(keyInfo);

    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {

    // Thread to look for new Keyboards
    thread t_newkeyboard(detect_new_keyboards);
    t_newkeyboard.detach();

    // Thread to reset flag every X minutes (5)
    int reset_timer = 5;
    thread t_resetflag(resetDetectionFlags, reset_timer);
    t_resetflag.detach();

    // Thread to detect keystroke pattern in buffer
    thread t_keypattern(detection_keystroke_pattern);
    t_keypattern.detach();

    // Thread to detect keystroke words in buffer
    thread t_keywords(detection_keystroke_words);
    t_keywords.detach();

    // keylogger buffer size, determined how many keys will be kept in the Vector
    int keylogger_buffer_size = 10000;
    
    // Create a vector object
    vector<KeyInfo> keyPressData;
    keyPressData.reserve(keylogger_buffer_size);

    // Initialize the keyPresses pointer to point to the vector object
    keylogged = &keyPressData;

    // Install the hook
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (hook == NULL) {
        cerr << "Failed to install hook!" << endl;
        return 1;
    }

    // Log event - Service started
    const wchar_t* eventmessage = L"Rubber Duck Hunt - Keystroke Injection Detection Started";
    report_event(eventmessage, EVENTLOG_INFORMATION_TYPE);


    // Run the message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


    // DEBUG Print if detection occured
    if (exitProgram) {
        print_keylogged(*keylogged);
        cout << endl;
        cout << boolalpha;
        cout << "[*] DETECTION SPEED = " << keystroke_speed << endl;
        cout << "[*] DETECTION WORDS = " << keystroke_words << endl;
        cout << "[*] DETECTION PATTERN = " << keystroke_pattern << endl;
        cout << "[*] DETECTION KEYBOARD = " << new_keyboard_found << endl;
        system("pause");
    }


    

    // Uninstall the hook
    UnhookWindowsHookEx(hook);

    return 0;
}

