/*BOUREGA Khadidja Yasmine
 *  A simple access control project with Raspberry pi.
contain four processes: wifi, ble ,client(LED) and servo

This code is compiled and run on the Raspberry as follows:
   
    gcc wifi.c -lrt -lcurl -lpthread -o wifi
    ./wifi

The process waits for the ble to send him the mac adress
*once received he send a connection request to the server.
The server receives the mac adress,check the database and sends back a response .
If the server send "found ",the door opens and the led turns on.
If the server send "not found",the door stay closed.
* 
* 
*  Original source file available at https://github.com/risingape/producer-consumer
*/



#include <stdio.h>// perror
#include <sys/mman.h>// shm_open, shm_unlink, mmap, munmap,
                      // PROT_READ, PROT_WRITE, MAP_SHARED, MAP_FAILED
#include <sys/types.h>/* exit() etc */
#include <unistd.h>// ftruncate, close
#include <fcntl.h>// O_RDWR, O_CREATE
#include <sys/stat.h> /* for random() stuff */
#include <stdlib.h>// malloc, free
#include <signal.h> // signal SIGINT
#include <semaphore.h> //sem_t
#include <curl/curl.h> //curl functions
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h> //strlen  memcpy
#include <errno.h>// errno, ENOENT


//define path 

#define SHMOBJ_PATH  "/shared_memory"    // name of the shared memory object

#define SEMOBJ_PATH   "/semaphore"       // name of the semaphores
sem_t * sem_id;

/* message structure for messages in the shared segment */
struct shared_data {
    char mac[20];               //shared variables
    int validation;
};
struct url_data 
{
    size_t size;
    char *data;
};
size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data)    /* ptr:pointer to a block of memory with a minimum size of size*nmemb bytes.  
                                                                                        *size − This is the size in bytes of each element to be read.

                                                                                        nmemb number of elements, each one with a size of size bytes.*/
{ 
    size_t index = data->size;
    size_t n = (size * nmemb);
    char* tmp;

    data->size += (size * nmemb);
    
    tmp = (char*) realloc(data->data, data->size + 1); /* +1 for '\0' */
    
    if (tmp == NULL) 
    {  /* out of memory! */ 
        fputs("Error (re)allocating memory", stderr);
        return 0;
    }
    data->data = tmp;

    memcpy((data->data + index), ptr, n);      //  copies n characters from memory area ptr to memory area data->data + index.
    
    data->data[data->size] = '\0';
    
    return size * nmemb;
}

/*void signal_callback_handler(int signum)
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
} */

    int main(int argc, char *argv[]) {
     
    
    int shmfd;
    int shared_seg_size = (1 * sizeof(struct shared_data));   /* want shared segment capable of storing 1 message */
    struct shared_data *shared_msg;      

           
   // signal(SIGINT, signal_callback_handler);

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


	sem_id=sem_open(SEMOBJ_PATH , O_CREAT, S_IRUSR | S_IWUSR, 1);         /* semaphores initially open to write initial message */


    /* requesting the shared segment    --  mmap() */
    shared_msg = (struct shared_data *)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shared_msg == NULL)
    {
        perror("In mmap()");
        exit(1);
    }
    fprintf(stderr, "Shared memory segment allocated correctly (%d bytes).\n", shared_seg_size);
            
            
    
    while(1)
    {
        CURL *curl ;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        struct url_data data;
        data.size = 0;
        data.data = (char*) malloc(4096);
    
        sleep(1);
        sem_wait(sem_id);  //locks the semaphore
  
            printf("\n the adress recieved is : ");
            printf("%s\n",shared_msg->mac);
   
        if(curl) {  //curl_easy_setopt is used to tell libcurl how to behave
                curl_easy_setopt(curl, CURLOPT_URL, "http://331506e4.ngrok.io");//provide the URL to use in the request
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, shared_msg->mac);//specify data to POST to server
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(shared_msg->mac));//size of POST data pointed to
                /* send all data to write_data */ 
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);//set callback for writing received data
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data); //custom pointer passed to the write callback
                curl_easy_perform(curl);
                /* always cleanup */
                curl_easy_cleanup(curl); 
 
                printf("the mac adress is %s onthe database\n", data.data);
            
                if  (data.data[0]=='f')  shared_msg->validation=1; //check if the received data is found or not found
           
        
                else shared_msg->validation=0; printf("The validation is %d \n",shared_msg->validation);
        
        
                }

    curl_global_cleanup();
    sem_post(sem_id);  //unlocks the semaphore
   
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


  
