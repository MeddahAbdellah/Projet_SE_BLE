#include <stdio.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
struct url_data 
{
    size_t size;
    char* data;
};
size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data)
{ 
    size_t index = data->size;
    size_t n = (size * nmemb);
    char* tmp;

    data->size += (size * nmemb);
  
    //fprintf(stderr, "data at %p size=%ld nmemb=%ld the data is %c\n", ptr, size, nmemb, data->data);
    
    tmp = (char*) realloc(data->data, data->size + 1); /* +1 for '\0' */
    if (tmp == NULL) {
        fputs("Error (re)allocating memory", stderr);
        return 0;
    }
    data->data = tmp;

    memcpy((data->data + index), ptr, n);
    data->data[data->size] = '\0';
fprintf(stderr, " %s\n",  data->data);
    return size * nmemb;
}
//size_t
//RecvResponseCallback ( char *ptr, size_t size, size_t nmemb, char *data ) {
  // handle received data
  //return size * nmemb;}
int main(void)
{
  
    char outputmessage[]="abcdefgh21";
    CURL *curl ;
     CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
if(curl) {
  
   struct url_data data;
    data.size = 0;
    data.data = (char*) malloc(4096);
  
  
  
  
        curl_easy_setopt(curl, CURLOPT_URL, "http://d02bc7a6.ngrok.io");
 
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, outputmessage);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(outputmessage));
       // curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, RecvResponseCallback );
         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
          
        curl_easy_perform(curl);
        
        
        curl_easy_cleanup(curl);
        }
curl_global_cleanup();
return 0;
}
