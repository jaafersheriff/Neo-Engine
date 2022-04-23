
#include "Util.hpp"

namespace neo {
	namespace util {

		inline bool fileExists(const char* fn) {
			FILE* fp;
			if (fn != NULL) {
				fp = fopen(fn, "rt");
				if (fp != NULL) {
					fclose(fp);
					return true;
				}
			}
			return false;
		}

		inline char* textFileRead(const char* fn) {
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
						content = (char*)malloc(sizeof(char) * (count + 1));
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

		inline int textFileWrite(const char* fn, char* s) {
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
	
	}
}
