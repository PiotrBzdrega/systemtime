#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <Systime.hpp>
#include <thread>
#include <chrono>
 
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

 void tt(Systime &obj)
 {
    while (true)
    {
        obj.add(60);
        SetSystemTime(&obj);
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
 }

int main()
{ 
    Systime newTime;
    GetLocalTime(&newTime); // Get the current system time as a starting point
    std::cout << "Local time has been read:" <<newTime<<"\n";

    // HANDLE Hprocess = OpenProcess(
    //         SYNCHRONIZE | PROCESS_QUERY_INFORMATION,  // The access to the process object 
    //         FALSE,                                    // If this value is TRUE, processes created by this process will inherit the handle
    //         GetCurrentProcessId()                     // The identifier of the local process to be opened
    //         );

    HANDLE hToken; 
    /* Get a token for this process. */
    if (!OpenProcessToken(
        GetCurrentProcess(),                             // A handle to the process whose access token is opened
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,           // Specifies an access mask that specifies the requested types of access to the access token
        &hToken                                          // A pointer to a handle that identifies the newly opened access token when the function returns
        ))
    {
        printf ("OpenProcessToken error: %u\n", GetLastError());
        return( FALSE ); 
    } 

#pragma region TokenInfo

// #define TOKEN_INFO

#ifdef TOKEN_INFO
    /* Get the size of the token information */ 
    DWORD dwSize = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);

    /* Allocate memory for the token information */
    PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(dwSize);

    if (!pTokenUser) {
        std::cerr << "Failed to allocate memory for token information." << std::endl;
        CloseHandle(hToken);
        return false;
    }

    /* Retrieve the token information */ 
    if (!GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize)) {
        std::cerr << "Failed to get token information. Error code: " << GetLastError() << std::endl;
        free(pTokenUser);
        CloseHandle(hToken);
        return false;
    }    

    free(pTokenUser); 

#endif
#pragma endregion

    /* access token contains the security information fo a logon session */
    TOKEN_PRIVILEGES tp;

    /* locally unique identifier  A 64-bit value that is guaranteed to be unique on the operating system that generated it until the system is restarted. */
    LUID PrivilegeRequired ;

    /* determine the LUID on the local system */
    if(!LookupPrivilegeValue(
        nullptr,                                         // lookup privilege on local system
        SE_SYSTEMTIME_NAME,                              // privilege to lookup 
        &PrivilegeRequired                               // receives LUID of privilege
        ))
    {
        printf("LookupPrivilegeValue error: %u\n", GetLastError() );
        CloseHandle(hToken); 
        return FALSE;
    }

    ZeroMemory (&tp, sizeof (tp));
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = PrivilegeRequired;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    /* old token privileges */
    TOKEN_PRIVILEGES oldtp;    

    /* Enable the privilege */
    if ( !AdjustTokenPrivileges(
           hToken,                      // A handle to the access token that contains the privileges to be modified
           FALSE,                       // Specifies whether the function disables all of the token's privileges
           &tp,                         // A pointer to a TOKEN_PRIVILEGES structure that specifies an array of privileges and their attributes
           sizeof(TOKEN_PRIVILEGES),    // Specifies the size, in bytes, of the buffer pointed to by the PreviousState parameter
           (PTOKEN_PRIVILEGES) nullptr,    // A pointer to a buffer that the function fills with a TOKEN_PRIVILEGES structure 
           (PDWORD) nullptr                      // A pointer to a variable that receives the required size, in bytes, of the buffer pointed to by the PreviousState parameter
           ) )             
    { 
        printf("AdjustTokenPrivileges error: %u\n", GetLastError() );
        CloseHandle(hToken);           
        return FALSE; 
    } 

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        printf("The token does not have the specified privilege. \n");
        CloseHandle(hToken);
        return FALSE;
    } 

    SYSTEMTIME a{};
    a.wYear = 2024;
    a.wMonth = 2;
    a.wDay = 25;
    a.wHour = 15;
    a.wMinute = 30;
    a.wSecond = 0;

    // newTime(a);

    //newTime.add(2629800);

    // Set the system time
    if (SetSystemTime(&newTime)) {
        std::cout << "System time has been successfully set to:" <<newTime<< "\n";
    } else {
        std::cerr << "Failed to set system time. Error code: " << GetLastError() << "\n";
        CloseHandle(hToken);           
        return FALSE; 
    }

    std::thread time_travel(tt,std::ref(newTime));
    time_travel.join();
    while(true){
        
    };

        tp.Privileges[0].Attributes = 0;

            /* Disable privilege */
    if ( !AdjustTokenPrivileges(
           hToken,                      // A handle to the access token that contains the privileges to be modified
           FALSE,                       // Specifies whether the function disables all of the token's privileges
           &tp,                         // A pointer to a TOKEN_PRIVILEGES structure that specifies an array of privileges and their attributes
           sizeof(TOKEN_PRIVILEGES),    // Specifies the size, in bytes, of the buffer pointed to by the PreviousState parameter
           (PTOKEN_PRIVILEGES) NULL,    // A pointer to a buffer that the function fills with a TOKEN_PRIVILEGES structure 
           (PDWORD) NULL) )             // A pointer to a variable that receives the required size, in bytes, of the buffer pointed to by the PreviousState parameter
    { 
        printf("AdjustTokenPrivileges error: %u\n", GetLastError() ); 
        CloseHandle(hToken);
        return FALSE; 
    } 

    CloseHandle(hToken);
}
