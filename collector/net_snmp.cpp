//To be upgraded to handel other protocols

#include "../../include/snmp.hpp"
#include "../../utils/utilsfoo.hpp"

#ifdef __linux__
#define PROC_NET_SNMP 	"/proc/net/snmp"
#define PAGE_SIZE 	4096
#endif

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
	 
	
	while(1){
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
		ipstat.incoming_discarded_packets   = std::stoull(tokens[8]);
		ipstat.packets_delivered_to_transport = std::stoull(tokens[9]);
		ipstat.total_packets_sent           = std::stoull(tokens[10]);
		ipstat.outgoing_discarded_packets   = std::stoull(tokens[11]);
		ipstat.packets_with_no_route        = std::stoull(tokens[12]);
		ipstat.total_packets_transmitted    = std::stoull(tokens[20]);
	result.push_back(ipstat);
	}

	close(filefd);
return result;
}




