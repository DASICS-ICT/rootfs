#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

int main(int argc, char *argv[]) {
    CURL *curl;
    CURLcode res;
    char *socks5h_proxy = "socks5h://127.0.0.1:10801"; // Default SOCKS5 proxy address
    char *http_url = "http://127.0.0.1:8000"; // Default HTTP URL

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Create a CURL handle
    curl = curl_easy_init();
    if(curl) {
        // Parse command line arguments for -s and -u options
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
                socks5h_proxy = argv[++i];
            } else if (strcmp(argv[i], "-u") == 0 && i + 1 < argc) {
                http_url = argv[++i];
            }
        }

        // Set the buffer size
        curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 16384);

        // Set the URL to work with
        curl_easy_setopt(curl, CURLOPT_URL, http_url);

        // Set the proxy server
        curl_easy_setopt(curl, CURLOPT_PROXY, socks5h_proxy);

        // Set the transfer rate limit
        curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, 1024L);

        // Enable detailed log output
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // Enable following of redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        // Cleanup
        curl_easy_cleanup(curl);
    }

    // Cleanup the libcurl global initialization
    curl_global_cleanup();

    return 0;
}