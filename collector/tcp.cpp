cat tcpx.cpp
#include <unistd.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#ifdef __linux__
#define PROC_NET_TCP 	"/proc/net/tcp"
#define PAGE_SIZE 	4096
#endif

struct TcpEntry {
    unsigned int sl;    
    std::string local_address;
    std::string remote_address;
    unsigned int state;  
    unsigned int tx_queue;
    unsigned int rx_queue;
    unsigned int timer_active;
    unsigned int retransmits;
    unsigned int uid;     
    unsigned int drops;           
};

std::vector<std::string> split(std::string const &input){

	using namespace std;
        vector<string> packs;
        string token;
        istringstream input_stream(input);

        while(input_stream >> token){
                packs.push_back(token);
        }
        return packs;
}

 std::vector<struct TcpEntry> r_NetTcp(){

	int filefd = 0;
        char    temp_buff[PAGE_SIZE] = {0};
        ssize_t nread  = 0;
        char    *buffer = NULL;
        size_t  buff_used = 0;
        size_t  capacity = 0;

        if(access(PROC_NET_TCP, F_OK) == -1){ 
		std::cout << "F_OK"; 
		return{}; 
	}
        if(access(PROC_NET_TCP, R_OK) == -1){ 
		std::cout << "R_OK";
	      	return{};
	}

        filefd = open(PROC_NET_TCP, O_RDONLY | O_CLOEXEC);
        if(filefd == -1){
                return{};
	}
	while((nread = read(filefd, temp_buff, PAGE_SIZE)) > 0){
        	if(buff_used + nread > capacity){
                        capacity = capacity ? capacity * 2 : PAGE_SIZE;
                        while(capacity < buff_used + nread){
                                capacity *= 2;
                        }
                        char* newbuf = (char*)realloc(buffer,capacity);
                        if(!newbuf){
                                free(buffer);
                                close(filefd);
                                return{};
                        }
                        buffer = newbuf;
                }
                memcpy(buffer + buff_used, temp_buff, nread);
                buff_used += nread;
        }
        std::string dev_data(buffer, buff_used);
        free(buffer);

        if(nread < 0){		
		if(close(filefd) < 0){
                	return{};
        	}
	      
		return{};
	}
	//--------
	
	std::vector<struct TcpEntry> result;
	std::vector<std::string> tokens;
	std::istringstream stream(dev_data);
	std::string line;
	
	//skip over first line
	std::getline(stream, line);

	while(std::getline(stream, line)){

		tokens = split(line);
	
		//remove : from srn: 	
                auto colon = line.find(':');
                if(colon == std::string::npos) continue;
                std::string sno = tokens[0];
                if(!sno.empty() && sno.back() == ':'){
                        sno.pop_back();
                }

		struct TcpEntry Tcptable = {0};
		if(tokens.size() < 17) continue;

		Tcptable.sl = std::stoul(sno);

		//HEXA IP:PORT
		Tcptable.local_address = tokens[1];
		Tcptable.remote_address = tokens[2];
		Tcptable.state = std::stoul(tokens[3], nullptr, 16);

		//tokens[4] = "00000000:00000000"
		auto pos = tokens[4].find(':');
		if(pos != std::string::npos){
			Tcptable.tx_queue = std::stoul(tokens[4].substr(0,pos),nullptr,16);
			Tcptable.rx_queue = std::stoul(tokens[4].substr(pos+1),nullptr,16);
		}
		
		//TIME STATE
		pos = tokens[5].find(':');
		if(pos != std::string::npos){
			Tcptable.timer_active = std::stoul(tokens[5].substr(0,pos),nullptr,16);
		}

		Tcptable.retransmits = std::stoul(tokens[6],nullptr,16);
		Tcptable.uid = std::stoul(tokens[7]);
		Tcptable.drops = std::stoul(tokens[16]);

		result.push_back(Tcptable);
	
	}

	
	if(close(filefd) < 0){
                return{};
        }
	return result;
}

int main(){
	std::vector<struct TcpEntry> res;
        res = r_NetTcp();
	std::cout << res.size() << std::endl;
	for(auto& r: res){
		std::cout << r.sl << std::endl;
	}	
	return 0;
}
