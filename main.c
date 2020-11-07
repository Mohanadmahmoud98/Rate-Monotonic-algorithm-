#include "FreeRTOS.h"
#include "task.h"
#include "basic_io.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <time.h>
#include "queue.h"
#include <stdbool.h>

/*-----------------------------------------------------------*/

 typedef struct { //struct to be sent to the initial task 
	 int N; 
	 int latestArrival;
	 int maxComp;
	 int maxPeriodMul;
	 int tst;
 } TasksParams;
 
 typedef struct { //it is the paramter of each task
	 int arrival;
	 int computation;
	 int period;
	 float CPU;
	 float rate;
	 int priority;
 } Task;
 
 typedef struct { // paramters will be sent to each created tasks
	 int computationalTime;
	long lcm;
	int periodsTime;
	 int taskId;
	 int totaltimes;
	 
 } CurrentTaskParams;
 bool checker(int nooftasks,float arrcpu[]) // fn to check the utilization is less than or equal to the max utilization
{
	float maxcpu= nooftasks*((2^(1/nooftasks))-1);
	float sum=0;
	for(int i=0;i<nooftasks;i++)
	{
		sum+=arrcpu[i];
	}
	if(sum<=maxcpu)
		return true;
	else
		return false;
}
int gcd(int n1, int n2) // fn needed by find lcm fn
{ 
    if (n2 == 0) 
        return n1; 
    return gcd(n2, n1 % n2); 
} 
  
// Returns LCM of array elements 
long findlcm(int numbers[], int n) // Inspired by GeeksForGeeks.com , fn to find lcm to know how much time that all tasks will need
{
    long lcm = numbers[0]; 
		int i=1;
    while(i<n){
				lcm = (((numbers[i] * lcm))/(gcd(numbers[i], lcm))); 
				i++;
		}
  
    return lcm; 
} 
void swap(Task *task1, Task *task2) // swaping two tasks together , needed by sort fn
{  
    Task temp = *task1; 
    *task1 = *task2; 
    *task2 = temp; 
} 
void Sort(Task unsortedarray[], int size) // selection sort to sort the tasks by their periods
{
    int  minimumIndex; 
  
		int i=0;
		int j;
    while ( i < size-1) 
    { 

        minimumIndex = i; 

				j=i+1;
			
        while (j < size) 
			{
          if (unsortedarray[j].period < unsortedarray[minimumIndex].period) 
					{
            minimumIndex = j; 
						
					}
					 j++;
  
				}
        swap(&unsortedarray[minimumIndex], &unsortedarray[i]); 
				i++;
    } 
} 
static void intializeTasks( void *pvParameters );
static void taskHandler(void *pvParameters);

xQueueHandle xqueue;
int currentCount;
int main( void )

{
	
	static const TasksParams params = {5, 15, 8, 17,1};
	xqueue=xQueueCreate(params.N,sizeof(CurrentTaskParams));
	xTaskCreate( intializeTasks,(const signed portCHAR *) "Task initalizing", configMINIMAL_STACK_SIZE,(void *)&params, 1, NULL );
	vTaskStartScheduler();	

	for( ;; );
}

void fillArrayWithRands(int arr[], int size, int upperRange) { // fn to find random numbers for the arrivals and computational
	int i=0;
	srand(rand());
	while ( i < size) {
		
		arr[i] = (rand() % (upperRange)) + 1;
		i++;
	}
}

void intializeTasks( void *pvParameters )
{
	TasksParams params = *((TasksParams*)pvParameters);
	srand(rand());
	int n= (rand() % (params.N - 2 + 1)) + 2; // find the number of tasks randomly
	// make array with the number of randomized tasks
	int arrivals [n];
	int computations [n] ;
	int periods [n];
	Task tasks [n] ;
	bool flags=false;
	
	while(flags==false) //check on the utilization
	{
	fillArrayWithRands(arrivals, n, params.latestArrival);
	fillArrayWithRands(computations, n,  params.maxComp);
	for(int v=0;v<n;v++)
	{
		arrivals[v]=arrivals[v]*params.tst;
		computations[v]=computations[v]*params.tst;
	}	
	int j=0;
	//Safe mode
	while(j<n)
	{
		int upperRange= 3*(computations[j]*params.tst);
		int lowerRange= params.maxPeriodMul*(computations[j]*params.tst);
		periods[j]= (rand() % (upperRange - lowerRange + 1)) + lowerRange;
		j++;
	}
	//No Guarantee mode
	/*
	while(j<n)
		{
			int lowerRange=3*(computations[j]*params.tst);
			int upperRange=10*(computations[j]*params.tst);
			periods[j]=(rand()%(upperRange-lowerRange+1)+lowerRange);
			j++;
		}*/
	int i=0;
	while(i<n) { // fill the array of tasks
		tasks[i].arrival=arrivals[i]; 
		tasks[i].computation=computations[i];
		tasks[i].period=periods[i];
		tasks[i].CPU=((float)tasks[i].computation)/tasks[i].period; 
		tasks[i].rate=1/((float)tasks[i].period); 
		i++;
	}
	float CPUS [n];	
	for(int x=0;x<n;x++)
	{
		CPUS[x]=tasks[x].CPU;
	}
	if(checker(n,CPUS))
	{
		flags=true;
		
	}
		
}
	
	Sort(tasks,n);
int counterpr=0;
	for(int g = n; g>=1;g--)
 {

		tasks[counterpr].priority=g;
	 counterpr++;
	}
	counterpr=0;
	
	printf("ID \t A \t P \t C \t priority\n");
	for(int v=1;v<=n;v++)
	{
		printf("T%d) \t %d \t %d \t %d \t %d\n",v,tasks[v-1].arrival,tasks[v-1].period,tasks[v-1].computation,tasks[v-1].priority);
	
	}
	printf("---------------------------------------------\n");
	CurrentTaskParams currentTaskParams;
	currentTaskParams.lcm = findlcm(periods, params.N);
	
	for(int i = 0;i<n;i++) {
		currentTaskParams.periodsTime = tasks[i].period;
		currentTaskParams.computationalTime = tasks[i].computation;
		currentTaskParams.taskId = i+1;
		currentTaskParams.totaltimes=(currentTaskParams.lcm/currentTaskParams.periodsTime)*(currentTaskParams.computationalTime);
		
		xTaskCreate(taskHandler,(const signed portCHAR *) "Task ", configMINIMAL_STACK_SIZE,(void *)&currentTaskParams, tasks[i].priority+1, NULL );
	
	}
	// TO be deleted
	vTaskDelete(NULL);
}

void taskHandler(void *pvParameters)
{
	CurrentTaskParams currentTaskParams = *((CurrentTaskParams*)pvParameters);
//	unsigned portBASE_TYPE uxpriority=uxTaskPriorityGet(NULL);
//	int runningCount;
//	int periodsCount;
	portTickType xLastWakeTime;
	xLastWakeTime=xTaskGetTickCount();
	
int counter=0;
	for(;;){
		
		counter=0;
		printf("Task %d is running\n",currentTaskParams.taskId);
		printf("Task %d comp time is %d\n",currentTaskParams.taskId,currentTaskParams.computationalTime);
		printf("the %d Task starts at %d\n",currentTaskParams.taskId,(int)xLastWakeTime);
		printf("the Task %d priod is %d\n",currentTaskParams.taskId,(int)(currentTaskParams.periodsTime/portTICK_RATE_MS));
		printf("the %d Task should start again at %d\n",currentTaskParams.taskId,(((int)xLastWakeTime)+currentTaskParams.periodsTime));
		printf("-------------------------------------------------------------\n");
		for(int i=0;i<(currentTaskParams.computationalTime)*1000;i++)
		{
		}		
		vTaskDelayUntil(&xLastWakeTime,(currentTaskParams.periodsTime/portTICK_RATE_MS));
		
	}
}














