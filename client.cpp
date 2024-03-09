//未完成
//封装socket通讯的客户端
//非阻塞
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
using namespace std;

//把socket设置成非阻塞
int setnonblocking(int fd){
	int flags;
	//获取flag状态。
	if ((flags=fcntl(fd,F_GETFL,0))==-1)flags=0;
	return fcntl(fd,F_SETFL,flags|O_NONBLOCK);
}
class ctcpclient
{
public:
	int m_clientfd;		//客户端socket，-1表示未连接或断开
	string m_ip;		//服务端IP/域名
	unsigned short m_port;	//通信端口

	ctcpclient():m_clientfd(-1){}
	
	//一、连接服务器
	bool connect(const string in_ip,const unsigned short in_port){
	 if(m_clientfd != -1) return false;	//若已连接，返回失败
	 m_ip=in_ip; m_port=in_port; 		//保存传入参数

	 //1.创建客户端socket
	 if((m_clientfd = socket(AF_INET,SOCK_STREAM,0))==-1) return false;
	 //
	 setnonblocking(m_clientfd);
	 //2.向服务器发起连接请求
	 struct sockaddr_in servaddr;		//存放协议、端口、IP的结构体
	 servaddr.sin_family =AF_INET;
	 servaddr.sin_port = htons(m_port);

	 struct hostent* h;
	 if((h = gethostbyname(m_ip.c_str())) == nullptr)
	 {::close(m_clientfd);m_clientfd=-1;return false;}
	 memcpy(&servaddr.sin_addr,h->h_addr,h->h_length);
	 if(::connect(m_clientfd,(struct sockaddr *)&servaddr,sizeof(servaddr))==-1)
	 {::close(m_clientfd);m_clientfd=-1;return false;}
	 return true;
	}
	//发送请求报文  //char* 只支持c_str
        bool send(const string &buffer){
       	 if(m_clientfd==-1)return false;
	 if((::send(m_clientfd,buffer.data(),buffer.size(),0))<=0)return false;
	 return true;
	}
	bool recv(string &buffer,const size_t maxlen){
	 buffer.clear();
	 buffer.resize(maxlen);
	 int readn=::recv(m_clientfd,&buffer[0],buffer.size(),0);//返回-1——失败；0——socket已断开；>0——收到的数据量
	 if(readn<=0){buffer.clear();return false;}
	 buffer.resize(readn);//重置实际大小
	 return true;
	}
	bool close(){
	if(m_clientfd==-1)return false;
	::close(m_clientfd);
	m_clientfd=-1;
	return true;
	}
	~ctcpclient(){close();}
	
};

int main(int argc,char* argv[]){
	if(argc!=3){cout<<"参数数量错误"<<endl;return -1;}
	ctcpclient tcpclient;
	tcpclient.connect(argv[1],atoi(argv[2]));
	//由于非阻塞connect()都会返回失败，需要判断错误代码：
	if(errno!=EINPROGRESS){
		perror("connect()");return -1;
	}
	
	string buffer;
	for(int i=0;i<10;i++){
	 //生成报文
	 buffer="aaa"+to_string(i);
	 
	 //发送请求报文
	 if(tcpclient.send(buffer)==false){perror("send");break;}
	 cout << "发送" << buffer << endl;
	 
	 //接收回应报文，recv()函数阻塞等待
	 if(tcpclient.recv(buffer,1024)==false)
	 {
	  perror("recv()");
	  break;
	 }
	 cout << "接收：" << buffer <<endl;
 
 	 sleep(1);
	}
	close(tcpclient.m_clientfd);	
	return 0;
}


