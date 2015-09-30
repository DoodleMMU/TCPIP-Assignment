/* client.c */
#include "inet.h"
#define BUFSIZE 1024

main(int agrc, char *argv[])
{
    int sockfd;
    char buffer[BUFSIZE+1];
    struct sockaddr_in serv_addr;
    int bytereceive = 0;
    char create_name[30];
    char delete_name[30];
    static struct sigaction sig_act;

    void catchin(int);

    sig_act.sa_handler = catchin;
    sigfillset(&(sig_act.sa_mask));

    sigaction(SIGINT, &sig_act, (void *) 0);

    if(agrc <= 1)
    {
        printf("How to use : %s remoteIPaddress [example: ./client 127.0.0.1]\n", argv[0]);
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons (SERV_TCP_PORT);
    inet_pton (AF_INET, argv[1], &serv_addr.sin_addr);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Client: socket() error\n");
        exit(1);
    }

    if(connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //
    {
        perror("Client: connect() error\n");
        exit(1);
    }

    /* Get the user name */
	char *bfr;
	bfr = (char *)malloc(10*sizeof(char));
	bfr = getlogin();

	/* set the 'client_file' path */
	char file_path[30];
	strcpy(file_path, "/home/");
	strcat(file_path, bfr);
	strcat(file_path, "/client_file/");

	/* Check the path exist or not, if not, create one */
	struct stat sts;
	if(stat(file_path, &sts) == -1)
    {
        mkdir(file_path, 0700);
    }

    do
    {
        bzero( buffer, sizeof(buffer));
        recv(sockfd, buffer, BUFSIZE, 0);
        printf("\n%s\n", buffer);
        gets(buffer);
        send(sockfd,buffer, BUFSIZE, 0);

        if(!strcmp(buffer, "1"))
        {
            bzero( buffer, sizeof(buffer));
            recv(sockfd, buffer, BUFSIZE, 0);
            printf("\n%s\n", buffer);
            gets(buffer);
            send(sockfd,buffer, BUFSIZE, 0);

            char file_name[30];
            strcpy(file_name, "/home/");
            strcat(file_name, bfr);
            strcat(file_name, "/client_file/");
            strcat(file_name, buffer);

            FILE *fp1;
            fp1 = fopen(file_name, "ab");

            if(NULL == fp1)
            {
                printf("Error opening file");
            }

            bzero( buffer, sizeof(buffer));

            bytereceive = recv(sockfd, buffer, BUFSIZE, 0);
            fwrite(buffer,1,bytereceive,fp1);

        }

        else if(!strcmp(buffer, "2"))
        {
            DIR *dir;
            struct dirent *ent;

            char dir_name[30];
            strcpy(dir_name, "/home/");
            strcat(dir_name, bfr);
            strcat(dir_name, "/client_file/");

            if ((dir = opendir (dir_name)) != NULL)
            {
                printf("\n[List of files in Client Directory]\n");
                /* print all the files and directories within directory */
                while ((ent = readdir (dir)) != NULL)
                {
                    printf("%s\n", ent->d_name);
                }

                closedir (dir);
            }

            printf("\nPlease enter one of the filename from above including extension\n");

            bzero( buffer, sizeof(buffer));
            gets(buffer);
            send(sockfd,buffer, BUFSIZE, 0);

            char file_name[30];
            strcpy(file_name, "/home/");
            strcat(file_name, bfr);
            strcat(file_name, "/client_file/");
            strcat(file_name, buffer);

            FILE *fp2;
            fp2 = fopen(file_name, "r");

            bzero( buffer, sizeof(buffer));
            int nread = fread(buffer,1,256,fp2);
            send(sockfd, buffer, nread, 0);
        }


        else if(!strcmp(buffer, "3"))
        {
            printf("Enter directory name that you want to create: ");
            scanf("%s", create_name);

            /* set the path/name of the directory that want to create */
            char create_dir[30];
            strcpy(create_dir, "/home/");
            strcat(create_dir, bfr);
            strcat(create_dir, "/");
            strcat(create_dir, create_name);

            /* Check the path exist or not, if not, create one */
            struct stat sts;
            if(stat(create_dir, &sts) == -1)
            {
                mkdir(create_dir, 0700);
            }
        }

        else if(!strcmp(buffer, "4"))
        {
            printf("Enter directory name that you want to delete: ");
            scanf("%s", delete_name);

            /* set the path of the directory that want to delete */
            char delete_dir[30];
            strcpy(delete_dir, "/home/");
            strcat(delete_dir, bfr);
            strcat(delete_dir, "/");
            strcat(delete_dir, delete_name);

            /* select all the files inside the directory that want to delete */
            char select_subdir[50];
            strcpy(select_subdir, "exec rm -r ");
            strcat(select_subdir, "/home/");
            strcat(select_subdir, bfr);
            strcat(select_subdir, "/");
            strcat(select_subdir, delete_name);
            strcat(select_subdir, "/*");

            /* Check the path exist or not, if exist, delete it */
            struct stat sts2;
            if(stat(delete_dir, &sts2) != -1)
            {
                system(select_subdir);
                rmdir(delete_dir);
            }
        }

    }while (strcmp(buffer,"/q"));
    close(sockfd);
}

void catchin(int signo)
{
	printf("\n[ Interrupt signal has been ignored.]\n");
}
