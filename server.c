#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include<time.h>

/*Variable declaration for defining a timer*/
int t;  
time_t start,end;

/*variable declaration*/
unsigned char buff[10]={0};   //Buffer that will be finally transfered to the client     
int z=1;                   //for calculating the sequence number of the packet that is being transfered


/*Main Function*/
int main(int argc,char *argv[])
{
if(argc>1)
{
}


int listenfd = 0;                    // for storing the socket id of the socket that is created
int connfd = 0;                      //for storing the id of incoming connections
struct sockaddr_in serv_addr;         // structure for storing the details of the port to bind a socket


/* Create a socket first */
listenfd = socket(AF_INET, SOCK_STREAM, 0);
printf("Socket retrieve success\n");
printf("Waiting for incoming connections from clients\n");

memset(&serv_addr, '0', sizeof(serv_addr));


/* Initialize sockaddr_in data structure */
serv_addr.sin_family = AF_INET;
serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
serv_addr.sin_port = htons(5000);


/*Binding the socket to the port*/
bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
if(listen(listenfd, 10) == -1)
{
printf("Failed to listen\n");
return -1;
}


while(1)
{
/*For accepting the connections that are obtained form clients*/
connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);
printf("A connection from a client is accepted\n");


/* Open the file that we wish to transfer */
FILE *fp = fopen(argv[1],"r");
if(fp==NULL)
{
printf("File opern error");
return 1;   
}   

       
/* Read data from file and send it */
while(1)
{
/* First read file in chunks of 8 bytes */
unsigned char bufff[8]={0};           //temporary buffer for reading data from the file
int nread = fread(bufff,1,8,fp);

int q=10;                 //'q' indicates the size of the buffer that is being transfered to the client
char client_message[3];   //variable for recieving the client acknowledgement


/*Condition where reading from a file is failed*/
if (nread < 8)        
{
if (feof(fp))
printf("End of file\n");
if (ferror(fp))
printf("Error reading\n");
break;
}

//Temporary variabled declaration
int j=0;
int i;

/*Transferring data from the temporary buffer into the buffer that will be transmitted*/
for(i=0;i<=7;i++)
{
buff[i]=bufff[i];
} 

/*CHECK SUM CALCULATION*/
unsigned int buffer[8]={0};
for(i=0;i<=7;i++)
buffer[i]=(int) buff[i];
for(i=0;i<=7;i++)
{
if(buffer[i]==49)
++j;
}
if(j%2==0)
buff[8]='0';    //Storing the calculated checksum into the buffer
else
buff[8]='1';
j=0;


z = z ^ 1;     // Calculating sequence number

/*Storing the sequence number into the buffer*/
if(z==1)
buff[9]='1';
else
buff[9]='0';

/* If read was success, send data. */
if(nread > 0)
{
printf("Sending sequence=%c \n",buff[9]);
write(connfd, buff, q);
}

/*
* There is something tricky going on with read .. 
* Either there was error, or we reached end of file.
*/
if (nread < 8)
{
if (feof(fp))
printf("End of file\n");
if (ferror(fp))
printf("Error reading\n");
break;
}

/*Wait for certain time before expecting an acknowledgement*/
do
{
start=clock();      //predefined  function in c
do
{
end=clock();
t=(end-start)/CLOCKS_PER_SEC;
}while(t<2);
printf("completed waiting for 2 seconds to recieve client acknowledgement\n");

/*Recieving acknowledgement from the client*/
if(recv(connfd , client_message , 3 , 0) >= 0)
{
printf("Client acknowledgement is recieved for sequence %c \n",client_message[0]);

/*If client indicated that it recieved the packet correctly */
if(client_message[1]=='1')   
{
printf("client is stating that it recieved the packet correctly\n");
printf("It is expecting sequence %c now\n",client_message[2]);
}

/*If client indicated that it recieved the packet with bit error*/
else      
{
printf("client is stating that it recieved the packet with a bit error\n");
printf("It is expecting sequence %c now\n",client_message[2]);
}

if(client_message[1]=='0')   //Client indidcated that it recieved the packet with bit error
{
printf("client is expecting the same packet again\n");
printf("Again sending the packet\n");
write(connfd, buff, q);
printf("Sending sequence=%c \n",buff[9]);
}
}

}while(client_message[1]=='0');  //repeat the same packet transmission until the client indicate that it recieved the packet correctly

}

close(connfd);
sleep(1);
}

return 0;
}
