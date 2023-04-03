#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>


char pf[100]="/usr/bin:/bin:/sbin";
char *PATH[100];
int cwdd=0;


void tokenizel(char** main,int *size,char *inarr,const char *c){
	char *token;
	token=strtok(inarr,c);
	int count=-1;
	while(token){
		main[++count]=malloc(sizeof(token)+1);
		strcpy(main[count],token);
		char *temp=main[count];
		if(temp[strlen(temp)-1]==' ' || temp[strlen(temp)-1]=='\n')
		temp[strlen(temp)-1]='\0';
		if(temp[0]==' ' || temp[0]=='\n') memmove(temp, temp+1, strlen(temp));
		token=strtok(NULL,c);
	}
	main[++count]=NULL;
	*size=count;
}


void mainexec(char** argv){
	if(fork()==0){
		execvp(argv[0],argv);
		perror("invalid input\n");
		exit(1);
	}
	else{
		wait(NULL);	
	}
}

void pipeexec(char** inarr,int size){

	int fd[15][2];
	int count;
	int temp;
	char *argv[100];

	for(temp=0;temp<size;temp++){
		tokenizel(argv,&count,inarr[temp]," ");  // ls , -l  // head -2
		if(temp!=size-1){
			if(pipe(fd[temp])<0){
				perror("ERROR!\n");
				return;
			}
		}
		if(fork()==0){
			if(temp!=size-1){
				dup2(fd[temp][1],1);
				close(fd[temp][0]);
				close(fd[temp][1]);
			}

			if(temp!=0){
				dup2(fd[temp-1][0],0);
				close(fd[temp-1][1]);
				close(fd[temp-1][0]);
			}
			execvp(argv[0],argv);
			perror("ERROR!");
			exit(1);
		}

		if(temp!=0){
			close(fd[temp-1][0]);
			close(fd[temp-1][1]);
		}
		wait(NULL);
	}
}

void inred(char** inarr,int size){
	int count,fd;
	char *argv[100];
	tokenizel(argv,&count,inarr[0]," ");
	if(fork()==0){
        fd=open(inarr[1],O_RDONLY);
        dup2(fd,0);
        char str1[100]="/bin/";
		strcat(str1,argv[0]);
		execvp(argv[0],argv);
		perror("invalid input ");
		exit(1);
	}
	wait(NULL);
}

void outred(char** inarr,int size){
	int count,fd;
	char *argv[100];
	tokenizel(argv,&count,inarr[0]," ");
	if(fork()==0){
        fd=open(inarr[1],O_WRONLY); 
        dup2(fd,1);
        char str1[100]="/bin/";
		strcat(str1,argv[0]);
		execvp(argv[0],argv);
		perror("invalid input ");
		exit(1);
	}
	wait(NULL);
}

void addhis(char *inarr){
	FILE *filepointer;
   	filepointer = fopen("history.txt", "a");
   	fprintf(filepointer, "%s", inarr);
   	fclose(filepointer);
}

int main()
{
	char inarr[500],*mainarr2[100], *mainarr1[100],currdir[1024];
	int size=0; char prompt[100]=">> "; 
	while(1){
		if (cwdd==1)
        {
            if (getcwd(currdir, sizeof(currdir)) != NULL){
		    strcpy(prompt,currdir);
            strcat(prompt,"$");
            }
	        else perror("getcwd failed\n");
        }
        printf("%s", prompt);

		fgets(inarr, 500, stdin);
		addhis(inarr);

		
		if(strstr(inarr,"history")){
			FILE *filepointer;
    		char c;
    		filepointer = fopen("history.txt", "r");
    		if (filepointer == NULL)
    		{
    		    printf("Cannot open file \n");
    		    exit(0);
    		}
    		c = fgetc(filepointer);
    		while (c != EOF)
    		{
    		    printf ("%c", c);
    		    c = fgetc(filepointer);
    		}
			printf("\n");
    		fclose(filepointer);
		}
		else if(strchr(inarr,'|')){
			tokenizel(mainarr2,&size,inarr,"|");
			pipeexec(mainarr2,size);
		}
		else if(strchr(inarr,'>')){
			tokenizel(mainarr2,&size,inarr,">");
			if(size==2){
				outred(mainarr2,size);
			}
			else printf("ERROR!");
		}
		else if(strchr(inarr,'<')){
			tokenizel(mainarr2,&size,inarr,"<");
			if(size==2){
				inred(mainarr2,size);
			}
			else printf("ERROR!");
		}
		else{
			tokenizel(mainarr1,&size,inarr," ");
			if(strstr(mainarr1[0],"cd")){
				chdir(mainarr1[1]);
			}
            else if(strstr(mainarr1[0],"PS1")){
                char temp[100];
                char *temp3=" ";
                char cw[100]="w$";
                strcpy(prompt, "");
                strcpy(temp, "");
                strcat(temp,&mainarr1[0][5]);
                if (size==1)
                {
                    temp[strlen(temp)-1] = '\0';
                    if (strstr(temp,cw))
                    {
                        cwdd=1;
                        if (getcwd(currdir, sizeof(currdir)) != NULL){
		                strcpy(prompt,currdir);
                        strcat(prompt,"$");
                        }
		                else perror("getcwd failed\n");
                    }
                    else {
                    	cwdd=0;
                        strcpy(prompt,temp);
                        strcat(prompt," ");
                    }
                }
                else{
                	cwdd=0;
                    strcat(temp,temp3);
                    for (int i = 1; i < size; i++)
                    {
                        strcat(temp,mainarr1[i]);
                        strcat(temp,temp3);
                    } 
                    temp[strlen(temp)-2] = '\0';
                    strcat(temp," ");
                    strcpy(prompt,temp);
                }
            }
			else if(strstr(mainarr1[0],"exit")){
				exit(0);
			}
            else if(strstr(mainarr1[0],"PATH")){
                printf("PATH updated!\n");
            }
			else mainexec(mainarr1);
		}
	}

	return 0;
}
