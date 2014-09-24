#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <time.h>

#ifndef TRACER_H
#define TRACER_H

#define TRACER_CLOCK	CLOCK_THREAD_CPUTIME_ID

class Tracer{

	// Private struct
	struct Event{
		string 		name;
		long long	min;
		long long	max;
		long long	total;
		long		count;
	};
	
	// Private variables
	std::vector<Event>	event_list;
	timespec			start_time;
	
public:
	
	void start(){
		clock_gettime(TRACER_CLOCK, &start_time);
	}
	
	void event(string name){
		std::vector<Event>::iterator i;
		timespec event_time;
		long elapsed_time;
		
		clock_gettime(TRACER_CLOCK, &event_time);
		
		if(event_time.tv_nsec < start_time.tv_nsec){
			// wrong returned time
			return ;
		} else {
			elapsed_time = event_time.tv_nsec - start_time.tv_nsec;
		}
		
		// find name in list
		for (i = event_list.begin(); i != event_list.end(); ++i) {
		    if (i->name == name) {
		        
		        // update values
		        if(i->min > elapsed_time){
		        	i->min 	= elapsed_time;
		        }
		        
		        if(i->max < elapsed_time){
		        	i->max 	= elapsed_time;
		        }
		        
		        i->total += elapsed_time;
		        i->count++;
		        
		        break;
		    }
		}
		
		// append event if not in the list
		if(i == event_list.end()){
			Event new_event;
			
			new_event.name 	= name;
			new_event.min 	= elapsed_time;
			new_event.max 	= elapsed_time;
			new_event.total = elapsed_time;
			new_event.count = 1;
			
			event_list.insert(event_list.end(), new_event);
		}
		
		clock_gettime(TRACER_CLOCK, &start_time);
	}
	
	void end(){
		std::vector<Event>::iterator i;	
		long total_min = 0, total_max = 0, total_avg = 0;	
		
		std::cout << std::setw(20) << "Event Name";
		std::cout << std::setw(15) << "Min Time [us]";
		std::cout << std::setw(15) << "Max Time [us]";
		std::cout << std::setw(15) << "Avg Time [us]";
		std::cout << std::endl;
		
		std::cout << std::setfill('-') << std::setw(69) << "-" << std::endl;
		std::cout << std::setfill(' ');
		
		for (i = event_list.begin(); i != event_list.end(); ++i){
			std::cout << std::setw(20) << i->name;
			std::cout << std::setw(15) << i->min / 1000;
			std::cout << std::setw(15) << i->max / 1000;
			std::cout << std::setw(15) << (i->total / i->count) / 1000;
			std::cout << std::endl;
			
			total_min += i->min / 1000;
			total_max += i->max / 1000;
			total_avg += (i->total / i->count) / 1000; 			
		}
		
		std::cout << std::setfill('-') << std::setw(69) << "-" << std::endl;
		std::cout << std::setfill(' ');
		std::cout << std::setw(20) << "Total";
		std::cout << std::setw(15) << total_min;
		std::cout << std::setw(15) << total_max;
		std::cout << std::setw(15) << total_avg;
		std::cout << std::endl;
	}

};

#endif
