#include <stdio.h> 
#include <pthread.h> 
#include <curl/curl.h> 

#define NUMT 4 

/* we have this global to let the callback get easy access to it */ 
static pthread_mutex_t *lockarray; 


#include <openssl/crypto.h> 
static void lock_callback(int mode, int type, char *file, int line) 
{ 
        (void)file; 
        (void)line; 
        if (mode & CRYPTO_LOCK) { 
                pthread_mutex_lock(&(lockarray[type])); 
        } 
        else { 
                pthread_mutex_unlock(&(lockarray[type])); 
        } 
} 

static unsigned long thread_id(void) 
{ 
        unsigned long ret; 

        ret=(unsigned long)pthread_self(); 
        return(ret); 
} 

 void init_locks(void) 
{ 
        int i; 
		//char * p1 ;
		//strcpy(p1,"hello");
        lockarray=(pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() * 
                        sizeof(pthread_mutex_t)); 
        for (i=0; i<CRYPTO_num_locks(); i++) { 
                pthread_mutex_init(&(lockarray[i]),NULL); 
        } 

        CRYPTO_set_id_callback((unsigned long (*)())thread_id); 
        CRYPTO_set_locking_callback((void (*)())lock_callback); 
} 

 void kill_locks(void) 
{ 
        int i; 

        CRYPTO_set_locking_callback(NULL); 
        for (i=0; i<CRYPTO_num_locks(); i++) 
                pthread_mutex_destroy(&(lockarray[i])); 

        OPENSSL_free(lockarray); 
} 


 void init_ysx_curl_ssl(void)
 {
    curl_global_init(CURL_GLOBAL_ALL);
	init_locks(); //openssl lock
	return ;
}

void uninit_ysx_curl_ssl(void)
{
	kill_locks();
	curl_global_cleanup();  //uinit   	curl_global_init();

	return ;	
}


#if 0
#ifdef USE_GNUTLS 
#include <gcrypt.h> 
#include <errno.h> 

GCRY_THREAD_OPTION_PTHREAD_IMPL; 

void init_locks(void) 
{ 
        gcry_control(GCRYCTL_SET_THREAD_CBS); 
} 

#define kill_locks() 
#endif 

/* List of URLs to fetch.*/ 
const char * const urls[]= { 
        "https://www.example.com/", 
        "https://www2.example.com/", 
        "https://www3.example.com/", 
        "https://www4.example.com/", 
}; 

static void *pull_one_url(void *url) 
{ 
        CURL *curl; 

        curl = curl_easy_init(); 
        curl_easy_setopt(curl, CURLOPT_URL, url); 
        /* this example doesn't verify the server's certificate, which means we 
         *          might be downloading stuff from an impostor */ 
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); 
        curl_easy_perform(curl); /* ignores error */ 
        curl_easy_cleanup(curl); 

        return NULL; 
} 

int main(int argc, char **argv) 
{ 
        pthread_t tid[NUMT]; 
        int i; 
        int error; 
        (void)argc; /* we don't use any arguments in this example */ 
        (void)argv; 

        /* Must initialize libcurl before any threads are started */ 
        curl_global_init(CURL_GLOBAL_ALL); 

        init_locks(); 

        for(i=0; i< NUMT; i++) { 
                error = pthread_create(&tid[i], 
                                NULL, /* default attributes please */ 
                                pull_one_url, 
                                (void *)urls[i]); 
                if(0 != error) 
                        fprintf(stderr, "Couldn't run thread number %d, errno %d\n", i, error); 
                else 
                        fprintf(stderr, "Thread %d, gets %s\n", i, urls[i]); 
        } 

        /* now wait for all threads to terminate */ 
        for(i=0; i< NUMT; i++) { 
                error = pthread_join(tid[i], NULL); 
                fprintf(stderr, "Thread %d terminated\n", i); 
        } 

        kill_locks(); 

        return 0; 
}
#endif
