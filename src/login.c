#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <json-c/json.h>

#include "login.h"
#include "platform/httpglue.h"


static int _check_if_logged_in(struct BSDLDLoginStateStruct *login) {
	struct BSDLDHTTPRequest req = { 0 };
	struct BSDLDHTTPResponse *ret;
	char *url;
	int val;
	
	asprintf(&url, "%s/_matrix/client/v3/capabilities", login->base_url);

	req.type = BSDLDHTTP_REQUEST_TYPE_GET;
	req.auth_token = login->access_token;
	req.url = url;
	ret = bsdld_httpglue_request(&req);

	val = (ret->error == BSDLDHTTP_ERROR_OK && ret->response == 200);

	// TODO: Evaluate error code, check if it is a temporary error or invalid tokens
	// TODO: This function should be moved out, allowing all error conditions to be evaluated
	login->state = BSDLD_LOGIN_STATE_OK;
	if (!val)
		login->state = BSDLD_LOGIN_STATE_EXPIRED_TOKENS;
	printf("Response: %i %i [%i]\n%s\n", ret->error, ret->response, ret->data_len, ret->data);
	
	bsdld_httpglue_response_free(ret);
	return val;
}


struct BSDLDLoginStateStruct *bsdld_login(char *user_str, char *password_str) {
	struct json_object *root, *identifier, *m_id_user, *user, *initial_display_name, *password, *type;
	struct BSDLDHTTPRequest req = { 0 };
	struct BSDLDHTTPResponse *ret;
	struct BSDLDLoginStateStruct *state;
	const char *str;
	char *dup_user, *saveptr, *username_str, *hostname, *full_url;

	if (!(state = calloc(sizeof(*state), 1)))
		return NULL;

	dup_user = strdup(user_str);
	username_str = strtok_r(dup_user, "@:", &saveptr);
	hostname = strtok_r(NULL, ":", &saveptr);
	if (!username_str || !hostname)
		return free(dup_user), state->state = BSDLD_LOGIN_STATE_BAD_USERNAME, state;
	
	// TODO: Auto-detect prefix for URL
	asprintf(&full_url, "https://%s/_matrix/client/v3/login", hostname);

	root = json_object_new_object();
	identifier = json_object_new_object();
	json_object_object_add(root, "identifier", identifier);
	m_id_user = json_object_new_string("m.id.user"), json_object_object_add(identifier, "type", m_id_user);
	user = json_object_new_string(username_str), json_object_object_add(identifier, "user", user);
	initial_display_name = json_object_new_string("libbesudlad"), json_object_object_add(root, "initial_device_display_name", initial_display_name);
	password = json_object_new_string(password_str), json_object_object_add(root, "password", password);
	type = json_object_new_string("m.login.password"), json_object_object_add(root, "type", type);
	str = json_object_to_json_string(root);

	req.type = BSDLDHTTP_REQUEST_TYPE_POST;
	req.url = full_url;
	req.data = (char *) str;
	req.data_len = strlen(str);
	ret = bsdld_httpglue_request(&req);

	json_object_put(root);
	printf("Response: %i %i [%i]\n%s\n", ret->error, ret->response, ret->data_len, ret->data);
	free(dup_user);
	free(full_url), full_url = NULL;


	// TODO: Construct state struct

	bsdld_httpglue_response_free(ret);
	
//	identifier = json_object_new_string
	return state;
}


struct BSDLDLoginStateStruct *bsdld_login_existing(char *login_state) {
	struct json_object *root, *obj;
	struct BSDLDLoginStateStruct *state;
	const char *str;

	//printf("State:\n%s\n", login_state);

	if (!(state = malloc(sizeof(*state))))
		return NULL;

	root = json_tokener_parse(login_state);
	state->access_token = NULL;
	if ((obj = json_object_object_get(root, "access_token"))) {
		if ((str = json_object_get_string(obj)))
			state->access_token = strdup(str);
	}

	state->device_id = NULL;
	if ((obj = json_object_object_get(root, "device_id"))) {
		if ((str = json_object_get_string(obj)))
			state->device_id = strdup(str);
	}

	state->user_id = NULL;
	if ((obj = json_object_object_get(root, "user_id"))) {
		if ((str = json_object_get_string(obj)))
			state->user_id = strdup(str);
	}

	state->base_url = NULL;
	if ((obj = json_object_object_get(root, "base_url"))) {
		if ((str = json_object_get_string(obj)))
			state->base_url = strdup(str);
	}

	json_object_put(root);
	_check_if_logged_in(state);
	return state;
}


int main(int argc, char **argv) {
	struct BSDLDLoginStateStruct *login;
	FILE *fp;
	char *str;
	long len;

	bsdld_httpglue_init();
	//return bsdld_login("@username:hostname", "password"), 0;

	fp = fopen(argv[1], "r");
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	rewind(fp);
	str = malloc(len + 1);
	fread(str, len, 1, fp);
	str[len] = 0;
	
	login = bsdld_login_existing(str);
	if (login->state == BSDLD_LOGIN_STATE_OK)
		printf("login OK\n");

	return 0;
}
