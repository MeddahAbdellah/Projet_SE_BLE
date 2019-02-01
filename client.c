#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <wiringPi.h>




#define SHMOBJ_PATH         "/shared_memory"
#define SEMOBJ_PATH         "/semaphore"
sem_t * sem_id;

/* message structure for messages in the shared segment */

struct shared_data {
    char mac[20];
    int validation;
};

void signal_callback_handler(int signum);

int main(int argc, char *argv[]) {
    int shmfd;
    int shared_seg_size = (1 * sizeof(struct shared_data));   /* want shared segment capable of storing 1 message */
    struct shared_data *shared_msg;      /* the shared segment, and head of the messages list */

    if (wiringPiSetup () == -1)
    return 1 ;
 
    pinMode (0, OUTPUT) ;         
   
    signal(SIGINT, signal_callback_handler);

    /* creating the shared memory object    --  shm_open()  */
    shmfd = shm_open(SHMOBJ_PATH, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
    if (shmfd < 0)
    {
        perror("In shm_open()");
        exit(1);
    }

    fprintf(stderr, "Created shared memory object %s\n", SHMOBJ_PATH);

    /* adjusting mapped file size (make room for the whole segment to map)      --  ftruncate() */
    ftruncate(shmfd, shared_seg_size);

	sem_id=sem_open(SEMOBJ_PATH , O_CREAT, S_IRUSR | S_IWUSR, 1);


    /* requesting the shared segment    --  mmap() */
    shared_msg = (struct shared_data *)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shared_msg == NULL)
    {
        perror("In mmap()");
        exit(1);
    }
    fprintf(stderr, "Shared memory segment allocated correctly (%d bytes).\n", shared_seg_size);

    while(1)
    {sleep(2);
    sem_wait(sem_id);
    if(shared_msg->validation){
		digitalWrite (0, 1) ; 
		printf("The validation value is %d \n",shared_msg->validation);
		printf("led on\n");
	}else{
		digitalWrite (0, 0) ;
		printf("The validation value is %d \n",shared_msg->validation);
		printf("led off\n");
	}
    sem_post(sem_id);
    }

    if (shm_unlink(SHMOBJ_PATH) != 0) {
        perror("In shm_unlink()");
    }
        exit(1);
    
    if ( sem_close(sem_id) < 0 )
    {
    	perror("sem_close");
    }

   
    if ( sem_unlink(SEMOBJ_PATH) < 0 )
    {
    	perror("sem_unlink");
    }

    return 0;
}

void signal_callback_handler(int signum)
{

        
          //Semaphore unlink: Remove a named semaphore  from the system.
         
        if ( shm_unlink(SHMOBJ_PATH) < 0 )
        {
                perror("shm_unlink");
        }

        
         // Semaphore Close: Close a named semaphore
         
        if ( sem_close(sem_id) < 0 )
        {
        	perror("sem_close");
        }

        
         // Semaphore unlink: Remove a named semaphore  from the system.
         
        if ( sem_unlink(SEMOBJ_PATH) < 0 )
        {
        	perror("sem_unlink");
        }
   // Terminate program
   exit(signum);
} 
