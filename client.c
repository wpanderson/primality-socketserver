#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

bool determinePrime(int x, int y)
{
    int test = x % y;
    if (test == 0)
    {
        return true;
    } else
        return false;
}

int main(int argc, char *argv[])
{
    int sockfd, portnumber, return_status;
    int prime_number = 0;
    int received_number = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //char buffer[256];
    if (argc < 3) {
        fprintf(stderr,"usage %s <hostname> <port_number>\n", argv[0]);
        exit(0);
    }
    portnumber = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //Check Socket
    if (sockfd < 0)
        error("ERROR opening socket\n");
    server = gethostbyname(argv[1]);

    //Check server name
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    //Initialize server connection properties
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portnumber);

    //Error check for connection
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    printf("Connected to server... beginning prime evaluation. \n");

    //Read from socket and get our prime number.
    return_status = read(sockfd, &prime_number,sizeof(prime_number));
    if (return_status < 0) //check for no response
    {
        error("ERROR couldn't read from socket.");
        close(sockfd);
        exit(1);
    }
    prime_number = ntohl(prime_number);
    printf("The prime number %d\n", prime_number);
    bool check_prime = false;

    int response = htonl(1);
    //return_status = write(sockfd, &response, sizeof(response));
    while(!check_prime)
    {
        //retrieve divisibility number
        return_status = read(sockfd, &received_number, sizeof(received_number));
        if (return_status < 0)
        {
            error("Could not receive number.");
            close(sockfd);
        }

        //determine if the prime is divisible by it.
        check_prime = determinePrime(prime_number, ntohl(received_number));

        //respond to server with the result
        if (check_prime)
        {
            printf("%d is divisible by %d\n",prime_number, ntohl(received_number));
            response = htonl(1);
        }
        else if(!check_prime)
        {
            response = htonl(0);
        }

        //Respond to the server with the response
        return_status = write(sockfd, &response, sizeof(response));
    }
    close(sockfd);
    exit(1);
}