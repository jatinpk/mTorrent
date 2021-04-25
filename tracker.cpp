#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include<pthread.h>
#include<iostream>
#include<map>
#include<algorithm>
#include<vector>
#include<errno.h>
#include<fstream>
#include<bits/stdc++.h>
#include <openssl/sha.h>

#define PORT 9375
using namespace std;


//clientInfo
//username password port gid active

//groupinfo
//CID groupID

//filehash
//file_name hash_value

//statusfile
//filename hash CID gid bitmap

vector<vector<string>> clientinfo;
vector<vector<string>> groupinfo;
vector<vector<string>> filehash;
vector<vector<string>> statusfile;



void create(int);
void login(int);
void create_group(int);
void join_group(int);
void ListGroups(int);
void upload(int);
void download(int);
void logout(int);

//##########################################################################################################
//UTILITIES

char* stringtocharptr(string a)
{
	int length=a.length()+1;
	int i=0;
	char *ptr=(char*)malloc(sizeof(char)*length);
	for(i=0;i<a.length();i++)
	{
		ptr[i]=a[i];
	}ptr[i]='\0';
	return ptr;
}

void printVofV(vector<vector<string>> temp)
{
	for(auto i=temp.begin();i!=temp.end();i++)
	{
		for(string x:(*i))
			cout<<x<<" ";
		cout<<endl;
	}
}

void LoadInmemory()
{
	//load clientInfo
	//load groupinfo
	fstream in("/home/jatin/torr/FILES/ClientINFO.txt",ios::out|ios::in);

	cout<<strerror(errno);

	string line;

	while(!in.eof())
	{
		vector<string> v;
		getline(in,line);
		stringstream ss(line);
		string temp="";
		while(ss>>temp)
		{	
			v.push_back(temp);
		}		
		clientinfo.push_back(v);
	}


}

void StoreInFile(vector<vector<string>> input,string filename)
{
	//store clientinfo
	//store goupinfo


	string filepath="/home/jatin/torr/FILES/"+filename;
	fstream in;
	
	in.open(filepath, std::ofstream::out | std::ofstream::trunc);
	in.close();

	
	in.open(filepath,ios::out|ios::in);

	string line="";

	for(auto i=input.begin();i!=input.end();i++)
	{
		line="";
		for(string x:(*i))
		{
			line+=x;
			line+=" ";
		}
		if(line != "")
			in<<line<<endl;
	}

	in.close();
}
//#######################################################################################################
void *handleClient(void *sock)
{
	
	int new_sock=*(int *)sock;
	int whichfunc;
	
	recv(new_sock,&whichfunc,sizeof(whichfunc),0);
					
	
	if(whichfunc == 1)			//Create_acc
	{ 
		create(new_sock);
		close(new_sock);		
	}
	else if(whichfunc == 2)			//login
	{
		login(new_sock);
		close(new_sock);	
	}
	else if(whichfunc == 3)			//create_group
	{
		create_group(new_sock);
		close(new_sock);	
	}
	else if(whichfunc == 4)			//join
	{
		join_group(new_sock);
		close(new_sock);	
	}
	else if(whichfunc == 5)			//listall
	{
		ListGroups(new_sock);
		close(new_sock);	
	}
	else if(whichfunc == 6)			//upload
	{
		upload(new_sock);
		close(new_sock);	
	}
	else if(whichfunc == 7)			//download
	{
		download(new_sock);
		close(new_sock);	
	}
	else if(whichfunc == 8)			//download
	{
		logout(new_sock);
		close(new_sock);	
	}
	pthread_exit(NULL);
} 

//########################################################################################################
void create(int socket)   					//1
{	
	char user[20];
	char pass[20];
	char port[20];

	recv(socket,user,20,0);
	recv(socket,pass,20,0);
	recv(socket,port,20,0);

	string u=user;
	string p=pass;
	string pt=port;
	string gid="00";
	string isActive="0";

	vector<string> temp;
	temp.push_back(u);
	temp.push_back(p);
	temp.push_back(pt);
	temp.push_back(gid);
	temp.push_back(isActive);

	clientinfo.push_back(temp);


	printVofV(clientinfo);
	//StoreInFile(clientinfo,"ClientINFO.txt");


	
}

void login(int socket)						//2
{	

	char un[20];
	char pass[20];

	recv(socket,un,20,0);
	recv(socket,pass,20,0);

	string u=un;
	string p=pass;
	string reply="NO";
	string port="N";
	string group="";
	auto i=clientinfo.begin();
	for(;i!=clientinfo.end();i++)
	{
		if((*i)[0]==u)
		{
			if((*i)[1]==p)
			{
				port=(*i)[2];
				group=(*i)[3];
				reply="YES";
				break;
			}
		}
	}

	//cout<<reply<<endl;
	
	char *rep=stringtocharptr(reply);
	send(socket,rep,5,0);
	
	char *prt=stringtocharptr(port);
	//cout<<"PORT "<<prt<<endl;	
	send(socket,prt,5,0);
	
	char *grp=stringtocharptr(group);
	//cout<<"GROUP "<<grp<<endl;	
	send(socket,grp,5,0);



	/*if(strcmp(reply.c_str(),"YES")==0)
	{
		(*i)[4]="1";
	}
	else
	{
		(*i)[4]="0";
	}
	StoreInFile(clientinfo,"ClientINFO.txt");
*/
}


void create_group(int sock)						//3
{
	char gid[20];
	char uid[20];		
	recv(sock,uid,20,0);
	recv(sock,gid,10,0);

	string user=uid;
	string groupid=gid;

	for(auto i=clientinfo.begin();i!=clientinfo.end();i++)
	{
		if((*i)[0]==uid)
		{
			(*i)[3]=gid;
		}
	}

	vector<string> temp;
	temp.push_back(user);
	temp.push_back(groupid);

	groupinfo.push_back(temp);

	printVofV(groupinfo);
	//StoreInFile(clientinfo,"ClientINFO.txt");
	//StoreInFile(groupinfo,"groupINFO.txt");
}

void join_group(int sock)							//4
{		
	char uid[20];
	char gid[10];
	recv(sock,uid,20,0);
	recv(sock,gid,10,0);

	string ownerID="";
	for(auto i=groupinfo.begin();i!=groupinfo.end();i++)
	{
		if((*i)[1] == gid)
		{
			ownerID=(*i)[0];
			break;
		}
	}
	cout<<ownerID<<endl;
	char *temp=stringtocharptr(ownerID);
	send(sock,temp,10,0);



}

void ListGroups(int sock)						//5
{
	string grps="";
	for(auto i=groupinfo.begin();i!=groupinfo.end();i++)
	{
		grps+=(*i)[1];
		grps+=" ";
	}

	char *groups=stringtocharptr(grps);

	send(sock,groups,1000,0);
}

void upload(int sock)							//6
{
	char filename[100];
	char sha[20];
	char cid[20];
	char gid[20];
	char port[20];

	recv(sock,filename,100,0);
	recv(sock,sha,20,0);
	recv(sock,cid,20,0);
	recv(sock,gid,20,0);
	recv(sock,port,20,0);

	string fn=filename;
	string Sha=sha;
	string clientID=cid;
	string clientgrp=gid;
	string prt=port;
	//update FILE-->HASH mapping table

	vector<string> temp;
	temp.push_back(filename);
	temp.push_back(Sha);
	filehash.push_back(temp);

	//update Status table
	vector<string> temp2;
	temp2.push_back(filename);
	temp2.push_back(Sha);
	temp2.push_back(clientID);
	temp2.push_back(clientgrp);
	temp2.push_back(port);
	statusfile.push_back(temp2);

	cout<<"-------------------\n";
	printVofV(filehash);
	cout<<"--------------------\n";
	printVofV(statusfile);

}

void download(int sock)
{
	char filename[100];
	char grp[10];

	recv(sock,filename,100,0);
	recv(sock,grp,10,0);

	//cout<<filename<<" "<<grp<<endl;
	string fN=filename;
	string grpid=grp;

	cout<<fN<<" "<<grpid<<endl;
	//search in status to get client IDs

	//statusfile ==> filename , hash , cliendID , groupID , PORT
	string reqclientCLIENTS="";
	vector<string> clientVector;
	string reqclientPORTS="";
	
	
	// find clientIDS
	auto i=statusfile.begin();

	int j=0;
	for(;i!=statusfile.end();i++)
	{
		if((*i)[0] == fN && (*i)[3] == grpid)
		{
			string temp1=(*i)[2];
			reqclientCLIENTS+=temp1;
			reqclientCLIENTS+=" ";
			string temp2=(*i)[4];
			//clientVector.push_back(temp);
			reqclientPORTS+=temp2;
			reqclientPORTS+=" ";
		}
	}


	cout<<reqclientCLIENTS<<endl;
	cout<<reqclientPORTS<<endl;
	char *clientports=stringtocharptr(reqclientPORTS);
	char *clientids=stringtocharptr(reqclientCLIENTS);

	send(sock,clientports,100,0);
	send(sock,clientids,100,0);
	close(sock);

}


void logout(int sock)							//8
{
	char user[20];
	recv(sock,user,20,0);

	string u=user;
	//cout<<reply<<endl;
	for(auto i=clientinfo.begin();i!=clientinfo.end();i++)
	{
		if((*i)[0]==u)
		{
			(*i)[4]="0";
			break;
			
		}
	}

	//StoreInFile(clientinfo,"ClientINFO.txt");

}
int main(int argc, char const *argv[]) 
{ 
	clientinfo.clear();

	//LoadInmemory();

	for(auto i=clientinfo.begin();i!=clientinfo.end();i++)
	{
		for(string x:(*i))
		{
			cout<<x<<" ";	
		}cout<<endl;
	}


	int server_fd, new_sock, valread; 
	int addrlen = sizeof(struct sockaddr_in);
	 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	struct sockaddr_in address; 

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	address.sin_port = htons(PORT); 
	

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	
	pthread_t th;
	
	while(1)	 
	{

		new_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

		if(pthread_create(&th,NULL,handleClient,(void *)&new_sock)!= 0)
		{
			cout<<"error in pthread_create\n";
			exit(EXIT_FAILURE);
		}		

	}
	//close(new_sock);
	
	return 0; 
} 