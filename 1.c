#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

char formula[10000];
int pos = 0;

char variables[26];
int var_count = 0;

int values[26];

void add_variable(char c) {
	c = toupper(c);

	for (int i = 0; i < var_count; ++i) {
		if (variables[i] == c) {
			return;
		}
	}

	variables[var_count++] = c;
}

void find_variables() {
    for (int i = 0; formula[i] != '\0'; i++) {
        if (isalpha(formula[i])) {
            add_variable(formula[i]);
        }
    }
}

int compare_chars(const void* a, const void* b) {
	char char_a = *(char*)a;
	char char_b = *(char*)b;

	return char_a - char_b;
}

/*
	ПОРЯДОК ДЕЙСТВИЙ И ИХ ОБОЗНАЧЕНИЯ
	!       НЕ
	&       И
	|       ИЛИ
	^       XOR
	~       Эквивалентность
	>       Импликация
	<       Коимпликация
	+       NOR
	-       NAND
*/ 

void skip_spaces() {
	while (formula[pos] == ' ' || formula[pos] == '\t' ||
		formula[pos] == '\n' || formula[pos] == '\r') {
		pos++;
	}
}

int base() {
	skip_spaces();

	if (formula[pos] == '(') {
		++pos;
		int res = ifnand();
		skip_spaces();

		if (formula[pos] == ')') {
			++pos;
		}

		return res;
	}

	if (formula[pos] == '0') {
		++pos;
		return 0;
	}

	if (formula[pos] == '1') {
		++pos;
		return 1;
	}

	if (isalpha(formula[pos])) {
		char c = toupper(formula[pos]);
		++pos;

		return values[c];
	}

	return 0;
}

int ifnot() {
	skip_spaces();

	if (formula[pos] == '!') {
		pos++;
		return !ifnot();
	}

	return base();
}

int ifand() {
	int left = ifnot();
	while (1) {
		skip_spaces();

		if (formula[pos] != '&') {
			break;
		}

		pos++;

		int right = ifnot();
		left = left && right;
	}

	return left;
}



int ifor() {
	int left = ifand();
	while (1) {
		skip_spaces();

		if (formula[pos] != '|') {
			break;
		}

		++pos;

		int right = ifand();
		left = (left || right);
	}

	return left;
}

int ifxor() {
	int left = ifor();
	while (1) {
		skip_spaces();

		if (formula[pos] != '^') {
			break;
		}

		++pos;

		int right = ifor();
		left = left ^ right;
	}

	return left;
}

int ifeq() {
	int left = ifxor();
	while (1) {
		skip_spaces();

		if (formula[pos] != '~') {
			break;
		}

		++pos;

		int right = ifxor();
		left = (left == right);
	}

	return left;
}

int ifimpl() {
	int left = ifeq();
	while (1) {
		skip_spaces();

		if (formula[pos] != '>') {
			break;
		}

		++pos;

		int right = ifeq();
		left = (!left) || right;
	}

	return left;
}

int ifcoimpl() {
	int left = ifimpl();
	while (1) {
		skip_spaces();

		if (formula[pos] != '<') {
			break;
		}

		++pos;

		int right = ifimpl();
		left = left || (!right);
	}

	return left;
}

int ifnor() {
	int left = ifcoimpl();
	while (1) {
		skip_spaces();

		if (formula[pos] != '+') {
			break;
		}

		++pos;

		int right = ifcoimpl();
		left = !(left||right);
	}

	return left;
}

int ifnand() {
	int left = ifnor();
	while (1) {
		skip_spaces();

		if (formula[pos] != '-') {
			break;
		}

		++pos;

		int right = ifnor();
		left = !(left && right);
	}

	return left;
}

int eval() {
	pos = 0;
	return ifnand();
}

int main() {
	FILE* fp = fopen("formula.txt", "r");
	if (fp == NULL) {
		printf("Error opening the file!\n");
		return 1;
	}
	
	size_t len = fread(formula, 1, 10000, fp);
	formula[len] = '\0';
	
	find_variables();
	qsort(variables, var_count, sizeof(char), compare_chars);


	for (int i = 0; i < var_count; i++) {
		printf("  %c  |", variables[i]);
	}
	printf("  F\n");
	for (int i = 0; i < var_count*9; i++) {
		printf("-");
	}
	printf("\n");

	char PDNF[10000] = "";
	char PKNF[10000] = "";
	const unsigned long long combinations = 1ULL << var_count;

	for (unsigned long long mask=0; mask < combinations; ++mask) {
		for (int i = 0; i < var_count; ++i) {
			int bit = (mask >> (var_count - 1 - i)) & 1;

			values[variables[i]] = bit;
		}
		for (int i = 0; i < var_count; ++i) {
			printf("  %d  |", values[variables[i]]);
		}



		int result = eval();
		printf("  %d \n", result);
		if (result == 1) {
			if (strlen(PDNF)!=0) {
				strncat_s(PDNF, 10000, "|", 1);
			}
			strncat_s(PDNF, 10000, "(", 1);
			for (int i = 0; i < var_count; ++i) {
				char temp[2] = { variables[i], '\0' };
				if (values[variables[i]] == 1) {
					strncat_s(PDNF, 10000, temp, 1);
				}
				else {
					strncat_s(PDNF, 10000, "!", 1);
					strncat_s(PDNF, 10000, temp, 1);
				}
				if(i+1!=var_count)
					strncat_s(PDNF, 10000, "&", 1);
			}
			strncat_s(PDNF, 10000, ")", 2);
			
		}

		if (result == 0) {
			if (strlen(PKNF) != 0) {
				strncat_s(PKNF, 10000, "&", 1);
			}
			strncat_s(PKNF, 10000, "(", 1);
			for (int i = 0; i < var_count; ++i) {
				char temp[2] = { variables[i], '\0' };
				if (values[variables[i]] == 0) {
					strncat_s(PKNF, 10000, temp, 1);
				}
				else {
					strncat_s(PKNF, 10000, "!", 1);
					strncat_s(PKNF, 10000, temp, 1);
				}
				if (i + 1 != var_count)
					strncat_s(PKNF, 10000, "|", 1);
			}
			strncat_s(PKNF, 10000, ")", 2);

		}
	}
	if (strlen(PDNF) != 0) {
		printf("PDNF: %s", PDNF);
	}
	else {
		printf("Impossible to create a PDNF.");
	}
	if (strlen(PKNF) != 0) {
		printf("\nPKNF: %s", PKNF);
	}
	else {
		printf("\nImpossible to create a PKNF.");
	}
	
	return 0;
}