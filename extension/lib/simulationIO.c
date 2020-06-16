#include "simulationIO.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>



void printToTerminal(Grid grid, int gridLength, int gridHeight) {
  assert(gridLength >= 0);
  assert(gridHeight >= 0);

  GridCell cell;

  for (int i = 0; i < gridHeight; i++) {
    for (int j = 0; j < gridLength; j++) {
      cell = grid[j][i]; //assuming array goes [x][y]
      if (cell.human) {
	switch (cell.human->status) {
	case HEALTHY:
	  printf("H"); //healthy
	  break;
	case LATENT:
	  printf("L"); //carrier
	  break;
	case SICK:
	  printf("I"); //infected
	  break;
	default:
	  printf("E"); //error
	  break;
	}
      } else {
	printf("-"); //empty cell
      }
    }
    printf("\n");
  }
}

int getNextInput(char *input) {
  printf("How many turns do you want to wait? (q to quit)\n");
  return scanf("%9s", input);
}

static void getInt(char *buffer, int *value){
  if(!sscanf(buffer, "%*s%d", value)){
    perror("Configuration value not found\n");
    exit(EXIT_FAILURE);
  }
}

static void getDouble(char *buffer, double *value){
  if(!sscanf(buffer, "%*s%lf", value)){
    perror("Configuration value not found\n");
    exit(EXIT_FAILURE);
  }
}

void setInitial(Disease *disease, int *population, int *initiallyInfected,int *gridLength, int *gridHeight){

  *population = DEFAULT_POPULATION;
  *initiallyInfected = DEFAULT_INFECTED;
  *gridLength = GRID_SIZE;
  *gridHeight = GRID_SIZE;
  disease->latencyPeriod = LATENCY;
  disease->infectionChance = INF_CHANCE;
  disease->fatalityChance = FATAL_CHANCE;
  disease->recoveryChance = RECOVERY_CHANCE;
   
}

void configurate(Disease *disease, int *population, int *initiallyInfected, int *gridLength, int *gridHeight) {

  FILE *configFile;
    if((configFile = fopen("config.txt", "r" )) == NULL){
        printf("Error loading configuration file... Reverting to default...\n");
	setInitial(disease, population, initiallyInfected, gridLength, gridHeight);
    } else{
      char buffer[BUFFER_SIZE];

      //Code duplication to remove - this is a working version
      while(!feof(configFile)) {
  
	fgets(buffer, BUFFER_SIZE, configFile);
    /* Comments in config file are denoted with / */
	if(buffer[0] == '/') {
	  continue;
	} else if(strstr(buffer, "population")) {
	  getInt(buffer, population);
	} else if(strstr(buffer, "initially_infected")) {
	  getInt(buffer, initiallyInfected);
	} else if(strstr(buffer, "latency_period")){
	  getInt(buffer, &disease->latencyPeriod);      
	} else if(strstr(buffer, "length")){
	  getInt(buffer, gridLength);      
	} else if(strstr(buffer, "height")){
	  getInt(buffer, gridHeight);      
	} else if(strstr(buffer, "infection_rate")){
	  getDouble(buffer, &disease->infectionChance);
	} else if(strstr(buffer, "fatality_rate")){
	  getDouble(buffer, &disease->fatalityChance);
	} else if(strstr(buffer, "recovery_rate")){
	  getDouble(buffer, &disease->recoveryChance);
	} else {
	  printf("Configuration variable %s does not exist\n", buffer);
	}
      }
      fclose(configFile);
    }
    printConfigValues(disease, population, initiallyInfected, gridLength, gridHeight); 

}

void printConfigValues(Disease *disease, int *population, int *initiallyInfected, int *gridLength, int *gridHeight) {

  printf("Population is: %d\n", *population);
  printf("The number of initially infected is: %d\n", *initiallyInfected);
  printf("The latency period of the virus is: %d\n", disease->latencyPeriod);
  printf("The infection rate is: %lf\n", disease->infectionChance);
  printf("The fatality rate is: %lf\n", disease->fatalityChance);
  printf("The recovery chance is: %lf\n", disease->recoveryChance);
  printf("The grid width is: %d\n", *gridHeight);
  printf("The grid length is %d\n", *gridLength);

}
