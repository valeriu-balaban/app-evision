#ifndef GPIO_H
#define GPIO_H

#include <iostream>
#include <fstream>
#include <string>

const int GPIO_MAP[] = {75, 91, 191, 24, 200, 90, 72, 101};

class GPIO {
	
	bool is_enabled;
	std::string sys_interface;
	std::ofstream sys_file; 

public:
	
	GPIO(const unsigned int port, const std::string direction): is_enabled(false){
		
		if( init(port, direction) ){
			is_enabled = true;
		}
		
	}
	
	bool init(const unsigned int port, const std::string direction){
	
		std::ofstream sys_file;		
		std::string   sys_path("/sys/class/gpio/");
		
		sys_interface = sys_path + "gpio" + std::to_string(GPIO_MAP[port]) + "/";
		
		// check input variables
		if( port < 8 && ( (direction == "in") || (direction == "out") ) ){
		
			// enable port in system
			sys_file.open(sys_path + "export");
			
			if(sys_file.is_open()) {
				sys_file << GPIO_MAP[port] <<std:: endl;
				sys_file.close();
				
			} else {
				std::cout << "ERROR: GPIO could not open system file '/sys/class/gpio/export'" << std::endl;
				return false;
				
			}
		
			// set	direction
			
			sys_file.open(sys_interface + "direction");
			if(sys_file.is_open()) {
				sys_file << direction << std::endl;
				sys_file.close();
				
			} else {
				std::cout << "ERROR: GPIO could not set direction for " << sys_interface << std::endl;
				return false;
				
			}
			
			// set value to low
			sys_file.open(sys_interface + "value");
			if(sys_file.is_open()) {
				sys_file << 0 << std::endl;
				sys_file.close();
				
			} else {
				std::cout << "ERROR: GPIO could not set value for " << sys_interface << std::endl;
				return false;
				
			}			
			
		} else {
			std::cout << "ERROR: Wrong parameters for GPIO. Should be 0 <= port <= 7 and direction in ('in', 'out')" << std::endl;	
			
			return false;
		}
				
		return true;
	}
	
	void set(bool state){

		if(is_enabled){
			if(!sys_file.is_open())
				sys_file.open(sys_interface + "value");

			if(sys_file.is_open()) {
				sys_file << int(state) << std::endl;
				sys_file.flush();
		
			} else {
				std::cout << "ERROR: GPIO could not set value for " << sys_interface << std::endl;
		
			}
		} else {
		
			std::cout << "ERROR: GPIO was not enabled" << std::endl;
		}
	}
	
	void high(){
		set(true);
	}
	
	void low(){
		set(false);
	}

};

#endif
