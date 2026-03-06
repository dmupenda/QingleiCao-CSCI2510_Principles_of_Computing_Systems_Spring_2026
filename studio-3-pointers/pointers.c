#include <stdio.h>
#include <stdlib.h>

//print string in reverse
void printReverse(char* string){

  int length = 0;

  while(string[length] != '\0'){

      length++; 
  }

  for(int i=length-1; i>=0; i--){
      printf("%c\n", string[i]);
  }
}

//reverse copy of a string
char* reverseString(char* input){

  int length = 0;

  while(input[length] != '\0'){
      length++;
  }

  char* output = (char*)malloc(length+1);

  for(int i=0; i<length; i++){
      output[i]= input[length-1-i];
  }
  
  output[length]='\0';

  return output;

}

int main() {
    char *messagePtr = "HELLOWORLD!";

    //print string in reverse
    printReverse(messagePtr);

    //copy of a string
    char* reversedMessage = reverseString(messagePtr);
    printf("Reversed string: %s\n", reversedMessage);
    
    return 0;
}

