#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "httpglue.h"

#define HEADER_LENGTH	128

#if 0
static int _test_chr(char ch) {
	if (ch >= 'a' && ch <= 'z')
		return 0;
	if (ch >= '0' && ch <= '9')
		return 0;
	if (ch >= 'A' && ch <= 'Z')
		return 0;
	if (ch == '-' || ch == '_' || ch == '.' || ch == '~')
		return 0;
	return 1;
}
#endif

static void _make_header(char *buff, char *key, char *value) {
	*buff = 0;
	if (strlen(key) + strlen(value) + 3 > HEADER_LENGTH)
		return;
	sprintf(buff, "%s: %s", key, value);
}


#if 0
static char *_url_escape(char *str, long len, long *newlen) {
	long i, len;
	char *newstr;

	for (i = len = 0; str[i]; i++, len++)
		if (_test_chr(str[i]))
			len += 2;
	newstr = malloc(len + 16);
	for (i = len = 0; str[i]; i++, len++) {
		if (_test_chr(str[i])) {
			newstr[len] = '%';
			sprintf(&newstr[len + 1], "%.2X", str[i]);
			len += 2;
		} else
			newstr[len] = str[i];
	}

	newstr[len] = 0;
	if (newlen)
		newlen = len;
	return newstr;
}
#endif


static size_t _write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	struct BSDLDHTTPResponse *response = userdata;
	
	response->data = realloc(response->data, response->data_len + nmemb + 1);
	if (!response->data)
		return response->data_len = 0;
	memcpy(response->data + response->data_len, ptr, nmemb);
	response->data_len += nmemb;
	((char *) response->data)[response->data_len] = 0;
	return nmemb;
}


void bsdld_httpglue_init() {
	curl_global_init(CURL_GLOBAL_ALL);
}


struct BSDLDHTTPResponse *bsdld_httpglue_request(struct BSDLDHTTPRequest *request) {
	CURL *c;
	CURLcode res;
	struct BSDLDHTTPResponse *response = malloc(sizeof(*response));
	struct curl_slist *list = NULL;
	char error[CURL_ERROR_SIZE + 1] = { 0 };
	char str[HEADER_LENGTH];
	
	if (!response)
		return response;

	response->data = NULL, response->data_len = 0;

	if (!(c = curl_easy_init())) {
		response->error = BSDLDHTTP_ERROR_INIT;
		response->data = strdup("Curl init error"), response->data_len = strlen(response->data);
		return response;
	}


	curl_easy_setopt(c, CURLOPT_URL, request->url);
	curl_easy_setopt(c, CURLOPT_ERRORBUFFER, error);
	curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, _write_callback);
	curl_easy_setopt(c, CURLOPT_WRITEDATA, response);
	
	_make_header(str, "Content-Type", (request->content_type) ? request->content_type : "application/json");
	list = curl_slist_append(list, str);
	if (request->auth_token)
		_make_header(str, "Authorization", request->auth_token), list = curl_slist_append(list, str);
	
	curl_easy_setopt(c, CURLOPT_HTTPHEADER, list);
	
	request->data_pos = 0;
	if (request->type == BSDLDHTTP_REQUEST_TYPE_POST) {
		curl_easy_setopt(c, CURLOPT_POSTFIELDS, request->data);
		curl_easy_setopt(c, CURLOPT_POSTFIELDSIZE, (long) request->data_len);
	} else if (request->type == BSDLDHTTP_REQUEST_TYPE_GET) {
	} else {
		response->error = BSDLDHTTP_ERROR_INTERNAL;
		response->data = strdup("invalid http method"), response->data_len = strlen(response->data);
		curl_easy_cleanup(c);
		return response;
	}

	if ((res = curl_easy_perform(c)) == CURLE_OK) {
		response->error = BSDLDHTTP_ERROR_OK;
		curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &response->response);
	} else {
		if (response->data)
			free(response->data);
		response->error = BSDLDHTTP_ERROR_EXEC;
		response->data = strdup(error);
		response->data_len = strlen(error);
	}
	
	curl_easy_cleanup(c);
	curl_slist_free_all(list);
	return response;
}


struct BSDLDHTTPResponse *bsdld_httpglue_response_free(struct BSDLDHTTPResponse *response) {
	free(response->data);
	free(response);
	return NULL;
}
