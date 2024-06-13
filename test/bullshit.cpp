#include <algorithm>
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <map>
#include <vector>
#include <unistd.h>
using namespace std;


int main(int ac, char **av, char **envp) {
	int status;
	pid_t pid = fork();
	if (pid == 0) {
		execve(av[1], av + 1, envp);	
		return 1;
	} else {
		waitpid(pid, &status, 0);
	}
	cout << WIFEXITED(status) << endl;
}
