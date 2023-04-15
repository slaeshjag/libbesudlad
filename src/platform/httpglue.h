#ifndef HTTPGLUE_H_
#define	HTTPGLUE_H_


enum BSDLDHTTPError {
	BSDLDHTTP_ERROR_OK,
	BSDLDHTTP_ERROR_INIT,
	BSDLDHTTP_ERROR_EXEC,
	BSDLDHTTP_ERROR_INTERNAL,
};


enum BSDLDHTTPRequestType {
	BSDLDHTTP_REQUEST_TYPE_POST,
	BSDLDHTTP_REQUEST_TYPE_GET,
};


struct BSDLDHTTPRequest {
	enum BSDLDHTTPRequestType	type;
	char				*url;
	char				*auth_token;
	char				*data;
	char				*content_type;
	long				data_len;
	long				data_pos;
};


struct BSDLDHTTPResponse {
	enum BSDLDHTTPError		error; // Error code
	long				response; // HTTP Response code

	void				*data;
	long				data_len;
};

void bsdld_httpglue_init();
struct BSDLDHTTPResponse *bsdld_httpglue_request(struct BSDLDHTTPRequest *request);
struct BSDLDHTTPResponse *bsdld_httpglue_response_free(struct BSDLDHTTPResponse *response);


#endif
