// (c) Nikolay Prodanov, January 2013, Juelich, Germany

//Required headers
#include <memory.h>
#include <stdlib.h>      // atoi()
#include <stdio.h>
#include <unistd.h>      // close()
#include <sys/socket.h>
#include <netinet/in.h>  // sockaddr_in
#include <arpa/inet.h>   // inet_addr

#define BUF_SIZE 256

// Contains the required data
typedef struct _TimeData{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
} TimeData;

///////////////////////////////////////////////////////////////////////////////
// Helper functions
int ConnectAndGetData(TimeData *td);
int ParseTimeString(TimeData *td, char* buf);
int CheckFirstEl(char *buf);

// Globals
char *g_szServer[] = {"207.200.81.113", "64.113.32.5", "64.147.116.229", "64.90.182.55",
                      "96.47.67.105", "165.193.126.229", "206.246.122.250", "64.250.177.145"};
unsigned int g_port = 13;   // Port number
int g_numOfServers = 8;
int g_iError = 0;
TimeData td;

// Exported funcitons
void getyear_(int *year);
void getmonth_(int *month);
void getday_(int *day);
void gethour_(int *hour);
void getminute_(int *min);
void getsecond_(int *sec);

// Connects to one of the servers, gets and parses the string with the date and time
// returns 0 if success
// returns -1 if socket problem
// returns -2 if cannot connect to the server
// returns -3 if connection is closed
// returns -4 if recv failed
// returns -5 if parsing error
int ConnectAndGetData(TimeData *td)
{
	char recvbuf[BUF_SIZE];
	int recvbuflen = BUF_SIZE;
        int iResult;
	// Create a SOCKET for connecting to server
        int ConnectSocket;
	ConnectSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ConnectSocket < 0) {
		//printf("Error at socket(): %ld\n", WSAGetLastError());
		g_iError = -1;
		return g_iError;
	}
	//printf("Socket %i created. \n", ConnectSocket);	

	//----------------------
	// Try connecting to one of the servers
	int i = 0;
	for(i=0; i < g_numOfServers; i++){
		// The sockaddr_in structure specifies the address family,
		// IP address, and port of the server to be connected to.
		struct sockaddr_in clientService; 
		clientService.sin_family = AF_INET;
		clientService.sin_addr.s_addr = inet_addr( g_szServer[i] );
		clientService.sin_port = htons( g_port );

		//----------------------
		// Connect to server.	
		if ( connect( ConnectSocket, (struct sockaddr*) &clientService, sizeof(clientService) ) 
			< 0) {
				if(i < g_numOfServers - 1)
					continue;
				else{
					//printf( "Failed to connect.\n" );
					close(ConnectSocket);
					g_iError = -2;
					return g_iError;
				}
		}// end if connect

        //printf("Connected to server %s.\n", g_szServer[i]);

        // Cleanup the buffer.
        memset(recvbuf, 0, BUF_SIZE*sizeof(char));
        // Receive the data.
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 ){
            //printf("\rBytes received: %d\n", iResult);
            //printf("\r%s\n", recvbuf);

            // Parse the string
            iResult = ParseTimeString(td, recvbuf);
            if(iResult < 0)	// if parsing error
                g_iError = -5;
        }
        else if ( iResult == 0 )
            //printf("Connection closed.\n");
            g_iError = -3;
        else
            //printf("recv failed: %d\n", WSAGetLastError());
            g_iError = -4;

        // If connected, then break the loop.
        if(g_iError >= 0)
            break;
    }// end for i

    close(ConnectSocket);

    return g_iError;
}

// Parse time string
// Its format is 
// 56319 13-01-27 21:16:34 00 0 0 268.5 UTC(NIST) * 
// returns -1 in case of error
int ParseTimeString(TimeData *td, char* buf)
{
	char szTmp[5];
	int iError = 0;
	memset(szTmp, 0, 5);

	int i = 0, j = 0, k = 0;

	// Skip the first number
	while(buf[i] != ' '){
		++i;
		// Prevent infinite loops
		if(BUF_SIZE - 1 == i){
			iError = -1;
			return iError;
		}
	}
	// Skip the space itself
	++i;

	// Get year, month, day
	j = 0;
        k = 0; // number of dividors, e. g. -, or :
	while(buf[i] != ' '){
		szTmp[j] = buf[i];
		++j; ++i;
		if(buf[i] == '-'){
			++i;//skip '-'
			szTmp[j] = '\0';
			j = 0;
			if(k == 0)			
				td->year = atoi(szTmp);				
			else if(k == 1){				
				CheckFirstEl(szTmp);// check whether we have 0 at the beginning
				td->month = atoi(szTmp);				
			}
			++k;
		}// end if
		// Prevent from infinite loop
		if(BUF_SIZE - 1 == k){
			iError = -1;
			return iError;
		}
	}// end while !=' '
	++i;// skip ' '
	++j;
	szTmp[j] = '\0';
	CheckFirstEl(szTmp);// check whether we have 0 at the beginning
	td->day = atoi(szTmp);

	// Get hour, minutes, seconds
	j = 0;
	j = 0;
        k = 0; // number of dividors, e. g., -, or :
	
	while(buf[i] != ' '){
		szTmp[j] = buf[i];
		++j; ++i;
		if(buf[i] == ':'){
			++i;//skip ':'
			szTmp[j] = '\0';
			j = 0;
			if(k == 0){
				CheckFirstEl(szTmp);// check whether we have 0 at the beginning
				td->hour = atoi(szTmp);
			}
			else if(k == 1){				
				CheckFirstEl(szTmp);// check whether we have 0 at the beginning
				td->minute = atoi(szTmp);				
			}
			++k;
		}// end if
		// Prevent from infinite loop
		if(BUF_SIZE - 1 == k){
			iError = -1;
			return iError;
		}
	}// end while !=' '
	++i;// skip ' '
	++j;
	szTmp[j] = '\0';
	CheckFirstEl(szTmp);// check whether we have 0 at the beginning
	td->second = atoi(szTmp);

	return iError;
}

int CheckFirstEl(char *szTmp){
	if(szTmp[0] == '0'){// skip 0 at the beginning of numbers
		szTmp[0] = szTmp[1];
		szTmp[1] = '\0';
	}
	return 0;
}

// This functions to export into the dll.
void getyear_(int *year)
{
	int err = ConnectAndGetData(&td);
        if(err < 0) // check for errors
		*year = err;
	else if(g_iError != 0)
		*year = g_iError;
        else
  		*year = td.year;
}
void getmonth_(int *month)
{
	if(g_iError != 0)
		*month = g_iError;
        else
		*month = td.month;
}

void getday_(int *day)
{
	if(g_iError != 0)
		*day = g_iError;
 	else
		*day = td.day;
}

void gethour_(int *hour)
{
	if(g_iError != 0)
		*hour = g_iError;
	else
		*hour = td.hour;
}

void getminute_(int *min)
{
	if(g_iError != 0)
		*min = g_iError;
	else
		*min = td.minute;
}

void getsecond_(int *sec)
{
	if(g_iError != 0)
		*sec = g_iError;
	else
		*sec = td.second;
}
