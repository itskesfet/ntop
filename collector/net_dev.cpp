//➜  net cat dev.cpp

#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <sstream>

#ifdef __linux__
#define PROC_NET_DEV "/proc/net/dev"
#define PAGE_SIZE    4096
#endif

struct InterfaceBandwidthStats{
	std::string iface;
	unsigned long long 	recived_bytes;
	unsigned long long 	transmitted_byte;
	unsigned long 		  recived_packets;
	unsigned long 		  transmited_packets;
	unsigned long 		  recived_drop;
	unsigned long 		  transmited_drop;
};

std::vector<std::string> split(std::string const &input){
	using namespace std;
	vector<string> packs;
	string token;
	istringstream input_stream(input);
  
	while(input_stream >> token){
		if( ":" == token ){
			continue;
		}
		packs.push_back(token);
	}
	return packs;
}

std::vector <struct InterfaceBandwidthStats> r_NetDev(){	
	
	int 	filefd = 0;
	char 	temp_buff[PAGE_SIZE] = {0};
	ssize_t nread  = 0;
	char    *buffer = NULL;
	size_t  buff_used = 0;
	size_t  capacity = 0;

	if(access(PROC_NET_DEV, F_OK) == -1) return {};
	if(access(PROC_NET_DEV, R_OK) == -1) return {};
	
	filefd = open(PROC_NET_DEV, O_RDONLY | O_CLOEXEC);
	if(filefd == -1){
		return {};
	}
  
	while((nread = read(filefd, temp_buff, PAGE_SIZE)) > 0){
		if(buff_used + nread > capacity){
			capacity = capacity ? capacity * 2 : PAGE_SIZE;
			while(capacity < buff_used + nread){
				capacity *= 2;
			}
			char* newbuf = (char*)realloc(newbuf,capacity);
			if(!newbuf){
				free(buffer);
				close(filefd);
				return {};
			}
			buffer = newbuf;
		}	
		memcpy(buffer + buff_used, temp_buff, nread);
		buff_used += nread;
	}
	std::string dev_data(buffer, buff_used);
	free(buffer);

	if(nread < 0) return {};

	//----------
	
	std::vector <struct InterfaceBandwidthStats> result;
	std::vector <std::string>		    tokens;
	std::istringstream stream(dev_data);
	std::string line;
	
	//Skiping Over Two Lines
	std::getline(stream, line);
	std::getline(stream, line);
	while(std::getline(stream,line)){
		tokens = split(line);
		
		struct InterfaceBindwidthStats Intfb;
		if(tokens.size() < 17 ) continue;
    
		Intfb.iface = tokens[0];
		Intfb.recived_bytes   = std::stoull(tokens[1]);
		Intfb.recived_packets = std::stoul(tokens[2]);
		Intfb.recived_drop    = std::stoul(tokens[4]);
		Intfb.transmitted_byte= std::stoull(tokens[9]);
		Intfb.transmited_packets=std::stoul(tokens[10]);
		Intfb.transmited_drop = std::stoul(tokens[12]);
			
		result.push_back(Intfb);
		
	}

	if(close(filefd) < 0){
       		return {};
	}
	return result;
}

/*
	EACCES	, ENOENT , EINVAL , ENOTDIR 
	Errors has to be Mannaged in 18-21 
*/
