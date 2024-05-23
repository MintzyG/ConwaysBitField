#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char BSE(char rule[21]){
  char Sc = 0, Ec = 0;

  if (rule[0] != 'B'){ return 0; }
  for(int i = 0; i < 21; i++){
    if(rule[i] == 'S') {
      Sc++;
    }
    if(rule[i] == 'E') {
      Ec++;
    }
  }
  if (Sc != 1 || Ec != 1) return 0;
  return 1;
}

char NumberLength(char rule[21], short* B, short* S){
  (*B) = 0;
  (*S) = 0;
  int i = 1;
  while((*B) < 10){
    if(!isalpha(rule[i]) && !ispunct(rule[i])){ (*B)++; }
    else if (rule[i] == 'S'){ i++; break; }
    else{ return 0; }
    i++;
  }
  while((*S) < 10){
    if(!isalpha(rule[i]) && !ispunct(rule[i])){ (*S)++; }
    else if(rule[i] == 'E'){ break; }
    else{ return 0;}
    i++;
  }

  for (int i = 1; i < (*B) + 1; i++) {
    for (int j = 1; j < (*B) + 1; j++) {
      if(rule[i] == rule[j] && i != j){ return 0; }
    }
  }

  for (int i = (*B) + 2; i < ((*B) + 2) + (*S); i++) {
    for (int j = (*B) + 2; j < ((*B) + 2) + (*S); j++) {
      if(rule[i] == rule[j] && i != j){ return 0; }
    }
  }

  for (int i = 0; i < 21; i++) {
    if(rule[i] == '9'){ return 0; }
  }

  if((*S) >= 10 || (*B) >= 10){ return 0; }
  return 1;
}

int main(){
  char rule[21];
  short B = 0, S = 0;

  while(1){
    printf("Please input the ruleset for the game\n");
    scanf("%s21[^\n]", rule);
    scanf("%*c");
    if(BSE(rule)){
      if(NumberLength(rule, &B, &S)){
        break;
      }else{
        printf("Please enter a valid rule!\n");
        continue;
      }
    } else{
      printf("Please enter a valid rule!\n");
      continue;
    }
  }

  if (access("ruleset.c", F_OK) == 0) {
    remove("ruleset.c");
  }
  if (access("ruleset.h", F_OK) == 0) {
    remove("ruleset.h");
  }

  FILE* file;
  file = fopen("ruleset.h", "w");
  fprintf(file, "#pragma once\n\n");
  fprintf(file, "char RuleStateManager(char state, char cState);");
  fclose(file);

  file = fopen("ruleset.c", "w");
  fprintf(file, "#include \"ruleset.h\"\n\n");
  fprintf(file, "char RuleStateManager(char state, char cState) {\n");
  fprintf(file, "  if (cState == 0) {\n");

  // LOGICA DE IF PARA B
  short number = 0;
  for (int i = 0; i < B; i++) {
    if (i == 0) {
      number = strtoul(&rule[i+1], NULL, 10);
      fprintf(file, "    if (state == %d) {\n      return 1;\n    }", number);
    } else {
      number = strtoul(&rule[i+1], NULL, 10);
      fprintf(file, "    else if (state == %d) {\n      return 1;\n    }", number);
    }
  }

  fprintf(file, " else {\n");
  fprintf(file, "      return 0;\n    }\n");

  fprintf(file, "  } else {\n");

  // LOGICA DE IF PARA S

  fprintf(file, "  }\n}");
  fclose(file);
}
