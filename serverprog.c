/* server.c */
#include "inet.h"
#include <stdbool.h>
#define BUFSIZE 1024

main()
{
    int sockfd,new_sockfd,clilen;
    char buffer[BUFSIZE+1];
    struct sockaddr_in serv_addr,cli_addr;
    int byte_rcv = 0;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Server: socket() error\n");
        exit(1);
    }
    printf("\nFile Repository Server\n");

    /*getting server's IP address */
    FILE *fp_svr;
	char retData[64];
	char *tkn;
	char retChar[1000];
	fp_svr = popen("/sbin/ifconfig eth0", "r");
	while (fgets(retData, 64, fp_svr) != NULL)
	{
		/* store the string in retData into variable retChar */
		strcat(retChar, retData);
	}

	/* get the first token and walk through other tokens
	   until the IP address token */
	tkn = strtok(retChar, " ");
	tkn = strtok(NULL, " ");
	tkn = strtok(NULL, " ");
	tkn = strtok(NULL, " ");
	tkn = strtok(NULL, " ");
	tkn = strtok(NULL, " ");
	tkn = strtok(NULL, " ");
	tkn = strtok(NULL, " addr:");

	/* print the IP address token */
	printf("Server IP address: %s\n", tkn);

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;//
    serv_addr.sin_addr.s_addr = inet_addr(tkn);
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    pclose(fp_svr);

    if(bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Server: bind() error\n");
    }
    printf("\nWaiting for connection... [bind]\n");

    /* Get the user name */
	char *bfr;
	bfr = (char *)malloc(10*sizeof(char));
	bfr = getlogin();

	/* set the 'server_file' path */
	char file_path[30];
	strcpy(file_path, "/home/");
	strcat(file_path, bfr);
	strcat(file_path, "/server_file/");

	/* Check the path exist or not, if not, create one */
	struct stat sts;
	if(stat(file_path, &sts) == -1)
    {
        mkdir(file_path, 0700);
    }

    listen(sockfd,5);
    int pipe1[2];
	if(pipe(pipe1) == -1)
	{
		perror("Fail to create pipe");
		exit(1);
	}

    for(;;)
    {
        clilen = sizeof(cli_addr); //
        new_sockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);

        char *clt_add = inet_ntoa(cli_addr.sin_addr);
        char *idv_address; /* individual address of the client */
        write(pipe1[1], &clt_add, sizeof(clt_add));

        if(fork() == 0)
        {
            close(sockfd);
            strcpy(buffer,"** Welcome to the file repository server. ** \n\nPress\n1.Download File\t\t2.Send File\n3.Create Directory\t4.Delete Directory(include sub directory)\n[type /q to quit] : ");
            send(new_sockfd, buffer, BUFSIZE, 0);

            if(new_sockfd > 0)
                read(pipe1[0],&idv_address,sizeof(idv_address));
                printf("\nClient %s connected now.\n", idv_address);

            do
            {
                recv(new_sockfd, buffer, BUFSIZE, 0);

                if(!strcmp(buffer, "1"))
                {
                    bzero( buffer, sizeof(buffer));
                    strcat(buffer,"[Below is the List of files in Server Directory]\nPlease select a file from the list to be downloaded:\n\n");

                    DIR *dir;
                    struct dirent *ent;

                    char dir_name[30];
                    strcpy(dir_name, "/home/");
                    strcat(dir_name, bfr);
                    strcat(dir_name, "/server_file/");

                    if ((dir = opendir (dir_name)) != NULL)
                    {
                        /*print all the files and directories within directory */
                        while ((ent = readdir (dir)) != NULL)
                        {
                            strcat(buffer, ent->d_name);
                            strcat(buffer, "\n");
                        }
                        closedir (dir);
                        send(new_sockfd, buffer, BUFSIZE, 0);

                    }

                    else
                    {
                        /*error for could not open directory*/
                        perror ("Directory not exist.");
                        return EXIT_FAILURE;
                    }

                    bool exist = true;

                    do
                    {
                        bzero( buffer, sizeof(buffer));
                        recv(new_sockfd, buffer, BUFSIZE, 0);

                        char file_name[30];
                        strcpy(file_name, "/home/");
                        strcat(file_name, bfr);
                        strcat(file_name, "/server_file/");
                        strcat(file_name, buffer);

                        FILE *fp1 = fopen(file_name, "r");

                        if(fp1 == NULL)
                        {
                            strcpy(buffer," There is no such file in server.\nPlease key in the correct filename with extension.\n");
                            send(new_sockfd, buffer, BUFSIZE, 0);
                            exist = false;
                        }

                        if(exist == true )
                        {
                            bzero( buffer, sizeof(buffer));
                            int nread = fread(buffer,1,256,fp1);
                            send(new_sockfd, buffer, nread, 0);
                        }

                        bzero( buffer, sizeof(buffer));
                        strcpy(buffer,"Sucessfully Downloaded. [/q to quit]");

                    }while(exist == false);
                }

                if(!strcmp(buffer, "2"))
                {

                    bzero( buffer, sizeof(buffer));
                    recv(new_sockfd, buffer, BUFSIZE, 0);

                    char file_name[30];
                    strcpy(file_name, "/home/");
                    strcat(file_name, bfr);
                    strcat(file_name, "/server_file/");
                    strcat(file_name, buffer);


                    FILE *fp2;
                    fp2 = fopen(file_name, "ab");
                    bzero( buffer, sizeof(buffer));
                    byte_rcv = recv(new_sockfd, buffer, BUFSIZE, 0);
                    fwrite(buffer,1,byte_rcv,fp2);
                    fclose(fp2);

                    bzero( buffer, sizeof(buffer));
                    strcat(buffer,"Sucessfully Sent. [/q to quit]");
                }

                if(!strcmp(buffer, "3"))
                {
                    bzero( buffer, sizeof(buffer));
                    strcat(buffer,"Sucessfully Created Directory. [/q to quit]");
                }

                if(!strcmp(buffer, "4"))
                {
                    bzero( buffer, sizeof(buffer));
                    strcat(buffer,"Sucessfully Deleted Directory. [/q to quit]");
                }
                send(new_sockfd, buffer, BUFSIZE, 0);

            }while (strcmp(buffer, "/q"));

            printf("\nClient %s disconnected now.\n", idv_address);
            exit(0);
        }
        close(new_sockfd);
    }
    close(sockfd);
}
