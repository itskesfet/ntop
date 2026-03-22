#include "../../include/dev.hpp"
#include "../../utils/utilsfoo.hpp"

#ifdef __linux__
#define PROC_NET_DEV "/proc/net/dev"
#define PAGE_SIZE    4096
#endif

std::vector <struct InterfaceBandwidthStats> r_NetDev(){	
	
	int 	filefd = 0;
	char 	temp_buff[PAGE_SIZE] = {0};
	ssize_t nread  = 0;
	char    *buffer = NULL;
	size_t  buff_used = 0;
	size_t  capacity = 0;

	if(access(PROC_NET_DEV, F_OK) == -1){ std::cout << "F_OK"; return {};};
	if(access(PROC_NET_DEV, R_OK) == -1){ std::cout << "R_OK"; return {};};
	
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
	
	std::vector <struct InterfaceBandwidthStats> result;
	std::vector <std::string>		     tokens;
	std::istringstream stream(dev_data);
	std::string line;
	
	//Skiping Over Two Lines
	std::getline(stream, line);
	std::getline(stream, line);
	while(std::getline(stream,line)){
		
		tokens = split(line);

		//removing ':' from interface name formate
		auto colon = line.find(':');
		if(colon == std::string::npos) continue;
		std::string iface = tokens[0];
		if(!iface.empty() && iface.back() == ':'){
			iface.pop_back();
		}
		struct InterfaceBandwidthStats Intfb;
		if(tokens.size() < 17 ) continue;
		
		Intfb.iface 			= iface;
		Intfb.received_bytes   		= std::stoull(tokens[1]);
		Intfb.received_packets 		= std::stoul(tokens[2]);
		Intfb.received_drop    		= std::stoul(tokens[4]);
		Intfb.transmitted_byte		= std::stoull(tokens[9]);
		Intfb.transmited_packets	= std::stoul(tokens[10]);
		Intfb.transmited_drop 		= std::stoul(tokens[12]);
			
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



