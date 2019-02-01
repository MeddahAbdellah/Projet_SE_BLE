#include <stdio.h> 
#include <sys/mman.h>
#include <string.h> 
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <string.h>
  
#define SHMOBJ_PATH         "/shared_memory"
#define SEMOBJ_PATH         "/semaphore"
sem_t * sem_id;  
/* message structure for messages in the shared segment */

struct shared_data {
    char mac[20];
    int validation;
};

void signal_callback_handler(int signum)
{

        /**
         * Shared Memory unlink: Remove a Shared Memory from the system.
         */
        if ( shm_unlink(SHMOBJ_PATH) < 0 )
        {
        	perror("shm_unlink");
        }
        /**
         * Semaphore Close: Close a named semaphore
         */
        if ( sem_close(sem_id) < 0 )
        {
        	perror("sem_close");
        }

        /**
         * Semaphore unlink: Remove a named semaphore  from the system.
         */
        if ( sem_unlink(SEMOBJ_PATH) < 0 )
        {
        	perror("sem_unlink");
        }

   // Terminate program
   exit(signum);
}

void substring(char s[], char sub[], int p, int l);

int main() 

{ 
    char * myfifo = "BLEfifo"; 
    char mac[80]; 
    int dataSize=0;
    int fd; 
    int shmfd;
    int shared_seg_size = (1 * sizeof(struct shared_data));   /* want shared segment capable of storing 1 message */
    struct shared_data *shared_msg;      /* the shared segment, and head of the messages list */

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
    
    /**
	 * Semaphore open
	 */
	sem_id=sem_open(SEMOBJ_PATH, O_CREAT, S_IRUSR | S_IWUSR, 1);

    /* requesting the shared segment    --  mmap() */
    shared_msg = (struct shared_data *)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shared_msg == NULL)
    {
        perror("In mmap()");
        exit(1);
    }
    fprintf(stderr, "Shared memory segment allocated correctly (%d bytes).\n", shared_seg_size);

    shared_msg->validation=0;

    while(1){ 

        // Open FIFO for Read only 
        fd = open(myfifo, O_RDONLY); 
        // Read from FIFO 
        dataSize=read(fd, mac, sizeof(mac)); 
        // Print the read message 
        if(dataSize>0)
            {
                substring(mac,mac,0,dataSize);
                printf("MAC Recived From BLE : %s\n", mac); 
                close(fd); 
                sem_wait(sem_id);
                shared_msg->validation=1;
                printf("The validation is %d \n",shared_msg->validation);
                strcpy(shared_msg->mac, mac );
                printf("The Shared Mac is : %s\n",shared_msg->mac);
                sem_post(sem_id);
            }else{
                close(fd); 
            }
    } 
        if (shm_unlink(SHMOBJ_PATH) != 0) {
            perror("In shm_unlink()");
            exit(1);
        }
        /**
         * Semaphore Close: Close a named semaphore
         */
        if ( sem_close(sem_id) < 0 )
        {
            perror("sem_close");
        }

        /**
         * Semaphore unlink: Remove a named semaphore  from the system.
         */
        if ( sem_unlink(SEMOBJ_PATH) < 0 )
        {
            perror("sem_unlink");
        }
        

    return 0; 

} 

void substring(char s[], char sub[], int p, int l) {
   int c = 0;
   
   while (c < l) {
      sub[c] = s[p+c];
      c++;
   }
   sub[c] = '\0';
}
