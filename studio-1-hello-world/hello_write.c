#include <unistd.h>

// Darcy Mupenda
// Feb 18 2026
// Prints hello world using write system

int main(int argc, char* argv[]){
	write(STDOUT_FILENO, "Hello, world!\n", 14);
	return 0;
}
