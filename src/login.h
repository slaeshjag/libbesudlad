#ifndef BESUDLAD_LOGIN_H_
#define	BESUDLAD_LOGIN_H_

enum BSDLDLoginState {
	BSDLD_LOGIN_STATE_NONE,
	BSDLD_LOGIN_STATE_OK,
	BSDLD_LOGIN_STATE_BAD_USERNAME,
	BSDLD_LOGIN_STATE_EXPIRED_TOKENS,
};

struct BSDLDLoginStateStruct {
	enum BSDLDLoginState	state;
	char			*access_token;
	char			*device_id;
	char			*refresh_token;
	char			*user_id;
	char			*base_url;
	long			expiry_time;
};

#endif
