#include <curl/curl.h>
#include <string.h>

#define MAX_URL_LENGTH 2048

/**
 * @brief Decodes possibly encoded URLs
 *
 * @param url URL to decode
 * @param decoded_url Pointer to store the decoded URL
 * @param max_url_length Max decoded URL length
 * @return int 0 in case of success, -1 otherwise
 */
int decode_url(char *url, char *decoded_url, int max_url_length) {
    CURL *curl = curl_easy_init();
    if (!curl) { return -1; }
    char *decoded = curl_easy_unescape(curl, url, 0, NULL);
    if (!decoded) {
        curl_easy_cleanup(curl);
        return -1;
    }
    strncpy(decoded_url, decoded, max_url_length);
    curl_free(decoded);
    curl_easy_cleanup(curl);
    return 0;
}

/**
 * @brief Encodes URLs
 *
 * @param url URL to encode
 * @param decoded_url Pointer to store the encoded URL
 * @param max_url_length Max encoded URL length
 * @return int 0 in case of success, -1 otherwise
 */
int encode_url(char *url, char *encoded_url, int max_url_length) {
    CURL *curl = curl_easy_init();
    if (!curl) { return -1; }
    char *encoded = curl_easy_escape(curl, url, strnlen(url, MAX_URL_LENGTH));
    if (!encoded) {
        curl_easy_cleanup(curl);
        return -1;
    }
    strncpy(encoded_url, encoded, max_url_length);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    return 0;
}
