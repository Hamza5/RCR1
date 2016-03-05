#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>
#include <math.h>

#define RESULTS_FILE_PATH "_results.txt"

int main(int argc, char * argv[]){
	if (argc != 3) {
		fprintf(stderr, "Usage : %s <base_de_connaissance.cnf> <littéral(entier)>\n", argv[0]);
		return 7;
	}
	int goal = (int) strtol(argv[2], NULL, 10);
	if (!goal) { // The formule isn't an integer
		fputs("La formule doit être un littéral sous forme d'un entier !\n", stderr);
		return 5;
	}
	char * buffer = NULL;
	size_t size = 0;
	regex_t regexp;
	regmatch_t matches[3];
	regcomp(&regexp, "([0-9]+)[ \\t]+([0-9]+)", REG_EXTENDED); // To get the literals and the clauses
	FILE * KB = fopen(argv[1], "r"); // Knowledge base file in CNF format
	if (KB == NULL) {
		fprintf(stderr, "Impossible de trouver le fichier %s !\n", argv[1]);
		return 1;
	}
	getline(&buffer, &size, KB); // The header
	if (!regexec(&regexp, buffer, 3, matches, 0)){
		int symbols, clauses;
		char * temp = strndup(buffer+matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
		symbols = (int)strtol(temp, NULL, 10);
		temp = strndup(buffer+matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);
		clauses = (int)strtol(temp, NULL, 10) + 1;
		if (abs(goal) < 1 || abs(goal) > symbols+1) {
			fprintf(stderr, "La nouvelle formule pour cette base de connaissance doit être entre %d et %d !\n", 1, symbols+1);
			return 6;
		}
		int found = 0;
		while (getline(&buffer, &size, KB)>0){ // Search for the new formule
			char * literal = strtok(buffer, " \t");
			if (abs(strtol(literal, NULL, 10)) == abs(goal)) found = 1;
			while (literal = strtok(NULL, " \t")) {
				if (abs(strtol(literal, NULL, 10)) == abs(goal)) found = 1;
			}
		}
			fclose(KB);
			if (!found) symbols++;
			KB = fopen(argv[1], "r");
			if (KB == NULL) {
				fprintf(stderr, "Impossible de trouver le fichier %s !\n", argv[1]);
				return 1;
			}
			getline(&buffer, &size, KB);
			char cmd[34+strlen(RESULTS_FILE_PATH)];
			sprintf(cmd, "./ubcsat -alg saps -r out null > %s", RESULTS_FILE_PATH);
			FILE * ubcsat = popen(cmd, "w");
			if (ubcsat == NULL) {
				fputs("Impossible de trouver UBCSAT !\n", stderr);
				return 3;
			}
			fprintf(ubcsat, "p\tcnf\t%d\t%d\n", symbols, clauses);
			while (getline(&buffer, &size, KB)>0) {
				fputs(buffer, ubcsat);
			}
			fprintf(ubcsat, "%d\t0\n", -goal);
			pclose(ubcsat);
		fclose(KB);
		free(buffer);
		struct stat fileInfos;
		stat(RESULTS_FILE_PATH, &fileInfos); // Load the file attributes
		FILE * results = fopen(RESULTS_FILE_PATH, "r"); // Open the results
		if (results == NULL) {
			fputs("Impossible de trouver les résultats de UBCSAT !\n", stderr);
			return 4;
		}
		buffer = malloc(sizeof(char)*fileInfos.st_size);
		fread(buffer, sizeof(char), fileInfos.st_size, results); // Read the whole file
		fclose(results);
		remove(RESULTS_FILE_PATH); // Delete it from the disk
		regcomp(&regexp, "PercentSuccess = ([0-9]+\\.[0-9]+)", REG_EXTENDED); // The part that we need in the results
		if (!regexec(&regexp, buffer, 2, matches, 0)) {
			char * answer = strtof(strndup(buffer+matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so), NULL) < 100 ? "La base de connaissances donnée peut inférer cette formule" : "La base de connaissances donnée ne peut pas inférer cette formule";
			puts(answer);
			putc('\n', stdout);
		} else{
			fputs("Le fichier des résultats de UBCSAT est invalide !\n", stderr);
		}
	} else {
		fprintf(stderr, "L'entête du fichier %s est invalide !\n", argv[1]);
		return 2;
	}
	return 0;
}
