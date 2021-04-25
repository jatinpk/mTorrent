// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <iostream>
#include <vector>
#include <bits/stdc++.h>
#include<errno.h>
#include<string.h>
#include <openssl/sha.h>


#define PORT 9375

using namespace std;

float CHUNKSIZE=65532;
vector<string> input;
vector<vector<string>> myFilesBitmap;
map<string,int> clientFlag;

//int sentdata;
int recdata;


struct node
{
	string cid;
	string port;
	string chunkno;
	string filename;
};



string currentUserID="";
string currentPort="";
string currentlyActive="";
string currentgrpid="";

pthread_mutex_t cs; 

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

//###################################################################################################
string calculateSHA(char *filename)
{

	FILE *fp=fopen(filename,"r");


	unsigned char *hash=new unsigned char [20];
	
	unsigned char finalhash[524288];
	size_t readbytes;

	float readsize=512*1024;
	int pos=0;


	ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    double filesize=in.tellg(); 

	int numberOfChunks=ceil(filesize/readsize);
	
	int temp=ceil(filesize);

	unsigned char buffer[temp];

	unsigned char SHA[20];

	
	SHA_CTX ctx;
	SHA1_Init(&ctx);

	readbytes=fread(buffer,1,filesize,fp);
	
	cout<<readbytes<<endl;
	buffer[readbytes]='\0';

	SHA1((unsigned char*)buffer,readbytes,hash);

	string hsh="";
	int x;
	stringstream stream;
	string result="";
	for(int i=0;i<20;i++)
	{
		x=(int)hash[i];
		stream << std::hex <<x;
	}
	result=stream.str();
		
	return result;


}

void createreq(int sock)
{
	//int sock=*(int *)sck;
	int whichfunc=1;
	char* username=stringtocharptr(input[1]);
	char* password=stringtocharptr(input[2]);
	char* port=stringtocharptr(input[3]);
			
	send(sock,&whichfunc,sizeof(whichfunc),0);		
	send(sock,username,20,0);
	cout<<strerror(errno);
	send(sock,password,20,0);
	cout<<strerror(errno);
	send(sock,port,20,0);
	cout<<strerror(errno);

}

void loginreq(int sock)
{
	int whichfunc=2;
	char* username=stringtocharptr(input[1]);
	char* password=stringtocharptr(input[2]);
			
	send(sock,&whichfunc,sizeof(whichfunc),0);		
	send(sock,username,20,0);
	send(sock,password,20,0);

	char buf[5];
	char prt[5];
	char grpid[5];

	recv(sock,buf,5,0);
	recv(sock,prt,5,0);
	recv(sock,grpid,5,0);

	if(strcmp(buf,"YES") == 0)
	{
		currentUserID=username;
		currentPort=prt;
		currentgrpid=grpid;
		cout<<"Login successful\n";
	}
	else
		cout<<"Wrong ID or Pass\n";	

	currentlyActive=buf;
	cout<<"R u Active Now: "<<currentlyActive<<endl;
}

void creategroupreq(int sock)
{
	int whichfunc=3;
	char* gid=stringtocharptr(input[1]);
	char* uid=stringtocharptr(currentUserID);
	send(sock,&whichfunc,sizeof(whichfunc),0);		
	send(sock,uid,20,0);
	send(sock,gid,10,0);

	char g[10];
	recv(sock,g,10,0);

	currentgrpid=gid;
	cout<<"CGRPID "<<currentgrpid<<endl;

}

void joingroupreq(int sock)
{
	int whichfunc=4;
	char* gid=stringtocharptr(input[1]);
	char* uid=stringtocharptr(currentUserID);
	send(sock,&whichfunc,sizeof(whichfunc),0);		
	send(sock,uid,20,0);
	send(sock,gid,10,0);

	char ownerID[10];
	recv(sock,ownerID,10,0);

	cout<<ownerID<<endl;

	string oid=ownerID;

}
void listgroupsre(int sock)
{
	char buff[1024];

	int whichfunc=5;

	send(sock,&whichfunc,sizeof(whichfunc),0);

	recv(sock,buff,1000,0);

	string grps=buff;

	stringstream ss(grps);

	string temp;

	cout<<"GROUPS\n";
	while(ss>>temp)
	{
		cout<<temp<<endl;
	}

}

 
void upload(int sock)
{
	cout<<"in upload\n";
	char *file_name=stringtocharptr(input[1]);
	//string sha=calculateSHA(file_name);
	string sha=" ";
	char *cid=stringtocharptr(currentUserID);
	char *gid=stringtocharptr(currentgrpid);
	char *port=stringtocharptr(currentPort);


	//find BITMAP size


	ifstream in(file_name, std::ifstream::ate | std::ifstream::binary);
    double filesize=in.tellg(); 

    cout<<"FILESIZE "<<filesize<<endl;

	int noc=ceil(filesize/CHUNKSIZE);

	cout<<"NOC "<<noc<<endl;
	//cout<<filesize<<" "<<noc<<endl;	

	in.close();

	string BITMAP="";

	for(int i=0;i<noc;i++)
	{
		BITMAP+="1";
	}

	vector<string> temp;
	temp.push_back(string(file_name));
	temp.push_back(BITMAP);

	myFilesBitmap.push_back(temp);

	printVofV(myFilesBitmap);

	//send request to tracker to get clientports

	int whichfunc=6;

	send(sock,&whichfunc,sizeof(whichfunc),0);

	send(sock,file_name,100,0);
	send(sock,sha.c_str(),20,0);
	send(sock,cid,20,0);
	send(sock,gid,20,0);
	send(sock,port,20,0);

	//char ports[100];				//PORTS to connect
	//recv(sock,ports,100,0);

	close(sock);


}

void* sendChunk(void *t)
{
	int sock=*(int *)t;

	int chunkno;
	long int offset;
	char filename[30];

	recv(sock,&chunkno,sizeof(chunkno),0);
	recv(sock,&offset,sizeof(offset),0);
	recv(sock,filename,30,0);


	cout<<"FILENAME "<<filename<<endl;
	fstream f;
	f.open(filename,ios::in|ios::out);
	
	if(!f)
	{
		cout<<"File not opened\n";

	}

	char *buffer=new char[(int)CHUNKSIZE];
	f.seekg(offset,f.beg);
	f.read(buffer,CHUNKSIZE);
	int sentdata=f.gcount();

	if(sentdata < CHUNKSIZE)
	{
		//cout<<chunkno<<" SENTLESS\n";
		send(sock,buffer,sentdata,0);
	}//cout<<buffer<<endl;
	else
	{
		//cout<<chunkno<<" SENTFULL\n";
		send(sock,buffer,sentdata,0);
	}

	f.close();
	
	/*int sent=0;
	int ite=CHUNKSIZE/100;
	for(int i=0;i<ite;i++)
	{
		send(sock,buffer,100,0);
		sent++;
	}*/
	//cout<<"sentbytes "<<sent<<endl;
	//cout<<"chunk-"<<chunkno<<" sent\n";

	memset(buffer,'\0',CHUNKSIZE);
}

void* downfromclient(void *t)
{
	    //pthread_mutex_lock(&cs); 

	struct node *temp=(struct node *)t;

	string cid=temp->cid;
	string port=temp->port;
	string chunkno=temp->chunkno;
	string filename=temp->filename;

	while(clientFlag[cid] == 1);

	clientFlag[cid]=1;

	//FILE *fp=fopen("/home/jatin/output","w");

	int chunksize=(long int)CHUNKSIZE;
	long int offset=stoi(chunkno)*chunksize;

	//cout<<chunkno<<" "<<offset<<endl;

	//connect to port

	int tempsock=0;
	
	if ((tempsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		pthread_exit(NULL); 
	} 

	struct sockaddr_in serv_addr; 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int x; 
    sscanf(port.c_str(), "%d", &x);
    serv_addr.sin_port = htons(x); 

	if (connect(tempsock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		cout<<strerror(errno)<<endl;	
		printf("\nConnection Failed \n"); 
		pthread_exit(NULL); 
	} 

	//send type of function
	//send chunk no
	//send offset
	//send filename


	//cout<<"CID="<<cid<<endl;
	int type=30;
	int chno=stoi(chunkno);

	char *file=stringtocharptr(filename);

	send(tempsock,&type,sizeof(type),0);			//typeoffunc	
	send(tempsock,&chno,sizeof(chno),0);			//chunk number
	send(tempsock,&offset,sizeof(offset),0);		//offset of that chunk
	send(tempsock,file,30,0);						//filename

	char *buffer=new char[(int)CHUNKSIZE];

	int rdata=recv(tempsock,buffer,CHUNKSIZE,0);


	/*int rec=0;
	int ite=CHUNKSIZE/100;
	for(int i=0;i<ite;i++)
	{
		recv(tempsock,buffer,100,0);
		rec++;
	}
*/
	//cout<<"received bytes "<<rec<<endl;
	//cout<<"Bytes read from "<<cid<<" for chunk-"<<chunkno<<" "<<rec<<endl;

	fstream f;
	f.open("ofile",ios::in|ios::out|ios::binary);
	
	if(!f)
	{
		cout<<"File not opened\n";

	}

	f.seekg(offset,f.beg);

	if(rdata<CHUNKSIZE)
	{
		//cout<<chunkno<<" "<<"RECVLESS\n";
		f.write(buffer,rdata);
	}
	else
	{
		//cout<<chunkno<<" "<<"RECVFULL\n";

		f.seekg(offset,f.beg);
		f.write(buffer,CHUNKSIZE);
	}
	    //pthread_mutex_unlock(&cs); 
	f.close();

	memset(buffer,'\0',CHUNKSIZE);

	close(tempsock);

	clientFlag[cid]=0;
}




void download(int sock)
{
	//sock1 is tracker port
	char *filename=stringtocharptr(input[1]);
	char *grpid=stringtocharptr(currentgrpid);

	//send tracker request and get the ports if clients
	int whichfunc=7;
	send(sock,&whichfunc,sizeof(whichfunc),0);
	send(sock,filename,100,0);
	send(sock,grpid,10,0);

	char clientports[100];
	char clientids[100];

	recv(sock,clientports,100,0);
	recv(sock,clientids,100,0);

	cout<<clientids<<endl;
	cout<<clientports<<endl;
	

	vector<string> requiredClientPorts;
	vector<string> requiredClientids;

	string cltpts=clientports;

	stringstream ss(cltpts);

	string temp;
	int clientcount=0;
	while(ss>>temp)
	{
		requiredClientPorts.push_back(temp);
		clientcount++;
	}

	string cltids=clientids;
	stringstream ss1(cltids);

	temp="";

	while(ss1>>temp)
	{
		requiredClientids.push_back(temp);
	}


	close(sock);


	ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    double filesize=in.tellg(); 


	int chunks=ceil(filesize/CHUNKSIZE);

	in.close();

	map<string,string>	BITMAPS;

	//get BITMAPS from clients
	for(int i=0;i<clientcount;i++)
	{
		int tempsock=0;
		
		if ((tempsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		{ 
			printf("\n Socket creation error \n"); 
			pthread_exit(NULL); 
		} 

		struct sockaddr_in serv_addr; 

		serv_addr.sin_family = AF_INET; 
		serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
		int x; 
    	sscanf(requiredClientPorts[i].c_str(), "%d", &x);
    	serv_addr.sin_port = htons(x); 


		if (connect(tempsock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
		{ 
			cout<<strerror(errno)<<endl;	
			printf("\nConnection Failed \n"); 
			pthread_exit(NULL); 
		} 

		char buf[100];
		int whichone=10;
		send(tempsock,&whichone,sizeof(whichone),0);       //sendinf TypeOf funct
		//send(tempsock,requiredClientids[i].c_str(),30,0);       //sending ID of client
		send(tempsock,filename,100,0);


		recv(tempsock,buf,100,0);
		string tempbuf=buf;
		
		//chunks = tempbuf.length();

		BITMAPS[requiredClientids[i]]=tempbuf;
		
		close(tempsock);
	}

	for(auto i=BITMAPS.begin();i!=BITMAPS.end();i++)
	{
		cout<<i->first<<"-"<<i->second<<endl;
	}
	
	//APPLY ALGORITHM

	map<string,string> chunktoclient;
	map<string,string> chunktoport;

	for(int i=0;i<chunks;i++)
	{
		char bit=0;
		int whichClient;
		do
		{
			whichClient = rand()%clientcount;
			string cid = requiredClientids[whichClient];
			string bmap=BITMAPS[cid];
			bit=bmap[i];
		}while(bit == '0');
			
		chunktoclient[to_string(i)]=requiredClientids[whichClient];
		chunktoport[to_string(i)]=requiredClientPorts[whichClient];

		//	cout<<"Choice for chunk "<<i<<"-"<<chunktoclient[i]<<endl;
	}


	for(int i=0;i<chunks;i++)
	{
		clientFlag[chunktoclient[to_string(i)]] = 0;
	}	

	
	struct node structarray[chunks];

	for(int i=0;i<chunks;i++)
	{
		string s=to_string(i);
		struct node temp;
		temp.chunkno=s;
		temp.cid=chunktoclient[s];
		temp.port=chunktoport[s];
		temp.filename=filename;
		structarray[i]=temp;
	}

	for(int i=0;i<chunks;i++)
	{
		struct node t=structarray[i];
		cout<<"chunkno "<<t.chunkno<<" ";
		cout<<"cid "<<t.cid<<" ";
		cout<<"port "<<t.port<<endl;
		cout<<"filename "<<t.filename<<endl;
	}


	pthread_t threadArray[chunks];

	for(int i=0;i<chunks;i++)
	{
		pthread_create(&threadArray[i],NULL,downfromclient,&(structarray[i]));

		void *status;
		pthread_join(threadArray[i],&status);
	}	
	
	cout<<"recdata "<<recdata<<endl;
}
void logout(int sock)
{
	const char *userid=currentUserID.c_str();
	int whichfunc=8;
	send(sock,&whichfunc,sizeof(whichfunc),0);
	send(sock,userid,20,0);

	currentUserID="";
	currentPort="";
	currentlyActive="NO";
	currentgrpid="";

	cout<<"Logged out\n";
	int x; 
    sscanf(currentPort.c_str(), "%d", &x);
	close(x);	

}
 
void *bitmapRequest(void* sock)
{
	int tsock=*((int *)sock);
	char CID[30];
	char FN[100];

	//recv(tsock,CID,30,0);
	recv(tsock,FN,100,0);

	//string cid=CID;
	string fn=FN;
	string bmp="";
	//buff == > client ID
	//return its BITMAP

	for(auto i=myFilesBitmap.begin();i!=myFilesBitmap.end();i++)
	{
		if((*i)[0] == fn)
		{
			bmp=(*i)[1];
		}
	}

	char *btmp=stringtocharptr(bmp);
	send(tsock,btmp,100,0);

	return NULL;
}

void* seeder(void *socket)
{
	int sock=*(int *)socket;

	int p;
	recv(sock,&p,sizeof(p),0);
	cout<<"P "<<p<<endl;

	int t=100;
	send(sock,&t,sizeof(t),0);
}

void* asServer(void *)
{
	int cserver_fd, new_sock; 
	

	int addrlen = sizeof(struct sockaddr_in);
	 
	if ((cserver_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	struct sockaddr_in address; 

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	int x; 
    sscanf(currentPort.c_str(), "%d", &x);
    //cout<<"port "<<currentPort<<endl; 
	address.sin_port = htons(x); 
	

	if (bind(cserver_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	if (listen(cserver_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	
	pthread_t th;

	while(1)	 
	{

		cout<<"waiting\n";

		new_sock = accept(cserver_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
		int whichone;
		recv(new_sock,&whichone,sizeof(whichone),0);

		if(whichone==10)
		{
			if(pthread_create(&th,NULL,bitmapRequest,(void *)&new_sock)!= 0)
			{
				cout<<"error in pthread_create\n";
				exit(EXIT_FAILURE);
			}		
		}
		else if(whichone==20)
		{
			if(pthread_create(&th,NULL,seeder,(void *)&new_sock)!= 0)
			{
				cout<<"error in pthread_create\n";
				exit(EXIT_FAILURE);
			}	
		}
		else if(whichone==30)
		{
			if(pthread_create(&th,NULL,sendChunk,(void *)&new_sock)!= 0)
			{
				cout<<"error in pthread_create\n";
				exit(EXIT_FAILURE);
			}	
		}
	}

}

int main(int argc, char const *argv[]) 
{ 
	pthread_t t;	

	
	while(1)
	{
		int sock=0;
		//CREATING SOCKET
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		{ 
			printf("\n Socket creation error \n"); 
			pthread_exit(NULL); 
		} 

		struct sockaddr_in serv_addr; 

		serv_addr.sin_family = AF_INET; 
		serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
		serv_addr.sin_port = htons(PORT); 
		


		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
		{ 
			cout<<strerror(errno)<<endl;	
			printf("\nConnection Failed \n"); 
			pthread_exit(NULL); 
		} 

		//cout<<"sentdata "<<sentdata<<endl;
		cout<<"again\n";
		
		string command="";

		getline(cin,command);

		stringstream ss(command);

		input.clear();

		string temp="";
		while(ss>>temp)
		{
			input.push_back(temp);
		}
		
		if(input[0] == "c")				// 1
		{
			createreq(sock);
		}	

		else if(input[0] == "l")      	// 2
		{
			if(currentlyActive=="YES")
				cout<<"Already Logged In\n";
			else
			{
				loginreq(sock);
				pthread_create(&t,NULL,asServer,NULL);
			}
		}	
		
		else if(input[0] == "cg")       // 3
		{
			if(currentlyActive!="YES")
				cout<<"Login First\n";
			else
				creategroupreq(sock);
		}
		else if(input[0] == "jg")  		// 4
		{
			if(currentlyActive!="YES")
				cout<<"Login First\n";
			else
				joingroupreq(sock);
		}
		else if(input[0] == "listall")	// 5
		{
			if(currentlyActive!="YES")
				cout<<"Login First\n";
			else
				listgroupsre(sock);
		}
		else if(input[0] == "up")		//6
		{
			if(currentlyActive!="YES")
				cout<<"Login First\n";
			else
				upload(sock);
		}
		else if(input[0] == "down")		//7
		{
			if(currentlyActive!="YES")
				cout<<"Login First\n";
			else
				download(sock);
		}
		else if(input[0] == "logout")		//8
		{
			if(currentlyActive!="YES")
				cout<<"Login First\n";
			else
				logout(sock);
		}
	}



	void *status;
	pthread_join(t,&status);
	
	return 0; 
} 
	