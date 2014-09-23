#include <iostream>
#include <fstream>
#include <string>

const int GPIO_MAP[] = {75, 91, 191, 24, 200, 90, 72, 101};

using namespace std;

class GPIO {
	
	bool is_enabled;
	string sys_interface;
	
public:
	
	GPIO(const unsigned int port, const string direction): is_enabled(false){
		
		if( init(port, direction) ){
			is_enabled = true;
		}
		
	}
	
	bool init(const unsigned int port, const string direction){
	
		ofstream sys_file;		
		string   sys_path("/sys/class/gpio/");
		
		sys_interface = sys_path + "gpio" + to_string(GPIO_MAP[port]) + "/";
		
		// check input variables
		if( port < 8 && ( (direction == "in") || (direction == "out") ) ){
		
			// enable port in system
			sys_file.open(sys_path + "export");
			
			if(sys_file.is_open()) {
				sys_file << GPIO_MAP[port] << endl;
				sys_file.close();
				
			} else {
				cout << "ERROR: GPIO could not open system file '/sys/class/gpio/export'" << endl;
				return false;
				
			}
		
			// set	direction
			
			sys_file.open(sys_interface + "direction");
			if(sys_file.is_open()) {
				sys_file << direction << endl;
				sys_file.close();
				
			} else {
				cout << "ERROR: GPIO could not set direction for " << sys_interface << endl;
				return false;
				
			}
			
			// set value to low
			sys_file.open(sys_interface + "value");
			if(sys_file.is_open()) {
				sys_file << 0 << endl;
				sys_file.close();
				
			} else {
				cout << "ERROR: GPIO could not set value for " << sys_interface << endl;
				return false;
				
			}			
			
		} else {
			cout << "ERROR: Wrong parameters for GPIO. Should be 0 <= port <= 7 and direction in ('in', 'out')" << endl;	
			
			return false;
		}
				
		return true;
	}
	
	void set(bool state){
	
		ofstream sys_file;
		
		if(is_enabled){
		
			sys_file.open(sys_interface + "value");
			if(sys_file.is_open()) {
				sys_file << int(state) << endl;
				sys_file.close();
		
			} else {
				cout << "ERROR: GPIO could not set value for " << sys_interface << endl;
		
			}
		} else {
		
			cout << "ERROR: GPIO was not enabled" << endl;
		}
	}
	
	void high(){
		set(true);
	}
	
	void low(){
		set(false);
	}

};
