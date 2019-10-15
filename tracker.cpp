#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <bits/stdc++.h>
#include <ctype.h>
#include <iostream>

#define BUFF_SIZE 2048
#define CHUNK_SIZE 524288
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
void *recieve(void *);
vector<struct_details*> vec_trackerdata;
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
void server(){
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[512];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 9595;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
    listen(sockfd,5);

    while(1){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
              
        pthread_t tid;
        pthread_create(&tid, NULL, recieve, &newsockfd);
        pthread_detach(tid);
        
        /////debugging purpose
            FILE *fp1;
            fp1=fopen ( "uploadinfo.txt" , "w" );
            
            for(int i=0;i<vec_trackerdata.size();i++){
                fprintf(fp1, "%s ", vec_trackerdata[i]->ipaddr.c_str());
                fprintf(fp1, "%s ", vec_trackerdata[i]->port_no.c_str());
                fprintf(fp1, "%s ", vec_trackerdata[i]->filename.c_str());
                fprintf(fp1, "%lld ", vec_trackerdata[i]->filesize);
                fprintf(fp1, "%lld ", vec_trackerdata[i]->number_chunks);
                for(int j=1;j<=vec_trackerdata[i]->number_chunks;j++){
                    if(j!=vec_trackerdata[i]->number_chunks){
                        fprintf(fp1, "%d ", vec_trackerdata[i]->chunk_details[j]);
                    }else{
                        fprintf(fp1, "%d\n", vec_trackerdata[i]->chunk_details[j]);
                    }
                }
            }
            fclose(fp1);
        /////////////////////////
        
    }
    close(sockfd);
}
void *recieve(void *sockfd){
    char Buffer [ BUFF_SIZE]; 
    struct_details *arg_details=new struct_details();
    int *newsock=(int *)sockfd;
    int newsockfd=*newsock;
    memset ( Buffer , '\0', BUFF_SIZE);
    recv(newsockfd,Buffer, BUFF_SIZE, 0);
    int count=0;
//////////////////////////////
        FILE *fp1;
        fp1=fopen ( "uploadinfo1.txt" , "a" );
        fprintf(fp1, "%s\n", Buffer);
        fclose(fp1);
///////////////////////////////
    //string data(Buffer);
    char *token;
    if(Buffer[0]=='1'){
        struct_details *new_struct=new struct_details();
        //FILE *fp = fopen ( "clientinfo.txt" , "a" );
        token=strtok(Buffer," ");
        int count=1;
        while(token!=NULL){
            if(count==2){
                new_struct->ipaddr=token;
                //fprintf(fp, "%s", token);
            }
            if(count==3){
                new_struct->port_no=token;
                //fprintf(fp, "%s", token);   
            }
            if(count==4){
                new_struct->filename=token;
                //fprintf(fp, "%s", token);   
            }
            if(count==5){
                new_struct->groupid=token;
                //fprintf(fp, "%s", token);   
            }
            token=strtok(NULL," ");
            count++;
        }
        ////to get the file size
        char *file1;
        char tempb[new_struct->filename.length()+1];
        strcpy(tempb,new_struct->filename.c_str());
        file1=tempb;
        FILE *fpsize=fopen(file1,"r");
        fseek(fpsize,0,SEEK_END);
        new_struct->filesize = ftell ( fpsize );
        fclose(fpsize);
        ////////to store the total chunks
        if(new_struct->filesize%CHUNK_SIZE==0){
            new_struct->number_chunks=new_struct->filesize/CHUNK_SIZE;
        }else{
            new_struct->number_chunks=new_struct->filesize/CHUNK_SIZE+1;
        }


        for(int i=0;i<=new_struct->number_chunks;i++){
            new_struct->chunk_details.push_back(1);
        }
        vec_trackerdata.push_back(new_struct);
        ////////////////
        /////debugging purpose
        FILE *fp1;
        fp1=fopen ( "uploadinfo.txt" , "w" );
        
        for(int i=0;i<vec_trackerdata.size();i++){
            fprintf(fp1, "%s ", vec_trackerdata[i]->ipaddr.c_str());
            fprintf(fp1, "%s ", vec_trackerdata[i]->port_no.c_str());
            fprintf(fp1, "%s ", vec_trackerdata[i]->filename.c_str());
            fprintf(fp1, "%lld ", vec_trackerdata[i]->filesize);
            fprintf(fp1, "%lld ", vec_trackerdata[i]->number_chunks);
            for(int j=1;j<=vec_trackerdata[i]->number_chunks;j++){
                if(j!=vec_trackerdata[i]->number_chunks){
                    fprintf(fp1, "%d ", vec_trackerdata[i]->chunk_details[j]);
                }else{
                    fprintf(fp1, "%d\n", vec_trackerdata[i]->chunk_details[j]);
                }
            }
        }
        fclose(fp1);
        /////////////////////////
    }else if(Buffer[0]=='2'){
        string file_tosearch;
        token=strtok(Buffer," ");
        while(token!=NULL){
            if(count==3){
                file_tosearch=token;
            }
            token=strtok(NULL," ");
            count++;
        }
        string data_tosend;
        /////send the number of places that file is present
        int numberofplaces=0;
        for(int i=0;i<vec_trackerdata.size();i++){
            if(vec_trackerdata[i]->filename.compare(file_tosearch)==0){
                numberofplaces++;
            }
        }
        string temp1=to_string(numberofplaces);
        char buff1[BUFF_SIZE];
        strcpy(buff1,temp1.c_str());
        send ( newsockfd , (const void *)buff1, BUFF_SIZE, 0);


        for(int i=0;i<vec_trackerdata.size();i++){
            if(vec_trackerdata[i]->filename.compare(file_tosearch)==0 ){
                data_tosend=vec_trackerdata[i]->ipaddr+" "
                            +vec_trackerdata[i]->port_no+" "
                            +to_string(vec_trackerdata[i]->filesize)+" "
                            +to_string(vec_trackerdata[i]->number_chunks);
                for(int j=1;j<=vec_trackerdata[i]->number_chunks;j++){
                    data_tosend=data_tosend+" "+to_string(vec_trackerdata[i]->chunk_details[j]);
                }


                char buff[BUFF_SIZE];
                strcpy(buff,data_tosend.c_str());

                send ( newsockfd , (const void *)buff, BUFF_SIZE, 0);
                ///debug
                FILE *fp1;
                fp1=fopen ( "sentinfo.txt" , "a" );
                fprintf(fp1, "%s\n", buff);
                fclose(fp1);
            }
        }
        
    }
    else if(Buffer[0]=='3'){
        token=strtok(Buffer," ");
        int count=1;
        string userid="";
        string passwd="";
        while(token!=NULL){
            if(count==2){
                userid=token;
            }
            if(count==3){
                passwd=token;
            }
            token=strtok(NULL," ");
            count++;
        }
        FILE *fp=fopen("login.txt","r");
        char linedata[200],linedata1[200];
        string res="No";
        int flagu=0,flagp=0;
        while(fscanf(fp,"%s %s",linedata,linedata1)!= EOF){
            char *tok1;
            tok1=strtok(linedata," ");
            /////////////////for debugging
                FILE *fp1;
                fp1=fopen ( "create_acct.txt" , "a" );
                fprintf(fp1, "%s\n", linedata);
                fprintf(fp1, "%s\n", linedata1);
                fclose(fp1);
        ////////////////////////////////
            
            int cnt=1;
            string temp1=linedata;
            string temp2=linedata1;
            if(temp1.compare(userid)==0){
                flagu=1;
            }
            if(temp2.compare(passwd)==0){
                flagp=1;
            }
            if(flagu==1 && flagp==1){
                    res="Yes";
                    break;
            }
            memset ( linedata , '\0', 200);
            memset ( linedata1 , '\0', 200);
               
        }
        char buff[BUFF_SIZE];
        strcpy(buff,res.c_str());
        send ( newsockfd , (const void *)buff, BUFF_SIZE, 0);

    }else if(Buffer[0]=='4'){
        
        FILE *fp=fopen("login.txt","a");
        token=strtok(Buffer," ");
        int count=1;
        string datatowrite="";
        while(token!=NULL){
            string temp=token;
            if(count>1){
                datatowrite=datatowrite+temp;
            }
            if(count==2){
                datatowrite=datatowrite+" ";
            }
            token=strtok(NULL," ");
            count++;
        }
        //char *t=new char[datatowrite.length()+1];
        //strcpy(t,datatowrite.c_str());
        string str1(datatowrite);
        fprintf(fp, "%s\n", str1.c_str());
        fclose(fp);
    }else if(Buffer[0]=='5'){
        ///////////////debugging purpose
            FILE *fp3;
            fp3=fopen ( "trackerrecv.txt" , "a" );
            fprintf(fp3, "%s\n", Buffer);
            fclose(fp3);
            ////////////////////
        string fname;
        string recvip;
        string recvport;
        int curr_chunk_number;
        long long int total_chunks,fsize;
        token=strtok(Buffer," ");
        int count=1;
        while(token!=NULL){
            if(count==2){
                recvip=token;
            }
            if(count==3){
                recvport=token;
            }
            if(count==4){
                fname=token;
            }
            if(count==5){
                string temp=token;
                curr_chunk_number=stoi(temp);
            }
            if(count==6){
                string temp1=token;
                fsize=stoll(temp1);
            }
            if(count==7){
                string temp2=token;
                total_chunks=stoll(temp2);
            }
            count++;
            token=strtok(NULL," ");
        }
        int flag=0;
        for(int i=0;i<vec_trackerdata.size();i++){
            //check if entry already present
            if(vec_trackerdata[i]->filename.compare(fname)==0 
                && vec_trackerdata[i]->ipaddr.compare(recvip)==0
                && vec_trackerdata[i]->port_no.compare(recvport)==0){
                ///////////////debugging purpose
                FILE *fp4;
                fp4=fopen ( "countcompare.txt" , "a" );
                fprintf(fp4, "%d\n", curr_chunk_number);
                fclose(fp4);
                ////////////////////
                vec_trackerdata[i]->chunk_details[curr_chunk_number]=1;
                flag=1;
            }
        }
        //if entry not present
        if(flag==0){
            struct_details *push_new_struct=new struct_details();
            push_new_struct->ipaddr=recvip;
            push_new_struct->port_no=recvport;
            push_new_struct->filename=fname;
            push_new_struct->filesize=fsize;
            push_new_struct->number_chunks=total_chunks;
            
            for(int i=0;i<=total_chunks;i++){
                if(i==curr_chunk_number){
                    push_new_struct->chunk_details.push_back(1);
                }else{
                    push_new_struct->chunk_details.push_back(0);
                }
            }
            vec_trackerdata.push_back(push_new_struct);

            
        }


    }
    close(newsockfd);
    pthread_exit(NULL);
}
int main(int argc, char *argv[])
{

    //int trackerno=argv[2];
    //while(1)
    server();
    return 0;
}