//TO be upgraded for Handling other protocols ...
#include <unistd.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <fcntl.h>
#include <cstring>
#include <cstdint>
#include <unistd.h>

#ifdef __linux__
#define PROC_NET_SNMP 	"/proc/net/snmp"
#define PAGE_SIZE 	4096
#endif

struct IpStats {
    uint8_t  is_forwarding_enabled;        // Router forwarding?
    uint8_t  default_ttl;                  // TTL

    uint64_t total_packets_received;   
    uint64_t header_error_packets;      
    uint64_t address_error_packets; 

    uint64_t unknown_protocol_packets;    
    uint64_t incoming_discarded_packets;  
    uint64_t packets_delivered_to_transport; 

    uint64_t total_packets_sent;          
    uint64_t outgoing_discarded_packets; 
    uint64_t packets_with_no_route; 

    uint64_t reassembly_failed_packets;   
    uint64_t total_packets_transmitted;   
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

std::vector<struct IpStats> r_snmp(){
	
	int 	filefd = 0;
        char    temp_buff[PAGE_SIZE] = {0};
        ssize_t nread  = 0;
        char    *buffer = NULL;
        size_t  buff_used = 0;
        size_t 	capacity = 0;
	
	
	if(access(PROC_NET_SNMP, F_OK) == -1){ 
		std::cout << "F_OK"; 
		return{}; 
	}
        if(access(PROC_NET_SNMP, R_OK) == -1){ 
		std::cout << "R_OK";
	      	return{};
	}

        filefd = open(PROC_NET_SNMP, O_RDONLY | O_CLOEXEC);
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

	std::vector<struct IpStats> result; 
	std::vector<std::string> tokens;
	std::istringstream stream(dev_data);
       	std::string header,line;
	 
	
	while(true){
		if(!std::getline(stream, header)) break;
		if(!std::getline(stream, line)) break;
		
		IpStats ipstat {0}; 
		tokens = split(line); 
		if (tokens.empty()) continue;
			
		//remove : from srn: 
		auto colon = tokens[0].find(':'); 
		if(colon == std::string::npos) continue; 
		std::string sno = tokens[0]; 
		if(!sno.empty() && sno.back() == ':'){ 
			sno.pop_back(); 
		} 
			
		if(tokens.size() < 21 ) continue;
		if(sno != "Ip") continue;

		ipstat.is_forwarding_enabled        = static_cast<uint8_t>(std::stoul(tokens[1]));
		ipstat.default_ttl                  = static_cast<uint8_t>(std::stoul(tokens[2]));

		ipstat.total_packets_received       = std::stoull(tokens[3]);
		ipstat.header_error_packets         = std::stoull(tokens[4]);
		ipstat.address_error_packets        = std::stoull(tokens[5]);

		ipstat.unknown_protocol_packets     = std::stoull(tokens[7]);
		ipstat.incoming_discarded_packets   = std::stoull(tokens[8]);
		ipstat.packets_delivered_to_transport = std::stoull(tokens[9]);

		ipstat.total_packets_sent           = std::stoull(tokens[10]);
		ipstat.outgoing_discarded_packets   = std::stoull(tokens[11]);
		ipstat.packets_with_no_route        = std::stoull(tokens[12]);

		ipstat.reassembly_failed_packets    = std::stoull(tokens[16]);
		ipstat.total_packets_transmitted    = std::stoull(tokens[20]);
	result.push_back(ipstat);
	}

	close(filefd);
	
return result;
}

//For testing ...
/* int main() {
    std::vector<IpStats> stats = r_snmp();

    if (stats.empty()) {
        std::cout << "No IP stats parsed.\n";
        return 1;
    }
    for (const auto& ip : stats) {
        std::cout << "Forwarding: " << (int)ip.is_forwarding_enabled << "\n";
    }
    return 0;
}
*/



