#include <stdio.h>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <ctype.h>
#include <iostream>
#define TRACKER_IP "127.0.0.1"
#define TRACKER_PORT 9595
#define CHUNK_SIZE 524288
#define BUFF_SIZE 2048
using namespace std;
struct struct_details{
  string flag;
  string ipaddr;
  string port_no;
  string hashval;
  string groupid;
  string filename;
  string userid;
  string password;
  long long int filesize;
  long long int number_chunks;
  vector<int> chunk_details;
};
struct struct_buff{
  char buff[BUFF_SIZE];
  int filefd;
};
struct struct_thread_dwnld{
  int sock_fd;
  string t_ip;
  int t_port;
  string file_name;
  long long int t_fsize;
  long long int t_number_chunks;
  vector<int> t_chunk_details;
};
struct struct_for_thread{
    string ip;
    string original_ip; ///for sending to tracker
    string original_port; //for sending to tracker
    int port;
    string file_name;
    int chunk_number;
    long long int total_chunks;
    long long int f_size;
};
int login_flag=0;
FILE *fpop;
pthread_mutex_t lock1,lock2;
void *sendtrackerthread(void *);
void *continuouslisten_thread(void *);
void *communicatepeertopeer(void *);
void *listerthread(void *);
void error(const char *msg)
{
    perror(msg);
    exit(0);
}
int connect(string ip,int portno){
  int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFF_SIZE]; 
    char *ipadd=const_cast<char*>(ip.c_str());
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(ipadd);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    return sockfd;

}
void sendtotracker(struct_details *args_details){
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFF_SIZE]; 
    portno = TRACKER_PORT;
    string ip=TRACKER_IP;
    char *ipadd=const_cast<char*>(ip.c_str());
    //cout<<ipadd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(ipadd);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    bzero(buffer,BUFF_SIZE);
          
    char Buffer [ BUFF_SIZE] ;
    string data;
    if(args_details->flag=="1"){
            data=args_details->flag+" "
                +args_details->ipaddr+" "
                +args_details->port_no+" "
                +args_details->filename+" "
                +args_details->groupid;//+"\0";
    }else if(args_details->flag=="2"){
            data=args_details->flag+" "
                +args_details->ipaddr+" "
                +args_details->port_no+" "
                +args_details->filename+" "
                +args_details->groupid;
    }else if(args_details->flag=="3"){
            data=args_details->flag+" "
                +args_details->userid+" "
                +args_details->password;
    }else if(args_details->flag=="4"){
            data=args_details->flag+" "
                +args_details->userid+" "
                +args_details->password;
    }

    //cout<<data<<endl;
    strcpy(Buffer, data.c_str());
    ///////////////// thread create
    struct_buff *threaddata=new struct_buff();
    strcpy(threaddata->buff,Buffer);
    threaddata->filefd=sockfd;
    //cout<<Buffer<<endl;
    send ( sockfd , (const void *)Buffer, BUFF_SIZE, 0); //send the data as request to download or upload at tracker
    if(args_details->flag=="2"){
        char recv_cntbuff[BUFF_SIZE];
        recv(sockfd,recv_cntbuff, BUFF_SIZE, 0);//recv the number of places that file is present
        //cout<<"FILE IS PRESENT AT LOCATIONS:"<<recv_cntbuff<<endl;
        int number_of_places_file_present;
        string tempfileno=recv_cntbuff;
        number_of_places_file_present=stoi(tempfileno);
        vector<struct_thread_dwnld*> vec_roundrobin;
        ////recieve the port and ip of the file
        for(int i=0;i<number_of_places_file_present;i++){
        //cout<<recv_buff;
            char recv_buff[BUFF_SIZE];
            recv(sockfd,recv_buff, BUFF_SIZE, 0);
            char *token;
            token=strtok(recv_buff," ");
            string cntip;
            int cntport;
            int cnt=1,l=1;
            struct_thread_dwnld *thread_dwnld=new struct_thread_dwnld();
            while(token!=NULL){
                if(cnt==1){
                  thread_dwnld->t_ip=token;
                }
                if(cnt==2){
                  thread_dwnld->t_port=stoi(token);
                }
                if(cnt==3){
                    string temptok=token;
                    thread_dwnld->t_fsize=stoll(temptok);
                }
                if(cnt==4){
                    string temptok1=token;
                    thread_dwnld->t_number_chunks=stoll(temptok1);
                    thread_dwnld->t_chunk_details.assign(thread_dwnld->t_number_chunks+1,0);
                }
                if(cnt>=5){
                    string temptok2=token;
                    thread_dwnld->t_chunk_details[l]=stoi(temptok2);
                    l++;
                }
                token=strtok(NULL," ");
                cnt++;
            }
            vec_roundrobin.push_back(thread_dwnld);

        }

        /////////Debug purpose
        /*cout<<"Vector Details for RR"<<endl;
        for(int i=0;i<vec_roundrobin.size();i++){
            cout<<vec_roundrobin[i]->t_ip<<" "
                <<vec_roundrobin[i]->t_port<<" "
                <<vec_roundrobin[i]->t_fsize<<" "
                <<vec_roundrobin[i]->t_number_chunks<<endl;

        }*/

        ////////
        int current_chunk=1;
        int total_chunks=vec_roundrobin[0]->t_number_chunks;
        int file_ind=0;
        pthread_t tid[1000];
        int thd_cnt=0;
        if (pthread_mutex_init(&lock1, NULL) != 0) 
        { 
            printf("\n mutex init has failed\n"); 
            return;
        }
        //////////////
        //FILE *fp2;
        string f1=args_details->filename;
        char temp_arr[100];
        strcpy(temp_arr,f1.c_str());
        fpop=fopen ( temp_arr , "w" );
        
        for(long long int i=0;i<vec_roundrobin[0]->t_fsize;i++)
            fputc('\0',fpop);
        fclose(fpop);
        ////////////////
        while(current_chunk<=total_chunks){
            /*int dwndfd=connect(vec_roundrobin[file_ind]->t_ip,
                                vec_roundrobin[file_ind]->t_port);*/
            struct_for_thread *for_thread=new struct_for_thread();
            for_thread->file_name=args_details->filename;
            if(vec_roundrobin[file_ind]->t_chunk_details[current_chunk]==1){
                //cout<<"Current chunk:"<<current_chunk<<endl;
                for_thread->chunk_number=current_chunk;
                for_thread->total_chunks=vec_roundrobin[file_ind]->t_number_chunks;
                for_thread->f_size=vec_roundrobin[file_ind]->t_fsize;
                for_thread->ip=vec_roundrobin[file_ind]->t_ip;
                for_thread->port=vec_roundrobin[file_ind]->t_port;
                for_thread->original_ip=args_details->ipaddr;
                for_thread->original_port=args_details->port_no;
                pthread_create(&tid[thd_cnt], NULL, communicatepeertopeer, (void *)for_thread);
                pthread_detach(tid[thd_cnt]);
                thd_cnt++;

                current_chunk++;   //increment the chunk when found
            }
            
            file_ind++;
            if(file_ind>=vec_roundrobin.size()){
                file_ind=0;
            }
        }
        /*for(int i=0;i<thd_cnt;i++){
            pthread_join(tid[i], NULL); 
        }*/
        
    }
    else if(args_details->flag=="3"){
        char recv_buff[BUFF_SIZE];
        recv(sockfd,recv_buff, BUFF_SIZE, 0);
        string checkif=recv_buff;
        if(checkif.compare("Yes")==0){
            login_flag=1;
        }

    }
    close(sockfd);
}
void *communicatepeertopeer(void *for_thread){
//pthread_mutex_lock(&lock1);////adding thread lock 
    struct_for_thread *thread=(struct_for_thread*) for_thread;
    int dwndfd=connect(thread->ip,
                                thread->port);
    char Buffer[BUFF_SIZE];
    string data=thread->file_name+" "
                +to_string(thread->chunk_number)+" "
                +to_string(thread->total_chunks)+" "
                +to_string(thread->f_size);
    strcpy(Buffer,data.c_str());


    send ( dwndfd , (const void *)Buffer, BUFF_SIZE, 0);////sending file name and chunk number as request
    ///////////////debugging purpose
        FILE *fp1;
        fp1=fopen ( "datasentonthread.txt" , "a" );
        fprintf(fp1, "%s\n", Buffer);
        fclose(fp1);
    ////////////////////



    /////////////////FILE RECV
            //string fname=thread->file_name;
            char fname[100];
            strcpy(fname,thread->file_name.c_str());
            FILE *fp = fopen ( fname , "r+" );
            char Buffer1 [ BUFF_SIZE];
            int file_size;
            if(thread->chunk_number!=thread->total_chunks) 
                file_size=CHUNK_SIZE;
            else if(thread->chunk_number==thread->total_chunks && 
                    thread->f_size%CHUNK_SIZE!=0){                
                    file_size=thread->f_size%CHUNK_SIZE;
            }else{
                file_size=CHUNK_SIZE;
            }
            rewind(fp);
            fseek(fp,(thread->chunk_number-1)*CHUNK_SIZE,SEEK_SET);
            ///////////////debugging purpose
                    FILE *fp2;
                    fp2=fopen ( "writechunksize.txt" , "a" );
                    fprintf(fp2, "%d %d %d\n", file_size,thread->chunk_number,(thread->chunk_number-1)*CHUNK_SIZE);
                    fclose(fp2);
            ////////////////////
            int n;
            recv(dwndfd, &file_size, sizeof(file_size), 0);//recieve the file
            while ( ( n = recv( dwndfd , Buffer1,BUFF_SIZE, 0) ) > 0  &&  file_size > 0){
                fwrite (Buffer1 , sizeof (char), n, fp);
 
                memset ( Buffer1 , '\0', BUFF_SIZE);
                file_size = file_size - n;
                
            }   
            fclose ( fp );

    ///////////////////////

            ////send the chunk info to tracker for updating the chunk details
            struct_details *senddwnldinfo=new struct_details();
            senddwnldinfo->flag="5";
            senddwnldinfo->ipaddr=thread->original_ip;
            senddwnldinfo->port_no=thread->original_port;
            senddwnldinfo->filename=thread->file_name;
            string datasent=senddwnldinfo->flag+" "
                            +senddwnldinfo->ipaddr+" "
                            +senddwnldinfo->port_no+" "
                            +senddwnldinfo->filename+" "
                            +to_string(thread->chunk_number)+" "
                            +to_string(thread->f_size)+" "
                            +to_string(thread->total_chunks);

            //put in thread
            pthread_mutex_lock(&lock1);////adding thread lock
            char buff1[BUFF_SIZE];
            memset ( buff1 , '\0', BUFF_SIZE);
            int trackerfd=connect(TRACKER_IP,
                                TRACKER_PORT);
            strcpy(buff1,datasent.c_str());
            send ( trackerfd , (const void *)buff1, BUFF_SIZE, 0);
            ///////////////debugging purpose
            FILE *fp3;
            fp3=fopen ( "trackeratend.txt" , "a" );
            fprintf(fp3, "%s\n", buff1);
            fclose(fp3);
            ////////////////////
            pthread_mutex_unlock(&lock1);
            ///////////////////////////////////////////


    close(dwndfd);
    pthread_exit(NULL);
}

void *continuouslisten_thread(void *args_details){
    struct_details *arg_details=(struct_details*) args_details;
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    
    struct sockaddr_in serv_addr, cli_addr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = stoi(arg_details->port_no);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr =inet_addr(arg_details->ipaddr.c_str());
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
    listen(sockfd,5);
    pthread_t tid=0;
    while(1){
        clilen = sizeof(cli_addr);
        //cout<<"Inside initial thread"<<endl;
        newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
        int *arg =(int*)malloc(sizeof(*arg));
        *arg = newsockfd;
        pthread_create(&tid, NULL, listerthread, arg);
        pthread_detach(tid);
               
    }
}
void *listerthread(void *sockfd){
    struct_details *arg_details=new struct_details();
        char Buffer [ BUFF_SIZE]; 
        int n;
        int newsockfd=*((int *)sockfd);
        recv(newsockfd,Buffer, BUFF_SIZE, 0);
        char *token;
        token=strtok(Buffer," ");
        char *file1=token;
        int chunk_number;
        long long int total_chunks;
        long long int org_file_size;
        string temp,temp2,temp3;
        int count=1;
        while(token!=NULL){
            if(count==2){
                temp=token;
            }
            if(count==3){
                temp2=token;
            }
            if(count==4){
                temp3=token;
            }
            token=strtok(NULL," ");    
            count++;
        }
        chunk_number=stoi(temp);
        total_chunks=stoll(temp2);
        org_file_size=stoll(temp3);
        /////////////////////send the respective file
        FILE *fp = fopen ( file1  , "r" );
        
        char Buffer1 [ BUFF_SIZE] ; 
        //cout<<chunk_number<<" "<<total_chunks<<" "<<org_file_size<<endl;
        int size;
        if(chunk_number!=total_chunks){
            size=CHUNK_SIZE;
        }
        else if(chunk_number==total_chunks && org_file_size%CHUNK_SIZE!=0){
            size=org_file_size%CHUNK_SIZE;
        }else{
            size=CHUNK_SIZE;   
        }
        rewind(fp);
        fseek ( fp ,(chunk_number-1)*CHUNK_SIZE, SEEK_SET);
        ///////////////debugging purpose
                    FILE *fp2;
                    fp2=fopen ( "readchunksize.txt" , "a" );
                    fprintf(fp2, "%d %d %d\n", size,chunk_number,(chunk_number-1)*CHUNK_SIZE);
                    fclose(fp2);
        ////////////////////
        send ( newsockfd , &size, sizeof(size), 0);
        while ( ( n = fread( Buffer1 , sizeof(char) , BUFF_SIZE , fp ) ) > 0  && size > 0 ){
          send (newsockfd , Buffer1, n, 0 );
          memset ( Buffer1 , '\0', BUFF_SIZE);
          size = size - n ;
            
        }
        fclose(fp);
    close(newsockfd);
    pthread_exit(NULL);

}
int main(int argc, char *argv[])
{

    struct_details *args_details=new struct_details();
    args_details->ipaddr=argv[1];
    args_details->port_no=argv[2];
    //continuous listen port
        pthread_t tid;
        pthread_create(&tid, NULL, continuouslisten_thread, (void *)args_details);
        pthread_detach(tid);
    ///////end continuous listen
    
    string startcommand;
    getline(cin,startcommand);
    char *toklogin;

    char temparr[200];
    strcpy(temparr,startcommand.c_str());
    toklogin=strtok(temparr," ");
    string temp=toklogin;
    //cout<<temp<<endl;
    int count1=1;
    if(temp.compare("login")==0){
        args_details->flag="3";
        while(toklogin!=NULL){
            if(count1==2){
                args_details->userid=toklogin;
            }
            if(count1==3){
                args_details->password=toklogin;
            }
            toklogin=strtok(NULL," ");
            count1++;
        }
        sendtotracker(args_details);
        if(login_flag==0){
            login_flag=0;
            cout<<"Authentication failure"<<endl;    
            return 0;
        }
        
    }
    else if(temp.compare("create_account")==0){
        //cout<<"HI";
        args_details->flag="4";
        while(toklogin!=NULL){
            if(count1==2){
                args_details->userid=toklogin;
            }
            if(count1==3){
                args_details->password=toklogin;
            }
            toklogin=strtok(NULL," ");
            count1++;
        }
        cout<<args_details->userid<<" "<<args_details->password<<endl;
        sendtotracker(args_details);
    }
    while(1){
        
        string commandstr,command,dest_path;
        cout<<"Command$>";
        getline(cin,commandstr);
        //cout<<commandstr;
        char temp[512];
        int count=1;
        strcpy(temp,commandstr.c_str());
        char *token;
        token=strtok(temp," ");
        command=token;
        if(command.compare("upload_file")==0){
            while(token!=NULL){
              if(count==2){
                args_details->filename=token;
                //cout<<args_details->filename;
              }
              if(count==3){
                args_details->groupid=token;
              }
                token=strtok(NULL," ");
                count++;
            }
            args_details->flag="1";
            sendtotracker(args_details);
        }
        else if(command.compare("download_file")==0){
            while(token!=NULL){
              if(count==2){
                args_details->groupid=token;
                //cout<<args_details->filename;
              }
              if(count==3){
                args_details->filename=token;
              }
              if(count==4){
                dest_path=token;
              }
              token=strtok(NULL," ");
              count++;
            }
            args_details->flag="2";
            sendtotracker(args_details);
            //recievefromtracker();
        }
    }
    return 0;
}