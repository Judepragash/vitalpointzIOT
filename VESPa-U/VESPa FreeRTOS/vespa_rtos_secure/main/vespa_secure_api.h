#ifndef _VESPA_SECURE_API_H_
#define _VESPA_SECURE_API_H_

#include "vespa.h"

/* VESPa HTTPS APIs */
void vespa_secure_login(struct vespa_s *vespa, char* data);
void vespa_secure_device_auth(struct vespa_s *vespa, char* data);
void vespa_secure_device_cap(struct vespa_s * vespa, char* data);

/* VESPa response parser API's */
void vespa_parse_login_response(struct vespa_s * vespa, char * data);
void vespa_parse_auth_response(struct vespa_s * vespa, char* data);
void vespa_parse_cap_response(struct vespa_s * vespa, char* data);
void vespa_initialize(struct vespa_s * vespa, struct vespa_config_s * config);

#endif
