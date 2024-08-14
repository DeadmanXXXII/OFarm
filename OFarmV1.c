#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <regex.h>
#include <pthread.h>
#include <unistd.h>  // For sleep()

#define TOR_PROXY "socks5h://127.0.0.1:9050"
#define MAX_THREADS 10

struct MemoryStruct {
    char *memory;
    size_t size;
};

struct UrlData {
    char *url;
    struct UrlData *next;
};

pthread_mutex_t url_mutex = PTHREAD_MUTEX_INITIALIZER;
struct UrlData *url_list_head = NULL;

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

struct MemoryStruct fetch_page(const char *url) {
    CURL *curl;
    CURLcode res;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);  // will be grown as needed by the realloc
    chunk.size = 0;    // no data at this point

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_PROXY, TOR_PROXY);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);  // Set a timeout to avoid hanging

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    return chunk;
}

void extract_and_queue_onion_urls(const char *page_content, const char *origin_url) {
    regex_t regex;
    regmatch_t matches[2];
    const char *pattern = "([a-zA-Z2-7]{16}\\.onion)";
    
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        fprintf(stderr, "Could not compile regex\n");
        return;
    }

    const char *cursor = page_content;
    while (regexec(&regex, cursor, 2, matches, 0) == 0) {
        char url[24];
        snprintf(url, sizeof(url), "%.16s.onion", cursor + matches[1].rm_so);
        printf("Found .onion URL: %s (found on %s)\n", url, origin_url);

        // Add URL to the queue
        pthread_mutex_lock(&url_mutex);
        struct UrlData *new_url = (struct UrlData *)malloc(sizeof(struct UrlData));
        new_url->url = strdup(url);
        new_url->next = url_list_head;
        url_list_head = new_url;
        pthread_mutex_unlock(&url_mutex);

        cursor += matches[0].rm_eo;
    }

    regfree(&regex);
}

void *thread_func(void *arg) {
    while (1) {
        pthread_mutex_lock(&url_mutex);
        struct UrlData *current_url_data = url_list_head;
        if (current_url_data != NULL) {
            url_list_head = current_url_data->next;
        }
        pthread_mutex_unlock(&url_mutex);

        if (current_url_data == NULL) {
            sleep(1); // Wait for more URLs to be queued
            continue;
        }

        struct MemoryStruct page_content = fetch_page(current_url_data->url);

        if (page_content.memory) {
            extract_and_queue_onion_urls(page_content.memory, current_url_data->url);
            free(page_content.memory);
        }

        free(current_url_data->url);
        free(current_url_data);
    }

    return NULL;
}

void queue_initial_urls() {
    const char *seed_urls[] = {
        "http://msydqstlz2kzerdg.onion", // Ahmia
        "http://zqktlwi4fecvo6ri.onion", // The Hidden Wiki
        "http://deepweblinks.onion",     // Deep Web Links
        "http://onionlinksv3bdm5.onion", // OnionLinks
        "http://hss3uro2hsxfogfq.onion", // Not Evil
        NULL
    };

    for (int i = 0; seed_urls[i] != NULL; i++) {
        pthread_mutex_lock(&url_mutex);
        struct UrlData *new_url = (struct UrlData *)malloc(sizeof(struct UrlData));
        new_url->url = strdup(seed_urls[i]);
        new_url->next = url_list_head;
        url_list_head = new_url;
        pthread_mutex_unlock(&url_mutex);
    }
}

int main() {
    queue_initial_urls();

    pthread_t threads[MAX_THREADS];

    for (int i = 0; i < MAX_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, NULL)) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
