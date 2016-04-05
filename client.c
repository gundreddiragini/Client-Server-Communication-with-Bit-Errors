#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

//variable declaration
int sequ=0;          //temorary variable that is used for some purpose in the program
int createError=0;    //"createError" is used to create a random situation where a packet is recieved with a bit error
char seq='0';         //"seq" is used to indicate the sequence number that is expected next


/*Main Function*/
int main(int argc,char *argv[])
{

if(argc>1)
{
}


//Variable declaration
int sockfd = 0;            // for storing the socket id of the socket that is created
int bytesReceived = 0;     //for storing the number of bytes that are recieved from the server
char recvBuff[256];        //for storing the recieved data from the server
memset(recvBuff, '0', sizeof(recvBuff));
struct sockaddr_in serv_addr;    // structure for storing the details of the port to bind a socket


/* Create a socket first */
if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
{
printf("\n Error : Could not create socket \n");
return 1;
}


/* Initialize sockaddr_in data structure */
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(5000);        // port
serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


/* Attempt a connection for server*/
if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
{
printf("\n Error : Connect Failed \n");
return 1;
}
printf("Connection with server successfull \n");


/* Create file where data will be stored */
FILE *fp;
fp = fopen(argv[1], "ab"); 
if(NULL == fp)
{
printf("Error opening file");
return 1;
}


/* Receive data from the server */
while((bytesReceived = read(sockfd, recvBuff, 10)) > 0)
{
if(recvBuff[9]==seq)               //Check whether the desired sequence is recieved or not
{
unsigned int buffer[8]={0};
int i,j=0;
char checksum;           //"checksum" is used for verifying the checksum of the recieved packet (by calculating the expected checksum value)


/*verifying the checksum of the packet that is recieved */
for(i=0;i<=7;i++)
buffer[i]=(int) recvBuff[i];
for(i=0;i<=7;i++)
{
if(buffer[i]==49)
++j;
}
if(j%2==0)
checksum='0';          //checksum
else
checksum='1';


/*printing the obtained sequence in the packet onto the console */
printf("Recieved Sequence is %c\n",recvBuff[9]);
printf("Recieved data is:  ");
for(i=0;i<=9;i++)
{
printf("%c ",recvBuff[i]); 
} 
printf("\n");


//condition for creating a scenario where packet is recieved with a bit error (by manipulating the expected checksum value)
if (createError==1 && recvBuff[9]=='1')  
{
if(checksum=='0')
checksum='1';
else
checksum='0';
}


/* Checking whether the packet is recieved correctly or not*/
if(checksum==recvBuff[8])                   //situation where the packet is recieved correctly
{
fwrite(recvBuff, 1,8,fp);                   // write the obtained data onto the output file
printf("Acknowledgement sent for sequence %c\n",seq);
sequ=sequ^1;
if(sequ==1)
seq='1';                                  //next sequence number that is expected
else
seq='0';


char acknowledge[3]={recvBuff[9],'1',seq};                //(((sequence number for which the acknowledgement is being sent, '1' indicates that packet is reached correctly, next expected sequence number)))

//Send acknowledgement that packet is reached correctly
if( send(sockfd , acknowledge , strlen(acknowledge) , 0) < 0)
{
puts("Send failed");
return 1;
}
}


/*Situation where the packet is not recieved correctly*/
else
{
printf("Acknowledgement sent for sequence %c\n",seq);
char acknowledge[3]={recvBuff[9],'0',seq};                   //(((sequence number for which the acknowledgement is being sent, '0' indicates that packet is not reached correctly, next expected sequence number)))

//Send acknowledgement that packet is reached wrongly
if( send(sockfd , acknowledge , strlen(acknowledge) , 0) < 0)
{
puts("Send failed");
return 1;
}
} 
createError=createError^1;                        //for creating a situation where packet is recieved with a bit error
}
}


if(bytesReceived < 0)
{
printf("\n Read Error \n");
}


return 0;
}


