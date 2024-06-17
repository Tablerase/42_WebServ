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
	string p = "coucou/abc/";
	if (p.find_last_of("/") != p.size() - 1) {
		p += "/";
	}
	cout << p << endl;
	cout << p.rfind("coucou");
}
