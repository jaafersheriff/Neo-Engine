#pragma once

#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <sys/stat.h>

namespace neo {
	namespace util {

		static inline bool fileExists(const char* path) {
			std::ifstream f(path);
			return f.good();
		}

		static inline char* textFileRead(const char* fn) {
			FILE* fp;
			char* content = NULL;
			int count = 0;
			if (fn != NULL) {
				fp = fopen(fn, "rt");
				if (fp != NULL) {
					fseek(fp, 0, SEEK_END);
					count = (int)ftell(fp);
					rewind(fp);
					if (count > 0) {
						content = new char[count + 1];
						count = (int)fread(content, sizeof(char), count, fp);
						content[count] = '\0';
					}
					fclose(fp);
				}
				else {
					printf("error loading %s\n", fn);
				}
			}
			return content;
		}

		static inline int textFileWrite(const char* fn, char* s) {
			FILE* fp;
			int status = 0;
			if (fn != NULL) {
				fopen_s(&fp, fn, "w");
				if (fp != NULL) {
					if (fwrite(s, sizeof(char), strlen(s), fp) == strlen(s)) {
						status = 1;
					}
					fclose(fp);
				}
			}
			return(status);
		}


		static inline time_t getFileModTime(const char* fn) {
			struct stat fileInfo;
			if (stat(fn, &fileInfo) == 0) {
				return fileInfo.st_mtime;
			}
			return 0;
		}
	}
}
