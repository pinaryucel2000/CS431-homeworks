#include <REG51.H>
#include <inttypes.h>

#define MAX_SIZE 4

struct Array {
  int8_t array[MAX_SIZE];
  int8_t currentSize;
  int8_t leftIndex;
	int8_t rightIndex;
};

struct Array Array;

int8_t append_left(int8_t newValue){
	int8_t i;
	
	if(Array.currentSize == MAX_SIZE){
		return -1;
	}
	else if(Array.leftIndex == 0){
		
		for(i = Array.currentSize; i > 0; i--){
				Array.array[i] = Array.array[i-1];
		}
		
		Array.array[0] = newValue;
		Array.rightIndex = 	Array.rightIndex + 1;
	}
	else if(Array.leftIndex == -1){
		Array.leftIndex = 0;
		Array.rightIndex = 0;
		Array.array[0] = newValue;	
	}
	else{
		Array.leftIndex = Array.leftIndex - 1;
		Array.array[Array.leftIndex] = newValue;
	}
	
	Array.currentSize = Array.currentSize + 1;
	return Array.currentSize;
}

int8_t append_right(int8_t newValue){
	int8_t i;
	
	if(Array.currentSize == MAX_SIZE){
		return -1;
	}
	else if(Array.rightIndex == MAX_SIZE - 1){
		
		for(i = Array.leftIndex - 1; i < MAX_SIZE - 1; i++){
				Array.array[i] = Array.array[i+1];
		}
		
		Array.array[MAX_SIZE - 1] = newValue;
		Array.leftIndex = 	Array.leftIndex - 1;
	}
	else if(Array.rightIndex == -1){
		Array.leftIndex = 0;
		Array.rightIndex = 0;
		Array.array[0] = newValue;	
	}
	else{
		Array.rightIndex = Array.rightIndex + 1;
		Array.array[Array.rightIndex] = newValue;
	}
	
	Array.currentSize = Array.currentSize + 1;
	return Array.currentSize;
}

int8_t pop_left(int8_t* currentSize){
	int8_t value;
	
	if(Array.currentSize == 0){
		*currentSize = 0;
		return -1;
	}	
	
	value = Array.array[Array.leftIndex];
	
	if(Array.currentSize == 1){
		*currentSize = 0;
		Array.leftIndex = -1;
		Array.rightIndex = -1;
		Array.currentSize = 0;
	}	
	else{
		Array.leftIndex = Array.leftIndex + 1;
		Array.currentSize = Array.currentSize - 1;
		*currentSize = Array.currentSize;
	}
	
	return value;
}

int8_t  pop_right(int8_t* currentSize){
	int8_t value;
	
	if(Array.currentSize == 0){
		*currentSize = 0;
		return -1;
	}	
	
	value = Array.array[Array.rightIndex];
	
	if(Array.currentSize == 1){
		*currentSize = 0;
		Array.leftIndex = -1;
		Array.rightIndex = -1;
		Array.currentSize = 0;
	}	
	else{
		Array.rightIndex = Array.rightIndex - 1;
		Array.currentSize = Array.currentSize - 1;
		*currentSize = Array.currentSize;
	}
	
	return value;
}

int8_t seek_left(int8_t* currentSize){
	*currentSize = Array.currentSize;
	
	if(Array.currentSize != 0){
		return Array.array[Array.leftIndex];
	}
	else{
		return -1;
	}
}

int8_t seek_right(int8_t* currentSize){
	*currentSize = Array.currentSize;
	
	if(Array.currentSize != 0){
		return Array.array[Array.rightIndex];
	}
	else{
		return -1;
	}
}

int8_t length(){
	return Array.currentSize;
}

void main (void)  {
	int8_t result = 0xAA; // for easy detection in the RAM
	int8_t parameter = 0xBB; // for easy detection in the RAM
	Array.leftIndex = -1;
	Array.rightIndex = -1;
	Array.currentSize = 0;
	
  while (1) { /* Loop forever */
		if(Array.currentSize <= MAX_SIZE)
		{
			result = append_left(0xCC);
			result = append_right(0xDD);
			result = seek_right(&parameter);
			result = append_left(0xEE);
			result = append_right(0xFF);
			result = append_left(0x11);
			result = seek_left(&parameter);
			result = length();
			
			result = pop_left(&parameter);
			result = pop_right(&parameter);
			result = pop_left(&parameter);
			result = pop_right(&parameter);
			result = pop_left(&parameter);
			
			result = seek_right(&parameter);
			result = seek_left(&parameter);
		}
		

  }
}
