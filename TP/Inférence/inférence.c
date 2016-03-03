#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>
#include <math.h>

int main(int argc, char * argv[]){
	char * goal = argv[2];
	char * buffer = NULL;
	size_t size = 0;
	regex_t regexp;
	regmatch_t matches[3];
	regcomp(&regexp, "([0-9]+)[ \\t]+([0-9]+)", REG_EXTENDED);
	FILE * KB = fopen(argv[1], "r");
	getline(&buffer, &size, KB);
	if (!regexec(&regexp, buffer, 3, matches, 0)){
		int symbols, clauses;
		char * temp = strndup(buffer+matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
		symbols = (int)strtol(temp, NULL, 10);
		temp = strndup(buffer+matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);
		clauses = (int)strtol(temp, NULL, 10) + 1;
		int found = 0;
		while (getline(&buffer, &size, KB)>0){
			char * literal = strtok(buffer, " \t");
			if (abs(strtol(literal, NULL, 10)) == abs(strtol(goal, NULL, 10))) found = 1;
			while (literal = strtok(NULL, " \t")) {
				if (abs(strtol(literal, NULL, 10)) == abs(strtol(goal, NULL, 10))) found = 1;
			}
		}
		fclose(KB);
		if (!found) symbols++;
		KB = fopen(argv[1], "r");
		getline(&buffer, &size, KB);
		FILE * ubcsat = popen("./ubcsat -alg saps -r out null > results.txt", "w");
		fprintf(ubcsat, "p\tcnf\t%d\t%d\n", symbols, clauses);
		// printf("p\tcnf\t%d\t%d\n", symbols, clauses);
		while (getline(&buffer, &size, KB)>0) {
			// fputs(buffer, stdout);
			fputs(buffer, ubcsat);
		}
		// printf("%d\t0\n", -strtol(goal, NULL, 10));
		fprintf(ubcsat, "%d\t0\n", -strtol(goal, NULL, 10));
		pclose(ubcsat);
	}
	fclose(KB);
	free(buffer);
	struct stat fileInfos;
	stat("results.txt", &fileInfos);
	FILE * results = fopen("results.txt", "r");
	buffer = malloc(sizeof(char)*fileInfos.st_size);
	fread(buffer, sizeof(char), fileInfos.st_size, results);
	fclose(results);
	remove("results.txt");
	// printf("%d\n", fileInfos.st_size);
	// puts(buffer);
	regcomp(&regexp, "PercentSuccess = ([0-9]+\\.[0-9]+)", REG_EXTENDED);
	if (!regexec(&regexp, buffer, 2, matches, 0)) {
		char * answer = strtof(strndup(buffer+matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so), NULL) < 100 ? "La base de connaissances donnée peut inférer cette formule" : "La base de connaissances donnée ne peut pas inférer cette formule";
		puts(answer);
	}
	return 0;
}
