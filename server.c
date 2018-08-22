#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <limits.h>

//Error function for read and writes.
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

//generate a random number based on the port number entered and the current time
int prime(int seed)
{
    srand(seed);
    srand(time(NULL));
    int prime = rand()%INT_MAX;
    //printf("%d", prime);
    return prime;
}

//determine the square root of a prime number and return it
double checkPrime(int prime)
{
    double square;
    double primeNumber = (double) prime;
    square = sqrt(primeNumber);

    return square;
}

//Main server loop
int main(int argc, char *argv[]) {

    //Server socket address, client socket address, and port number
    int sockfd, newsockfd, portno;
    //server init values
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    //Placeholder for the return status of each read and write.
    int return_status;


    if (argc < 2) {
        fprintf(stderr, "ERROR, usage: %s <port_number>\n", argv[0]);
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    //Generate a prime number
    int primeNumber = prime(portno);
    double primeSquare = checkPrime(primeNumber); //Find the square root of that number

    printf("Square root of prime: %f\n", primeSquare);
    printf("Prime number: %d\n", primeNumber);

    //Server properties to allow connections
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    //bind the socket to allow connections
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
        close(sockfd);
        exit(1);
    }
    printf("Waiting for socket connection...\n");
    //wait for connection and limit those connections to 5

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr,
                       &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");

    int response = 0;
    int send_prime = htonl(primeNumber);
    return_status = write(newsockfd, &send_prime, sizeof(send_prime));
    if(return_status > 0)
    {
        printf("Communication has begun, Sending numbers...\n");
    }
    else {
        close(newsockfd);
        close(sockfd);
        exit(1);
    }
    int counter = 2;
    //Iterate while counter is less than srt(prime)
    while(counter < primeSquare)
    {
        //Send counter to client
        int send_counter = htonl(counter);
        return_status = write(newsockfd, &send_counter, sizeof(send_counter));
        if (return_status < 0)
        {
            error("Error encountered while sending counter");
            close(newsockfd);
            close(sockfd);
        }

        return_status = read(newsockfd, &response, sizeof(response));
        if (return_status > 0)
        {
            //printf("%d\n", ntohl(response));
            if (ntohl(response) == 1)
            {
                //Number is prime. Return the number and it's divisibility
                printf("%d is not prime and is divisible by %d", primeNumber, counter);
                close(newsockfd);
                close(sockfd);
                exit(1);
            }
            else {
                counter++;
            }
        }
        else {
            //close(newsockfd);
            //close(sockfd);
            //exit(1);
        }
    }

    close(newsockfd); //Close connection
    close(sockfd); //close server
    return 0;
}