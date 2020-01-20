/**
 * 
 * Iddawc OAuth2 client library
 * 
 * iddawc.h: structures and functions declarations
 * 
 * Copyright 2019-2020 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation;
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include <stdarg.h>
#include <yder.h>
#include <rhonabwy.h>
#include <jwt.h>

#include "iddawc.h"

static const char * get_response_type(uint response_type) {
  static char result[32] = {0};
  switch (response_type) {
    case I_RESPONSE_TYPE_CODE:
      o_strcpy(result, "code");
      break;
    case I_RESPONSE_TYPE_TOKEN:
      o_strcpy(result, "token");
      break;
    case I_RESPONSE_TYPE_ID_TOKEN:
      o_strcpy(result, "id_token");
      break;
    case I_RESPONSE_TYPE_PASSWORD:
      o_strcpy(result, "password");
      break;
    case I_RESPONSE_TYPE_CLIENT_CREDENTIALS:
      o_strcpy(result, "client_credentials");
      break;
    case I_RESPONSE_TYPE_REFRESH_TOKEN:
      o_strcpy(result, "refresh_token");
      break;
    case I_RESPONSE_TYPE_CODE|I_RESPONSE_TYPE_ID_TOKEN:
      o_strcpy(result, "code id_token");
      break;
    case I_RESPONSE_TYPE_TOKEN|I_RESPONSE_TYPE_ID_TOKEN:
      o_strcpy(result, "token id_token");
      break;
    case I_RESPONSE_TYPE_TOKEN|I_RESPONSE_TYPE_CODE|I_RESPONSE_TYPE_ID_TOKEN:
      o_strcpy(result, "code token id_token");
      break;
    default:
      o_strcpy(result, "");
      break;
  }
  return result;
}

static int extract_parameters(const char * url_params, struct _u_map * map) {
  char ** unescaped_parameters = NULL, * key, * value;
  size_t offset = 0;
  int ret = I_OK;
  
  if (split_string(url_params, "&", &unescaped_parameters)) {
    for (offset = 0; unescaped_parameters[offset] != NULL; offset++) {
      if (o_strchr(unescaped_parameters[offset], '=') != NULL) {
        key = o_strndup(unescaped_parameters[offset], o_strchr(unescaped_parameters[offset], '=') - unescaped_parameters[offset]);
        value = ulfius_url_decode(o_strchr(unescaped_parameters[offset], '=')+1);
        u_map_put(map, key, value);
        o_free(key);
        o_free(value);
      } else {
        u_map_put(map, unescaped_parameters[offset], NULL);
      }
    }
    free_string_array(unescaped_parameters);
  } else {
    ret = I_ERROR;
  }
  return ret;
}

static int parse_redirect_to_parameters(struct _i_session * i_session, struct _u_map * map) {
  const char ** keys = u_map_enum_keys(map);
  size_t i;
  int ret = I_OK;
  
  for (i=0; keys[i] != NULL && ret == I_OK; i++) {
    if (0 == o_strcasecmp(keys[i], "code")) {
      if ((i_get_response_type(i_session) & I_RESPONSE_TYPE_CODE) && o_strlen(u_map_get(map, keys[i]))) {
        ret = i_set_parameter(i_session, I_OPT_CODE, u_map_get(map, keys[i]));
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "parse_redirect_to_parameters - Got paramter code where response_type doesn't have code");
        ret = I_ERROR_SERVER;
      }
    } else if (0 == o_strcasecmp(keys[i], "id_token")) {
      if ((i_get_response_type(i_session) & I_RESPONSE_TYPE_ID_TOKEN) && o_strlen(u_map_get(map, keys[i]))) {
        ret = i_set_parameter(i_session, I_OPT_ID_TOKEN, u_map_get(map, keys[i]));
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "parse_redirect_to_parameters - Got paramter id_token where response_type doesn't have id_token");
        ret = I_ERROR_SERVER;
      }
    } else if (0 == o_strcasecmp(keys[i], "access_token")) {
      if (u_map_has_key_case(map, "token_type") && o_strlen(u_map_get_case(map, "token_type"))) {
        if ((i_get_response_type(i_session) & I_RESPONSE_TYPE_TOKEN) && o_strlen(u_map_get(map, keys[i]))) {
          ret = i_set_parameter(i_session, I_OPT_ACCESS_TOKEN, u_map_get(map, keys[i]));
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "parse_redirect_to_parameters - Got paramter access_token where response_type doesn't have token");
          ret = I_ERROR_SERVER;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "parse_redirect_to_parameters - Got paramter token but token_type is missing");
        ret = I_ERROR_SERVER;
      }
    } else if (0 == o_strcasecmp(keys[i], "token_type")) {
      ret = i_set_parameter(i_session, I_OPT_TOKEN_TYPE, u_map_get(map, keys[i]));
    } else if (0 == o_strcasecmp(keys[i], "expires_in")) {
      i_set_flag_parameter(i_session, I_OPT_EXPIRES_IN, (uint)strtol(u_map_get(map, keys[i]), NULL, 10));
    } else if (0 == o_strcasecmp(keys[i], "error")) {
      i_set_result(i_session, I_ERROR_UNAUTHORIZED);
      ret = i_set_parameter(i_session, I_OPT_ERROR, u_map_get(map, keys[i]));
    } else if (0 == o_strcasecmp(keys[i], "error_description")) {
      i_set_result(i_session, I_ERROR_UNAUTHORIZED);
      ret = i_set_parameter(i_session, I_OPT_ERROR_DESCRIPTION, u_map_get(map, keys[i]));
    } else if (0 == o_strcasecmp(keys[i], "error_uri")) {
      i_set_result(i_session, I_ERROR_UNAUTHORIZED);
      ret = i_set_parameter(i_session, I_OPT_ERROR_URI, u_map_get(map, keys[i]));
    }
  }
  return ret;
}

static int parse_token_response(struct _i_session * i_session, int http_status, json_t * j_response) {
  int ret = I_OK;
  const char * key = NULL;
  json_t * j_element = NULL;
  char * value;
  
  if (i_session != NULL && json_is_object(j_response)) {
    if (http_status == 200) {
      if (json_string_length(json_object_get(j_response, "access_token")) &&
          json_string_length(json_object_get(j_response, "token_type"))) {
        if (i_set_parameter(i_session, I_OPT_ACCESS_TOKEN, json_string_value(json_object_get(j_response, "access_token"))) == I_OK &&
            i_set_parameter(i_session, I_OPT_TOKEN_TYPE, json_string_value(json_object_get(j_response, "token_type"))) == I_OK) {
          if (json_integer_value(json_object_get(j_response, "expires_in")) && i_set_flag_parameter(i_session, I_OPT_EXPIRES_IN, json_integer_value(json_object_get(j_response, "expires_in"))) != I_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - Error setting expires_in");
            ret = I_ERROR;
          }
          if (json_string_length(json_object_get(j_response, "refresh_token")) && i_set_parameter(i_session, I_OPT_REFRESH_TOKEN, json_string_value(json_object_get(j_response, "refresh_token"))) != I_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - Error setting refresh_token");
            ret = I_ERROR;
          }
          if (json_string_length(json_object_get(j_response, "id_token"))) {
            if (i_set_parameter(i_session, I_OPT_ID_TOKEN, json_string_value(json_object_get(j_response, "id_token"))) != I_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - Error setting id_token");
              ret = I_ERROR;
            } else if (i_verify_id_token(i_session, I_HAS_ACCESS_TOKEN|I_HAS_CODE) != I_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - Error i_verify_id_token");
              ret = I_ERROR;
            }
          }
          json_object_foreach(j_response, key, j_element) {
            if (0 != o_strcmp("access_token", key) &&
                0 != o_strcmp("token_type", key) &&
                0 != o_strcmp("expires_in", key) &&
                0 != o_strcmp("refresh_token", key) &&
                0 != o_strcmp("id_token", key)) {
              if (json_is_string(j_element)) {
                if (i_set_additional_response(i_session, key, json_string_value(j_element)) != I_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - Error i_set_additional_response %s - %s", key, json_string_value(j_element));
                  ret = I_ERROR;
                }
              } else {
                value = json_dumps(j_element, JSON_ENCODE_ANY);
                if (i_set_additional_response(i_session, key, value) != I_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - Error i_set_additional_response %s - %s", key, json_string_value(j_element));
                  ret = I_ERROR;
                }
                o_free(value);
              }
            }
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - Error setting response parameters");
          ret = I_ERROR;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - required response parameters missing");
        ret = I_ERROR_PARAM;
      }
    } else if (http_status == 400) {
      if (json_string_length(json_object_get(j_response, "error"))) {
        if (i_set_parameter(i_session, I_OPT_ERROR, json_string_value(json_object_get(j_response, "error"))) == I_OK) {
          if (json_string_length(json_object_get(j_response, "error_description")) && i_set_parameter(i_session, I_OPT_ERROR_DESCRIPTION, json_string_value(json_object_get(j_response, "error_description"))) != I_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - Error setting error_description");
            ret = I_ERROR;
          }
          if (json_string_length(json_object_get(j_response, "error_uri")) && i_set_parameter(i_session, I_OPT_ERROR_URI, json_string_value(json_object_get(j_response, "error_uri"))) != I_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - Error setting error_uri");
            ret = I_ERROR;
          }
          json_object_foreach(j_response, key, j_element) {
            if (0 != o_strcmp("error", key) &&
                0 != o_strcmp("error_description", key) &&
                0 != o_strcmp("error_uri", key)) {
              if (json_is_string(j_element)) {
                if (i_set_additional_response(i_session, key, json_string_value(j_element)) != I_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - Error i_set_additional_response %s - %s", key, json_string_value(j_element));
                  ret = I_ERROR;
                }
              } else {
                value = json_dumps(j_element, JSON_ENCODE_ANY);
                if (i_set_additional_response(i_session, key, value) != I_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - Error i_set_additional_response %s - %s", key, json_string_value(j_element));
                  ret = I_ERROR;
                }
                o_free(value);
              }
            }
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - Error setting response parameters");
          ret = I_ERROR;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "parse_token_response - required response parameters missing");
        ret = I_ERROR_PARAM;
      }
    }
  } else {
    ret = I_ERROR_PARAM;
  }
  return ret;
}

static int load_jwks_endpoint(struct _i_session * i_session) {
  int ret;
  struct _u_request request;
  struct _u_response response;
  json_t * j_jwks;
  
  if (i_session != NULL && json_string_length(json_object_get(i_session->openid_config, "jwks_uri"))) {
    ulfius_init_request(&request);
    ulfius_init_response(&response);
    
    request.http_url = o_strdup(json_string_value(json_object_get(i_session->openid_config, "jwks_uri")));
    if (ulfius_send_http_request(&request, &response) == U_OK) {
      if (response.status == 200) {
        j_jwks = ulfius_get_json_body_response(&response, NULL);
        if (r_jwks_import_from_json_t(i_session->jwks, j_jwks) == RHN_OK) {
          ret = I_OK;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "load_jwks_endpoint - Error r_jwks_import_from_str");
          ret = I_ERROR;
        }
        json_decref(j_jwks);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "load_jwks_endpoint - Error invalid response status: %d", response.status);
        ret = I_ERROR;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "load_jwks_endpoint - Error getting config_endpoint");
      ret = I_ERROR;
    }
    ulfius_clean_request(&request);
    ulfius_clean_response(&response);
  } else {
    ret = I_ERROR_PARAM;
  }
  return ret;
}

static int parse_openid_config(struct _i_session * i_session, int get_jwks) {
  int ret;
  size_t index = 0;
  json_t * j_element = NULL;
  
  if (i_session != NULL && json_is_object(i_session->openid_config)) {
    // Check required metadata
    if (json_string_length(json_object_get(i_session->openid_config, "issuer")) &&
        json_string_length(json_object_get(i_session->openid_config, "authorization_endpoint")) &&
        json_string_length(json_object_get(i_session->openid_config, "jwks_uri")) &&
        json_is_array(json_object_get(i_session->openid_config, "response_types_supported")) &&
        json_is_array(json_object_get(i_session->openid_config, "subject_types_supported")) &&
        json_is_array(json_object_get(i_session->openid_config, "token_endpoint_auth_signing_alg_values_supported"))) {
      ret = I_OK;
      do {
        if (i_set_parameter(i_session, I_OPT_ISSUER, json_string_value(json_object_get(i_session->openid_config, "issuer"))) != I_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "parse_openid_config - Error setting issuer");
          ret = I_ERROR;
          break;
        }
        if (i_set_parameter(i_session, I_OPT_AUTH_ENDPOINT, json_string_value(json_object_get(i_session->openid_config, "authorization_endpoint"))) != I_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "parse_openid_config - Error setting authorization_endpoint");
          ret = I_ERROR;
          break;
        }
        if (get_jwks && load_jwks_endpoint(i_session) != I_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "parse_openid_config - Error load_jwks_endpoint");
          ret = I_ERROR;
          break;
        }
        json_array_foreach(json_object_get(i_session->openid_config, "response_types_supported"), index, j_element) {
          if (!json_string_length(j_element)) {
            y_log_message(Y_LOG_LEVEL_ERROR, "parse_openid_config - Error response_types_supported invalid at index %zu", index);
            ret = I_ERROR;
            break;
          }
        }
        if (ret != I_OK) {
          break;
        }
        json_array_foreach(json_object_get(i_session->openid_config, "subject_types_supported"), index, j_element) {
          if (!json_string_length(j_element)) {
            y_log_message(Y_LOG_LEVEL_ERROR, "parse_openid_config - Error subject_types_supported invalid at index %zu", index);
            ret = I_ERROR;
            break;
          }
        }
        if (ret != I_OK) {
          break;
        }
        json_array_foreach(json_object_get(i_session->openid_config, "token_endpoint_auth_signing_alg_values_supported"), index, j_element) {
          if (!json_string_length(j_element)) {
            y_log_message(Y_LOG_LEVEL_ERROR, "parse_openid_config - Error token_endpoint_auth_signing_alg_values_supported invalid at index %zu", index);
            ret = I_ERROR;
            break;
          }
        }
        if (ret != I_OK) {
          break;
        }
      } while (0);
      if (json_string_length(json_object_get(i_session->openid_config, "token_endpoint"))) {
        if (i_set_parameter(i_session, I_OPT_TOKEN_ENDPOINT, json_string_value(json_object_get(i_session->openid_config, "token_endpoint"))) != I_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "parse_openid_config - Error setting token_endpoint");
          ret = I_ERROR;
        }
      }
      if (json_string_length(json_object_get(i_session->openid_config, "userinfo_endpoint"))) {
        if (i_set_parameter(i_session, I_OPT_USERINFO_ENDPOINT, json_string_value(json_object_get(i_session->openid_config, "userinfo_endpoint"))) != I_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "parse_openid_config - Error setting userinfo_endpoint");
          ret = I_ERROR;
        }
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "parse_openid_config - Error missing required metadata in JSON response");
      ret = I_ERROR;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "parse_openid_config - Error invalid JSON response");
    ret = I_ERROR;
  }
  return ret;
}

static int has_openid_config_parameter_value(struct _i_session * i_session, const char * parameter, const char * value) {
  int ret;
  size_t index = 0;
  json_t * j_element = NULL;
  
  if (i_session != NULL) {
    if (i_session->openid_config != NULL && i_session->openid_config_strict) {
      if (json_is_string(json_object_get(i_session->openid_config, parameter))) {
        if (0 == o_strcmp(value, json_string_value(json_object_get(i_session->openid_config, parameter)))) {
          ret = 1;
        } else {
          ret = 0;
        }
      } else if (json_is_array(json_object_get(i_session->openid_config, parameter))) {
        ret = 0;
        json_array_foreach(json_object_get(i_session->openid_config, parameter), index, j_element) {
          if (0 == o_strcmp(value, json_string_value(j_element))) {
            ret = 1;
          }
        }
      } else {
        ret = 0;
      }
    } else {
      ret = 1;
    }
  } else {
    ret = 0;
  }
  return ret;
}

static int check_strict_parameters(struct _i_session * i_session) {
  char ** str_array = NULL;
  int ret;
  size_t i;
  
  if (i_session != NULL) {
    ret = 1;
    if (i_session->scope != NULL) {
      if (split_string(i_session->scope, " ", &str_array)) {
        for (i=0; str_array[i]!=NULL; i++) {
          if (!has_openid_config_parameter_value(i_session, "scopes_supported", str_array[i])) {
            y_log_message(Y_LOG_LEVEL_DEBUG, "scope %s not supported", str_array[i]);
            ret = 0;
          }
        }
      }
      free_string_array(str_array);
    }
    if (!has_openid_config_parameter_value(i_session, "response_types_supported", get_response_type(i_session->response_type))) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "response_type %s not supported", get_response_type(i_session->response_type));
      ret = 0;
    }
  } else {
    ret = 0;
  }
  return ret;
}

int i_init_session(struct _i_session * i_session) {
  int res;
  
  if (i_session != NULL) {
    i_session->response_type = I_RESPONSE_TYPE_NONE;
    i_session->scope = NULL;
    i_session->nonce = NULL;
    i_session->redirect_uri = NULL;
    i_session->redirect_to = NULL;
    i_session->state = NULL;
    i_session->client_id = NULL;
    i_session->client_secret = NULL;
    i_session->username = NULL;
    i_session->user_password = NULL;
    i_session->authorization_endpoint = NULL;
    i_session->token_endpoint = NULL;
    i_session->openid_config_endpoint = NULL;
    i_session->userinfo_endpoint = NULL;
    i_session->refresh_token = NULL;
    i_session->access_token = NULL;
    i_session->token_type = NULL;
    i_session->expires_in = 0;
    i_session->id_token = NULL;
    i_session->id_token_payload = NULL;
    i_session->code = NULL;
    i_session->result = I_OK;
    i_session->error = NULL;
    i_session->error_description = NULL;
    i_session->error_uri = NULL;
    i_session->glewlwyd_api_url = NULL;
    i_session->glewlwyd_cookie_session = NULL;
    i_session->auth_method = I_AUTH_METHOD_GET;
    i_session->x5u_flags = 0;
    i_session->openid_config = NULL;
    i_session->openid_config_strict = I_STRICT_YES;
    i_session->issuer = NULL;
    i_session->userinfo = NULL;
    i_session->j_userinfo = NULL;
    o_strcpy(i_session->auth_sign_alg, "");
    if ((res = u_map_init(&i_session->additional_parameters)) == U_OK) {
      if ((res = u_map_init(&i_session->additional_response)) == U_OK) {
        if ((res = r_init_jwks(&i_session->jwks)) == RHN_OK) {
          if ((res = r_init_jwk(&i_session->id_token_header)) == RHN_OK) {
            return I_OK;
          } else if (res == U_ERROR_MEMORY) {
            return I_ERROR_MEMORY;
          } else {
            return I_ERROR;
          }
        } else if (res == U_ERROR_MEMORY) {
          return I_ERROR_MEMORY;
        } else {
          return I_ERROR;
        }
      } else if (res == U_ERROR_MEMORY) {
        return I_ERROR_MEMORY;
      } else {
        return I_ERROR;
      }
    } else if (res == U_ERROR_MEMORY) {
      return I_ERROR_MEMORY;
    } else {
      return I_ERROR;
    }
  } else {
    return I_ERROR_PARAM;
  }
}

void i_clean_session(struct _i_session * i_session) {
  if (i_session != NULL) {
    o_free(i_session->scope);
    o_free(i_session->nonce);
    o_free(i_session->redirect_uri);
    o_free(i_session->redirect_to);
    o_free(i_session->state);
    o_free(i_session->client_id);
    o_free(i_session->client_secret);
    o_free(i_session->username);
    o_free(i_session->user_password);
    o_free(i_session->authorization_endpoint);
    o_free(i_session->token_endpoint);
    o_free(i_session->openid_config_endpoint);
    o_free(i_session->code);
    o_free(i_session->error);
    o_free(i_session->error_description);
    o_free(i_session->error_uri);
    o_free(i_session->refresh_token);
    o_free(i_session->access_token);
    o_free(i_session->token_type);
    o_free(i_session->id_token);
    o_free(i_session->glewlwyd_api_url);
    o_free(i_session->glewlwyd_cookie_session);
    o_free(i_session->userinfo_endpoint);
    o_free(i_session->issuer);
    o_free(i_session->userinfo);
    u_map_clean(&i_session->additional_parameters);
    u_map_clean(&i_session->additional_response);
    r_free_jwks(i_session->jwks);
    r_free_jwk(i_session->id_token_header);
    json_decref(i_session->id_token_payload);
    json_decref(i_session->openid_config);
    json_decref(i_session->j_userinfo);
  }
}

int i_set_response_type(struct _i_session * i_session, uint i_value) {
  return i_set_flag_parameter(i_session, I_OPT_RESPONSE_TYPE, i_value);
}

int i_set_result(struct _i_session * i_session, uint i_value) {
  return i_set_flag_parameter(i_session, I_OPT_RESULT, i_value);
}

int i_set_flag_parameter(struct _i_session * i_session, uint option, uint i_value) {
  int ret = I_OK;
  if (i_session != NULL) {
    switch (option) {
      case I_OPT_RESPONSE_TYPE:
        switch (i_value) {
          case I_RESPONSE_TYPE_CODE:
          case I_RESPONSE_TYPE_TOKEN:
          case I_RESPONSE_TYPE_ID_TOKEN:
          case I_RESPONSE_TYPE_PASSWORD:
          case I_RESPONSE_TYPE_CLIENT_CREDENTIALS:
          case I_RESPONSE_TYPE_REFRESH_TOKEN:
          case I_RESPONSE_TYPE_CODE|I_RESPONSE_TYPE_ID_TOKEN:
          case I_RESPONSE_TYPE_TOKEN|I_RESPONSE_TYPE_ID_TOKEN:
          case I_RESPONSE_TYPE_TOKEN|I_RESPONSE_TYPE_CODE:
          case I_RESPONSE_TYPE_TOKEN|I_RESPONSE_TYPE_CODE|I_RESPONSE_TYPE_ID_TOKEN:
            i_session->response_type = i_value;
            break;
          default:
            ret = I_ERROR_PARAM;
            break;
        }
        break;
      case I_OPT_RESULT:
        switch (i_value) {
          case I_OK:
          case I_ERROR:
          case I_ERROR_PARAM:
          case I_ERROR_UNAUTHORIZED:
          case I_ERROR_SERVER:
            i_session->result = i_value;
            break;
          default:
            ret = I_ERROR_PARAM;
            break;
        }
        break;
      case I_OPT_AUTH_METHOD:
        switch (i_value) {
          case I_AUTH_METHOD_GET:
          case I_AUTH_METHOD_POST:
            i_session->auth_method = i_value;
            break;
          default:
            ret = I_ERROR_PARAM;
            break;
        }
        break;
      case I_OPT_AUTH_SIGN_ALG:
        switch (i_value) {
          case I_AUTH_SIGN_ALG_RS256:
            o_strcpy(i_session->auth_sign_alg, "RS256");
            break;
          case I_AUTH_SIGN_ALG_RS384:
            o_strcpy(i_session->auth_sign_alg, "RS384");
            break;
          case I_AUTH_SIGN_ALG_RS512:
            o_strcpy(i_session->auth_sign_alg, "RS512");
            break;
          default:
            ret = I_ERROR_PARAM;
            break;
        }
        break;
      case I_OPT_EXPIRES_IN:
        if (i_value) {
          i_session->expires_in = i_value;
        } else {
          ret = I_ERROR_PARAM;
        }
        break;
      case I_OPT_OPENID_CONFIG_STRICT:
        i_session->openid_config_strict = i_value;
        break;
      default:
        ret = I_ERROR_PARAM;
        break;
    }
  } else {
    ret = I_ERROR_PARAM;
  }
  return ret;
}

int i_set_parameter(struct _i_session * i_session, uint option, const char * s_value) {
  int ret = I_OK;
  if (i_session != NULL) {
    switch (option) {
      case I_OPT_SCOPE:
        o_free(i_session->scope);
        if (o_strlen(s_value)) {
          i_session->scope = o_strdup(s_value);
        } else {
          i_session->scope = NULL;
        }
        break;
      case I_OPT_SCOPE_APPEND:
        if (o_strlen(s_value)) {
          if (i_session->scope == NULL) {
            i_session->scope = o_strdup(s_value);
          } else {
            i_session->scope = mstrcatf(i_session->scope, " %s", s_value);
          }
        } else {
          o_free(i_session->scope);
          i_session->scope = NULL;
        }
        break;
      case I_OPT_STATE:
        o_free(i_session->state);
        if (o_strlen(s_value)) {
          i_session->state = o_strdup(s_value);
        } else {
          i_session->state = NULL;
        }
        break;
      case I_OPT_NONCE:
        o_free(i_session->nonce);
        if (o_strlen(s_value)) {
          i_session->nonce = o_strdup(s_value);
        } else {
          i_session->nonce = NULL;
        }
        break;
      case I_OPT_REDIRECT_URI:
        o_free(i_session->redirect_uri);
        if (o_strlen(s_value)) {
          i_session->redirect_uri = o_strdup(s_value);
        } else {
          i_session->redirect_uri = NULL;
        }
        break;
      case I_OPT_REDIRECT_TO:
        o_free(i_session->redirect_to);
        if (o_strlen(s_value)) {
          i_session->redirect_to = o_strdup(s_value);
        } else {
          i_session->redirect_to = NULL;
        }
        break;
      case I_OPT_CLIENT_ID:
        o_free(i_session->client_id);
        if (o_strlen(s_value)) {
          i_session->client_id = o_strdup(s_value);
        } else {
          i_session->client_id = NULL;
        }
        break;
      case I_OPT_CLIENT_SECRET:
        o_free(i_session->client_secret);
        if (o_strlen(s_value)) {
          i_session->client_secret = o_strdup(s_value);
        } else {
          i_session->client_secret = NULL;
        }
        break;
      case I_OPT_AUTH_ENDPOINT:
        o_free(i_session->authorization_endpoint);
        if (o_strlen(s_value)) {
          i_session->authorization_endpoint = o_strdup(s_value);
        } else {
          i_session->authorization_endpoint = NULL;
        }
        break;
      case I_OPT_TOKEN_ENDPOINT:
        o_free(i_session->token_endpoint);
        if (o_strlen(s_value)) {
          i_session->token_endpoint = o_strdup(s_value);
        } else {
          i_session->token_endpoint = NULL;
        }
        break;
      case I_OPT_OPENID_CONFIG_ENDPOINT:
        o_free(i_session->openid_config_endpoint);
        if (o_strlen(s_value)) {
          i_session->openid_config_endpoint = o_strdup(s_value);
        } else {
          i_session->openid_config_endpoint = NULL;
        }
        break;
      case I_OPT_USERINFO_ENDPOINT:
        o_free(i_session->userinfo_endpoint);
        if (o_strlen(s_value)) {
          i_session->userinfo_endpoint = o_strdup(s_value);
        } else {
          i_session->userinfo_endpoint = NULL;
        }
        break;
      case I_OPT_ERROR:
        o_free(i_session->error);
        if (o_strlen(s_value)) {
          i_session->error = o_strdup(s_value);
        } else {
          i_session->error = NULL;
        }
        break;
      case I_OPT_ERROR_DESCRIPTION:
        o_free(i_session->error_description);
        if (o_strlen(s_value)) {
          i_session->error_description = o_strdup(s_value);
        } else {
          i_session->error_description = NULL;
        }
        break;
      case I_OPT_ERROR_URI:
        o_free(i_session->error_uri);
        if (o_strlen(s_value)) {
          i_session->error_uri = o_strdup(s_value);
        } else {
          i_session->error_uri = NULL;
        }
        break;
      case I_OPT_CODE:
        o_free(i_session->code);
        if (o_strlen(s_value)) {
          i_session->code = o_strdup(s_value);
        } else {
          i_session->code = NULL;
        }
        break;
      case I_OPT_REFRESH_TOKEN:
        o_free(i_session->refresh_token);
        if (o_strlen(s_value)) {
          i_session->refresh_token = o_strdup(s_value);
        } else {
          i_session->refresh_token = NULL;
        }
        break;
      case I_OPT_ACCESS_TOKEN:
        o_free(i_session->access_token);
        if (o_strlen(s_value)) {
          i_session->access_token = o_strdup(s_value);
        } else {
          i_session->access_token = NULL;
        }
        break;
      case I_OPT_ID_TOKEN:
        o_free(i_session->id_token);
        if (o_strlen(s_value)) {
          i_session->id_token = o_strdup(s_value);
        } else {
          i_session->id_token = NULL;
        }
        break;
      case I_OPT_GLEWLWYD_API_URL:
        o_free(i_session->glewlwyd_api_url);
        if (o_strlen(s_value)) {
          i_session->glewlwyd_api_url = o_strdup(s_value);
        } else {
          i_session->glewlwyd_api_url = NULL;
        }
        break;
      case I_OPT_GLEWLWYD_COOKIE_SESSION:
        o_free(i_session->glewlwyd_cookie_session);
        if (o_strlen(s_value)) {
          i_session->glewlwyd_cookie_session = o_strdup(s_value);
        } else {
          i_session->glewlwyd_cookie_session = NULL;
        }
        break;
      case I_OPT_TOKEN_TYPE:
        o_free(i_session->token_type);
        if (o_strlen(s_value)) {
          i_session->token_type = o_strdup(s_value);
        } else {
          i_session->token_type = NULL;
        }
        break;
      case I_OPT_USERNAME:
        o_free(i_session->username);
        if (o_strlen(s_value)) {
          i_session->username = o_strdup(s_value);
        } else {
          i_session->username = NULL;
        }
        break;
      case I_OPT_USER_PASSWORD:
        o_free(i_session->user_password);
        if (o_strlen(s_value)) {
          i_session->user_password = o_strdup(s_value);
        } else {
          i_session->user_password = NULL;
        }
        break;
      case I_OPT_OPENID_CONFIG:
        json_decref(i_session->openid_config);
        if (o_strlen(s_value)) {
          if ((i_session->openid_config = json_loads(s_value, JSON_DECODE_ANY, NULL)) != NULL) {
            if (parse_openid_config(i_session, 0) == I_OK) {
              ret = I_OK;
            } else {
              json_decref(i_session->openid_config);
              i_session->openid_config = NULL;
              ret = I_ERROR;
            }
          } else {
            ret = I_ERROR;
          }
        } else {
          i_session->openid_config = NULL;
        }
        break;
      case I_OPT_ISSUER:
        o_free(i_session->issuer);
        if (o_strlen(s_value)) {
          i_session->issuer = o_strdup(s_value);
        } else {
          i_session->issuer = NULL;
        }
        break;
      case I_OPT_USERINFO:
        o_free(i_session->userinfo);
        if (o_strlen(s_value)) {
          i_session->userinfo = o_strdup(s_value);
        } else {
          i_session->userinfo = NULL;
        }
        break;
      default:
        ret = I_ERROR_PARAM;
        break;
    }
  } else {
    ret = I_ERROR_PARAM;
  }
  return ret;
}

int i_set_additional_parameter(struct _i_session * i_session, const char * s_key, const char * s_value) {
  int ret = I_OK;
  if (i_session != NULL && s_key != NULL) {
    if (u_map_put(&i_session->additional_parameters, s_key, s_value) != U_OK) {
      ret = I_ERROR;
    }
  } else {
    ret = I_ERROR_PARAM;
  }
  return ret;
}

int i_set_additional_response(struct _i_session * i_session, const char * s_key, const char * s_value) {
  int ret = I_OK;
  if (i_session != NULL && s_key != NULL) {
    if (u_map_put(&i_session->additional_response, s_key, s_value) != U_OK) {
      ret = I_ERROR;
    }
  } else {
    ret = I_ERROR_PARAM;
  }
  return ret;
}

int i_set_parameter_list(struct _i_session * i_session, ...) {
  uint option, i_value, ret = I_OK;
  const char * str_key, * str_value;
  va_list vl;
  
  if (i_session != NULL) {
    va_start(vl, i_session);
    for (option = va_arg(vl, uint); option != I_OPT_NONE && ret == I_OK; option = va_arg(vl, uint)) {
      switch (option) {
        case I_OPT_RESPONSE_TYPE:
        case I_OPT_RESULT:
        case I_OPT_AUTH_METHOD:
        case I_OPT_EXPIRES_IN:
          i_value = va_arg(vl, uint);
          ret = i_set_flag_parameter(i_session, option, i_value);
          break;
        case I_OPT_SCOPE:
        case I_OPT_STATE:
        case I_OPT_NONCE:
        case I_OPT_REDIRECT_URI:
        case I_OPT_REDIRECT_TO:
        case I_OPT_CLIENT_ID:
        case I_OPT_CLIENT_SECRET:
        case I_OPT_AUTH_ENDPOINT:
        case I_OPT_TOKEN_ENDPOINT:
        case I_OPT_OPENID_CONFIG_ENDPOINT:
        case I_OPT_OPENID_CONFIG:
        case I_OPT_USERINFO_ENDPOINT:
        case I_OPT_ERROR:
        case I_OPT_ERROR_DESCRIPTION:
        case I_OPT_ERROR_URI:
        case I_OPT_CODE:
        case I_OPT_REFRESH_TOKEN:
        case I_OPT_ACCESS_TOKEN:
        case I_OPT_ID_TOKEN:
        case I_OPT_GLEWLWYD_API_URL:
        case I_OPT_GLEWLWYD_COOKIE_SESSION:
        case I_OPT_TOKEN_TYPE:
        case I_OPT_USERNAME:
        case I_OPT_USER_PASSWORD:
        case I_OPT_ISSUER:
          str_value = va_arg(vl, const char *);
          ret = i_set_parameter(i_session, option, str_value);
          break;
        case I_OPT_ADDITIONAL_PARAMETER:
          str_key = va_arg(vl, const char *);
          str_value = va_arg(vl, const char *);
          ret = i_set_additional_parameter(i_session, str_key, str_value);
          break;
        case I_OPT_ADDITIONAL_RESPONSE:
          str_key = va_arg(vl, const char *);
          str_value = va_arg(vl, const char *);
          ret = i_set_additional_response(i_session, str_key, str_value);
          break;
        default:
          ret = I_ERROR_PARAM;
          break;
      }
    }
    va_end(vl);
  } else {
    ret = I_ERROR_PARAM;
  }
  return ret;
}

int i_load_openid_config(struct _i_session * i_session) {
  int ret;
  struct _u_request request;
  struct _u_response response;
  
  if (i_session != NULL && i_session->openid_config_endpoint != NULL) {
    ulfius_init_request(&request);
    ulfius_init_response(&response);
    
    request.http_url = o_strdup(i_session->openid_config_endpoint);
    if (ulfius_send_http_request(&request, &response) == U_OK) {
      if (response.status == 200) {
        if ((i_session->openid_config = ulfius_get_json_body_response(&response, NULL)) != NULL) {
          if (parse_openid_config(i_session, 1) == I_OK) {
            ret = I_OK;
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "i_load_openid_config - Error parse_openid_config");
            ret = I_ERROR;
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "i_load_openid_config - Error response not in JSON format");
          ret = I_ERROR;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "i_load_openid_config - Error invalid response status: %d", response.status);
        ret = I_ERROR;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "i_load_openid_config - Error getting config_endpoint");
      ret = I_ERROR;
    }
    ulfius_clean_request(&request);
    ulfius_clean_response(&response);
  } else {
    ret = I_ERROR_PARAM;
  }
  return ret;
}

int i_load_userinfo(struct _i_session * i_session) {
  int ret;
  struct _u_request request;
  struct _u_response response;
  char * bearer = NULL;
  
  if (i_session != NULL && i_session->userinfo_endpoint != NULL && i_session->access_token != NULL) {
    ulfius_init_request(&request);
    ulfius_init_response(&response);
    
    request.http_url = o_strdup(i_session->userinfo_endpoint);
    bearer = msprintf("Bearer %s", i_session->access_token);
    if (u_map_put(request.map_header, "Authorization", bearer) == U_OK) {
      if (ulfius_send_http_request(&request, &response) == U_OK) {
        if (response.status == 200) {
          o_free(i_session->userinfo);
          if ((i_session->userinfo = o_strndup(response.binary_body, response.binary_body_length)) != NULL) {
            json_decref(i_session->j_userinfo);
            i_session->j_userinfo = json_loads(i_session->userinfo, JSON_DECODE_ANY, NULL);
            ret = I_OK;
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "i_load_userinfo - Error getting response");
            ret = I_ERROR;
          }
        } else if (response.status == 401 || response.status == 403) {
          ret = I_ERROR_UNAUTHORIZED;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "i_load_userinfo - Error invalid response status: %d", response.status);
          ret = I_ERROR;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "i_load_userinfo - Error getting userinfo_endpoint");
        ret = I_ERROR;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "i_load_userinfo - Error u_map_put");
      ret = I_ERROR;
    }
    o_free(bearer);
    ulfius_clean_request(&request);
    ulfius_clean_response(&response);
  } else {
    ret = I_ERROR_PARAM;
  }
  return ret;
}

uint i_get_response_type(struct _i_session * i_session) {
  return i_get_flag_parameter(i_session, I_OPT_RESPONSE_TYPE);
}

uint i_get_result(struct _i_session * i_session) {
  return i_get_flag_parameter(i_session, I_OPT_RESULT);
}

uint i_get_flag_parameter(struct _i_session * i_session, uint option) {
  if (i_session != NULL) {
    switch (option) {
      case I_OPT_RESPONSE_TYPE:
        return i_session->response_type;
        break;
      case I_OPT_RESULT:
        return i_session->result;
        break;
      case I_OPT_AUTH_METHOD:
        return i_session->auth_method;
        break;
      case I_OPT_EXPIRES_IN:
        return i_session->expires_in;
        break;
      case I_OPT_AUTH_SIGN_ALG:
        if (o_strncmp(i_session->auth_sign_alg, "RS256", I_AUTH_SIGN_ALG_MAX_LENGTH)) {
          return I_AUTH_SIGN_ALG_RS256;
        } else if (o_strncmp(i_session->auth_sign_alg, "RS384", I_AUTH_SIGN_ALG_MAX_LENGTH)) {
          return I_AUTH_SIGN_ALG_RS384;
        } else if (o_strncmp(i_session->auth_sign_alg, "RS512", I_AUTH_SIGN_ALG_MAX_LENGTH)) {
          return I_AUTH_SIGN_ALG_RS512;
        } else {
          return I_AUTH_SIGN_ALG_NONE;
        }
        return i_session->result;
        break;
      default:
        return 0;
        break;
    }
  }
  return 0;
}

int i_parse_redirect_to(struct _i_session * i_session) {
  int ret = I_OK;
  struct _u_map map;
  const char * fragment = NULL, * query = NULL, * redirect_to = i_get_parameter(i_session, I_OPT_REDIRECT_TO);
  char * state = NULL;
  
  if (o_strncmp(redirect_to, i_session->redirect_uri, o_strlen(i_session->redirect_uri)) == 0) {
    // Extract fragment if response_type has id_token or token
    if ((i_session->response_type & I_RESPONSE_TYPE_TOKEN || i_session->response_type & I_RESPONSE_TYPE_ID_TOKEN) && (fragment = o_strnchr(redirect_to, o_strlen(redirect_to), '#')) != NULL && has_openid_config_parameter_value(i_session, "response_modes_supported", "fragment")) {
      u_map_init(&map);
      fragment++;
      if (extract_parameters(fragment, &map) == I_OK) {
        if ((ret = parse_redirect_to_parameters(i_session, &map)) == I_OK) {
          if (i_session->id_token != NULL && r_jwks_size(i_session->jwks) && i_verify_id_token(i_session, i_session->response_type) != I_OK) {
            y_log_message(Y_LOG_LEVEL_DEBUG, "i_parse_redirect_to fragment - Error id_token invalid");
            ret = I_ERROR_SERVER;
          }
        }
      }
      if (u_map_get(&map, "state") != NULL) {
        state = o_strdup(u_map_get(&map, "state"));
      }
      u_map_clean(&map);
    }
    
    // Extract query without fragment if response_type is code only
    if (i_session->response_type & I_RESPONSE_TYPE_CODE && has_openid_config_parameter_value(i_session, "response_modes_supported", "query") && (query = o_strnchr(redirect_to, fragment!=NULL?(size_t)(redirect_to-fragment):o_strlen(redirect_to), '?')) != NULL) {
      u_map_init(&map);
      query++;
      if (extract_parameters(query, &map) == I_OK) {
        if ((ret = parse_redirect_to_parameters(i_session, &map)) == I_OK) {
         if (i_session->id_token != NULL && r_jwks_size(i_session->jwks) && i_verify_id_token(i_session, i_session->response_type) != I_OK) {
            y_log_message(Y_LOG_LEVEL_DEBUG, "i_parse_redirect_to query - Error id_token invalid");
            ret = I_ERROR_SERVER;
          }
        }
      }
      if (u_map_get(&map, "state") != NULL && state == NULL) {
        state = o_strdup(u_map_get(&map, "state"));
      }
      u_map_clean(&map);
    }
    
    if (i_get_parameter(i_session, I_OPT_STATE) != NULL) {
      if (o_strcmp(i_get_parameter(i_session, I_OPT_STATE), state)) {
        y_log_message(Y_LOG_LEVEL_DEBUG, "i_parse_redirect_to query - Error state invalid");
        ret = I_ERROR_SERVER;
      }
    }
    o_free(state);
  }
  return ret;
}

const char * i_get_parameter(struct _i_session * i_session, uint option) {
  const char * result = NULL;
  if (i_session != NULL) {
    switch (option) {
      case I_OPT_SCOPE:
      case I_OPT_SCOPE_APPEND:
        result = (const char *)i_session->scope;
        break;
      case I_OPT_STATE:
        result = (const char *)i_session->state;
        break;
      case I_OPT_NONCE:
        result = (const char *)i_session->nonce;
        break;
      case I_OPT_REDIRECT_URI:
        result = (const char *)i_session->redirect_uri;
        break;
      case I_OPT_REDIRECT_TO:
        result = (const char *)i_session->redirect_to;
        break;
      case I_OPT_CLIENT_ID:
        result = (const char *)i_session->client_id;
        break;
      case I_OPT_CLIENT_SECRET:
        result = (const char *)i_session->client_secret;
        break;
      case I_OPT_AUTH_ENDPOINT:
        result = (const char *)i_session->authorization_endpoint;
        break;
      case I_OPT_TOKEN_ENDPOINT:
        result = (const char *)i_session->token_endpoint;
        break;
      case I_OPT_OPENID_CONFIG_ENDPOINT:
        result = (const char *)i_session->openid_config_endpoint;
        break;
      case I_OPT_USERINFO_ENDPOINT:
        result = (const char *)i_session->userinfo_endpoint;
        break;
      case I_OPT_ERROR:
        result = (const char *)i_session->error;
        break;
      case I_OPT_ERROR_DESCRIPTION:
        result = (const char *)i_session->error_description;
        break;
      case I_OPT_ERROR_URI:
        result = (const char *)i_session->error_uri;
        break;
      case I_OPT_CODE:
        result = (const char *)i_session->code;
        break;
      case I_OPT_REFRESH_TOKEN:
        result = (const char *)i_session->refresh_token;
        break;
      case I_OPT_ACCESS_TOKEN:
        result = (const char *)i_session->access_token;
        break;
      case I_OPT_ID_TOKEN:
        result = (const char *)i_session->id_token;
        break;
      case I_OPT_GLEWLWYD_API_URL:
        result = (const char *)i_session->glewlwyd_api_url;
        break;
      case I_OPT_GLEWLWYD_COOKIE_SESSION:
        result = (const char *)i_session->glewlwyd_cookie_session;
        break;
      case I_OPT_TOKEN_TYPE:
        result = (const char *)i_session->token_type;
        break;
      case I_OPT_USERNAME:
        result = (const char *)i_session->username;
        break;
      case I_OPT_USER_PASSWORD:
        result = (const char *)i_session->user_password;
        break;
      case I_OPT_ISSUER:
        result = (const char *)i_session->issuer;
        break;
      case I_OPT_USERINFO:
        result = (const char *)i_session->userinfo;
        break;
      default:
        break;
    }
  }
  return result;
}

const char * i_get_additional_parameter(struct _i_session * i_session, const char * s_key) {
  if (i_session != NULL) {
    return u_map_get(&i_session->additional_parameters, s_key);
  } else {
    return NULL;
  }
}

const char * i_get_additional_response(struct _i_session * i_session, const char * s_key) {
  if (i_session != NULL) {
    return u_map_get(&i_session->additional_response, s_key);
  } else {
    return NULL;
  }
}

int i_run_auth_request(struct _i_session * i_session) {
  int ret = I_OK;
  struct _u_request request;
  struct _u_response response;
  char * url = NULL, * escaped = NULL, * redirect_to = NULL;
  const char ** keys = NULL;
  uint i;
  
  if (i_session != NULL && 
      i_session->response_type != I_RESPONSE_TYPE_NONE && 
      i_session->response_type != I_RESPONSE_TYPE_PASSWORD && 
      i_session->response_type != I_RESPONSE_TYPE_CLIENT_CREDENTIALS && 
      i_session->response_type != I_RESPONSE_TYPE_REFRESH_TOKEN && 
      i_session->redirect_uri != NULL && 
      i_session->client_id != NULL && 
      i_session->authorization_endpoint != NULL &&
      check_strict_parameters(i_session) &&
      has_openid_config_parameter_value(i_session, "grant_types_supported", "implicit")) {
    if (ulfius_init_request(&request) != U_OK || ulfius_init_response(&response) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error initializing request or response");
      ret = I_ERROR;
    } else {
      u_map_put(request.map_header, "User-Agent", "Iddawc runtime");
      if (i_session->auth_method == I_AUTH_METHOD_GET) {
        escaped = ulfius_url_encode(i_session->redirect_uri);
        url = msprintf("%s?redirect_uri=%s", i_session->authorization_endpoint, escaped);
        o_free(escaped);

        escaped = ulfius_url_encode(get_response_type(i_session->response_type));
        url = mstrcatf(url, "&response_type=%s", escaped);
        o_free(escaped);
        
        escaped = ulfius_url_encode(i_session->client_id);
        url = mstrcatf(url, "&client_id=%s", escaped);
        o_free(escaped);
        
        if (i_session->state != NULL) {
          escaped = ulfius_url_encode(i_session->state);
          url = mstrcatf(url, "&state=%s", escaped);
          o_free(escaped);
        }
        
        if (i_session->scope != NULL) {
          escaped = ulfius_url_encode(i_session->scope);
          url = mstrcatf(url, "&scope=%s", escaped);
          o_free(escaped);
        }
        
        if (i_session->nonce != NULL) {
          escaped = ulfius_url_encode(i_session->nonce);
          url = mstrcatf(url, "&nonce=%s", escaped);
          o_free(escaped);
        }
        
        keys = u_map_enum_keys(&i_session->additional_parameters);
        
        for (i=0; keys[i] != NULL; i++) {
          escaped = ulfius_url_encode(u_map_get(&i_session->additional_parameters, keys[i]));
          url = mstrcatf(url, "&%s=%s", keys[i], escaped);
          o_free(escaped);
        }
        
        request.http_verb = o_strdup("GET");
        request.http_url = url;
      } else if (i_session->auth_method == I_AUTH_METHOD_POST) {
        request.http_verb = o_strdup("POST");
        request.http_url = o_strdup(i_session->authorization_endpoint);
        u_map_put(request.map_post_body, "redirect_uri", i_session->redirect_uri);
        u_map_put(request.map_post_body, "response_type", get_response_type(i_session->response_type));
        u_map_put(request.map_post_body, "client_id", i_session->client_id);
        if (i_session->state != NULL) {
          u_map_put(request.map_post_body, "state", i_session->state);
        }
        if (i_session->scope != NULL) {
          u_map_put(request.map_post_body, "scope", i_session->scope);
        }
        if (i_session->nonce != NULL) {
          u_map_put(request.map_post_body, "nonce", i_session->nonce);
        }
        
        keys = u_map_enum_keys(&i_session->additional_parameters);
        
        for (i=0; keys[i] != NULL; i++) {
          u_map_put(request.map_post_body, keys[i], u_map_get(&i_session->additional_parameters, keys[i]));
        }
      } else {
        // Unsupported auth_method
        y_log_message(Y_LOG_LEVEL_DEBUG, "i_run_auth_request - Unsupported auth_method");
        ret = I_ERROR_PARAM;
      }
      
      if (i_get_parameter(i_session, I_OPT_GLEWLWYD_COOKIE_SESSION) != NULL) {
        u_map_put(request.map_header, "Cookie", i_get_parameter(i_session, I_OPT_GLEWLWYD_COOKIE_SESSION));
      }
      
      if (ret == I_OK) {
        if (ulfius_send_http_request(&request, &response) == U_OK) {
          if (response.status == 302 && o_strlen(u_map_get_case(response.map_header, "Location"))) {
            redirect_to = o_strdup(u_map_get_case(response.map_header, "Location"));
            if (i_set_parameter(i_session, I_OPT_REDIRECT_TO, redirect_to) == I_OK) {
              ret = i_parse_redirect_to(i_session);
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "i_run_auth_request - Error setting redirect url");
            }
            o_free(redirect_to);
          } else if (response.status == 400) {
            y_log_message(Y_LOG_LEVEL_DEBUG, "i_run_auth_request - Server response 400: %.*s", response.binary_body_length, response.binary_body);
            ret = I_ERROR_PARAM;
          } else {
            ret = I_ERROR;
          }
        } else {
          ret = I_ERROR;
        }
      }
      ulfius_clean_request(&request);
      ulfius_clean_response(&response);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_DEBUG, "i_run_auth_request - Invalid input parameters");
    ret = I_ERROR_PARAM;
  }
  return ret;
}

int i_run_token_request(struct _i_session * i_session) {
  int ret = I_OK;
  struct _u_request request;
  struct _u_response response;
  json_t * j_response;
  
  if (i_session != NULL && i_session->token_endpoint != NULL) {
    if (i_session->response_type & I_RESPONSE_TYPE_CODE) {
      if (i_session->redirect_uri != NULL && 
          i_session->client_id != NULL && 
          i_session->code != NULL &&
          check_strict_parameters(i_session) &&
          has_openid_config_parameter_value(i_session, "grant_types_supported", "authorization_code")) {
        ulfius_init_request(&request);
        ulfius_init_response(&response);
        request.http_verb = o_strdup("POST");
        request.http_url = o_strdup(i_session->token_endpoint);
        u_map_put(request.map_post_body, "grant_type", "authorization_code");
        u_map_put(request.map_post_body, "code", i_session->code);
        u_map_put(request.map_post_body, "redirect_uri", i_session->redirect_uri);
        u_map_put(request.map_post_body, "client_id", i_session->client_id);
        if (i_session->client_secret != NULL) {
          request.auth_basic_user = o_strdup(i_session->client_id);
          request.auth_basic_password = o_strdup(i_session->client_secret);
        }
        if (ulfius_send_http_request(&request, &response) == U_OK) {
          if (response.status == 200 || response.status == 400) {
            j_response = ulfius_get_json_body_response(&response, NULL);
            if (j_response != NULL) {
              if (parse_token_response(i_session, response.status, j_response) == I_OK) {
                ret = response.status == 200?I_OK:I_ERROR_PARAM;
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request code - Error parse_token_response");
                ret = I_ERROR_PARAM;
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request code - Error parsing JSON response");
              ret = I_ERROR;
            }
            json_decref(j_response);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request code - Invalid response status: %d", response.status);
            ret = I_ERROR;
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request code - Error sending token request");
          ret = I_ERROR;
        }
        ulfius_clean_request(&request);
        ulfius_clean_response(&response);
      } else {
        y_log_message(Y_LOG_LEVEL_DEBUG, "i_run_token_request code - Error input parameters");
        if (i_session->redirect_uri == NULL) {
          y_log_message(Y_LOG_LEVEL_DEBUG, "redirect_uri NULL");
        }
        if (i_session->client_id == NULL) {
          y_log_message(Y_LOG_LEVEL_DEBUG, "client_id NULL");
        }
        if (i_session->code == NULL) {
          y_log_message(Y_LOG_LEVEL_DEBUG, "code NULL");
        }
        ret = I_ERROR_PARAM;
      }
    } else {
      switch (i_session->response_type) {
        case I_RESPONSE_TYPE_PASSWORD:
          if (i_session->username != NULL && i_session->user_password != NULL) {
            ulfius_init_request(&request);
            ulfius_init_response(&response);
            request.http_verb = o_strdup("POST");
            request.http_url = o_strdup(i_session->token_endpoint);
            u_map_put(request.map_post_body, "grant_type", "password");
            u_map_put(request.map_post_body, "username", i_session->username);
            u_map_put(request.map_post_body, "password", i_session->user_password);
            if (i_session->client_secret != NULL) {
              request.auth_basic_user = o_strdup(i_session->client_id);
              request.auth_basic_password = o_strdup(i_session->client_secret);
            }
            if (i_session->scope != NULL) {
              u_map_put(request.map_post_body, "scope", i_session->scope);
            }
            if (ulfius_send_http_request(&request, &response) == U_OK) {
              if (response.status == 200 || response.status == 400) {
                j_response = ulfius_get_json_body_response(&response, NULL);
                if (j_response != NULL) {
                  if (parse_token_response(i_session, response.status, j_response) == I_OK) {
                    ret = response.status == 200?I_OK:I_ERROR_PARAM;
                  } else {
                    y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request password - Error parse_token_response");
                    ret = I_ERROR_PARAM;
                  }
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request password - Error parsing JSON response");
                  ret = I_ERROR;
                }
                json_decref(j_response);
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request password - Invalid response status");
                ret = I_ERROR;
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request password - Error sending token request");
              ret = I_ERROR;
            }
            ulfius_clean_request(&request);
            ulfius_clean_response(&response);
          } else {
            ret = I_ERROR_PARAM;
          }
          break;
        case I_RESPONSE_TYPE_CLIENT_CREDENTIALS:
          if (i_session->client_id != NULL && i_session->client_secret != NULL) {
            ulfius_init_request(&request);
            ulfius_init_response(&response);
            request.http_verb = o_strdup("POST");
            request.http_url = o_strdup(i_session->token_endpoint);
            u_map_put(request.map_post_body, "grant_type", "client_credentials");
            request.auth_basic_user = o_strdup(i_session->client_id);
            request.auth_basic_password = o_strdup(i_session->client_secret);
            if (i_session->scope != NULL) {
              u_map_put(request.map_post_body, "scope", i_session->scope);
            }
            if (ulfius_send_http_request(&request, &response) == U_OK) {
              if (response.status == 200 || response.status == 400) {
                j_response = ulfius_get_json_body_response(&response, NULL);
                if (j_response != NULL) {
                  if (parse_token_response(i_session, response.status, j_response) == I_OK) {
                    ret = response.status == 200?I_OK:I_ERROR_PARAM;
                  } else {
                    y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request client_credentials - Error parse_token_response");
                    ret = I_ERROR_PARAM;
                  }
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request client_credentials - Error parsing JSON response");
                  ret = I_ERROR;
                }
                json_decref(j_response);
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request client_credentials - Invalid response status");
                ret = I_ERROR;
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request client_credentials - Error sending token request");
              ret = I_ERROR;
            }
            ulfius_clean_request(&request);
            ulfius_clean_response(&response);
          } else {
            ret = I_ERROR_PARAM;
          }
          break;
        case I_RESPONSE_TYPE_REFRESH_TOKEN:
          if (i_session->refresh_token != NULL) {
            ulfius_init_request(&request);
            ulfius_init_response(&response);
            request.http_verb = o_strdup("POST");
            request.http_url = o_strdup(i_session->token_endpoint);
            u_map_put(request.map_post_body, "grant_type", "refresh_token");
            if (i_session->client_id != NULL && i_session->client_secret != NULL) {
              request.auth_basic_user = o_strdup(i_session->client_id);
              request.auth_basic_password = o_strdup(i_session->client_secret);
            }
            if (i_session->scope != NULL) {
              u_map_put(request.map_post_body, "scope", i_session->scope);
            }
            if (ulfius_send_http_request(&request, &response) == U_OK) {
              if (response.status == 200 || response.status == 400) {
                j_response = ulfius_get_json_body_response(&response, NULL);
                if (j_response != NULL) {
                  if (parse_token_response(i_session, response.status, j_response) == I_OK) {
                    ret = response.status == 200?I_OK:I_ERROR_PARAM;
                  } else {
                    y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request refresh - Error parse_token_response");
                    ret = I_ERROR_PARAM;
                  }
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request refresh - Error parsing JSON response");
                  ret = I_ERROR;
                }
                json_decref(j_response);
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request refresh - Invalid response status");
                ret = I_ERROR;
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "i_run_token_request refresh - Error sending token request");
              ret = I_ERROR;
            }
            ulfius_clean_request(&request);
            ulfius_clean_response(&response);
          } else {
            ret = I_ERROR_PARAM;
          }
          break;
        default:
          ret = I_ERROR_PARAM;
          break;
      }
    }
  } else {
    ret = I_ERROR_PARAM;
  }
  return ret;
}

int i_verify_id_token(struct _i_session * i_session, int token_type) {
  int ret = I_ERROR_PARAM;
  jwt_t * jwt;
  jwk_t * jwk = NULL;
  size_t i, len = 0, hash_len = 128, hash_encoded_len = 128;
  unsigned char * pem = NULL, hash[128], hash_encoded[128] = {0};
  char * jwt_str;
  json_t * j_tmp;
  json_error_t j_error;
  int alg = GNUTLS_DIG_UNKNOWN;
  gnutls_datum_t hash_data;
  time_t now = 0;
  
  if (i_session != NULL && i_session->id_token != NULL && r_jwks_size(i_session->jwks)) {
    for (i=0; i<r_jwks_size(i_session->jwks); i++) {
      jwk = r_jwks_get_at(i_session->jwks, i);
      if (jwk != NULL) {
        if (r_jwk_export_to_pem_der(jwk, R_FORMAT_PEM, NULL, &len, i_session->x5u_flags) == RHN_ERROR_PARAM && len) {
          if ((pem = o_malloc(len)) != NULL) {
            if (r_jwk_export_to_pem_der(jwk, R_FORMAT_PEM, pem, &len, i_session->x5u_flags) == RHN_OK) {
              if (!jwt_decode(&jwt, i_session->id_token, pem, len)) {
                json_decref(i_session->id_token_payload);
                i_session->id_token_payload = NULL;
                if ((jwt_str = jwt_dump_str(jwt, 0)) != NULL) {
                  if ((j_tmp = json_loads(jwt_str, JSON_DECODE_ANY|JSON_DISABLE_EOF_CHECK, &j_error)) != NULL) {
                    if (r_jwk_import_from_json_t(i_session->id_token_header, j_tmp) == RHN_OK) {
                      if ((i_session->id_token_payload = json_loads(jwt_str+j_error.position+1, JSON_DECODE_ANY, NULL)) != NULL) {
                        time(&now);
                        if (json_object_get(i_session->id_token_payload, "iat") == NULL || json_integer_value(json_object_get(i_session->id_token_payload, "iat")) > now) {
                          y_log_message(Y_LOG_LEVEL_DEBUG, "i_verify_id_token - invalid iat value");
                          ret = I_ERROR_PARAM;
                        } else if (json_object_get(i_session->id_token_payload, "exp") == NULL || json_integer_value(json_object_get(i_session->id_token_payload, "exp")) < now) {
                          y_log_message(Y_LOG_LEVEL_DEBUG, "i_verify_id_token - invalid exp value");
                          ret = I_ERROR_PARAM;
                        } else if (!json_string_length(json_object_get(i_session->id_token_payload, "iss"))) {
                          y_log_message(Y_LOG_LEVEL_DEBUG, "i_verify_id_token - required value iss missing");
                          ret = I_ERROR_PARAM;
                        } else if (i_session->issuer != NULL && 0 != o_strcmp(i_session->issuer, json_string_value(json_object_get(i_session->id_token_payload, "iss")))) {
                          y_log_message(Y_LOG_LEVEL_DEBUG, "i_verify_id_token - invalid iss");
                          ret = I_ERROR_PARAM;
                        } else if (!json_string_length(json_object_get(i_session->id_token_payload, "sub"))) {
                          y_log_message(Y_LOG_LEVEL_DEBUG, "i_verify_id_token - required value sub missing");
                          ret = I_ERROR_PARAM;
                        } else if (!json_string_length(json_object_get(i_session->id_token_payload, "aud"))) {
                          y_log_message(Y_LOG_LEVEL_DEBUG, "i_verify_id_token - required value aud missing");
                          ret = I_ERROR_PARAM;
                        } else {
                          ret = I_OK;
                          if (token_type & I_HAS_ACCESS_TOKEN) {
                            if (json_object_get(i_session->id_token_payload, "at_hash") != NULL && i_session->access_token != NULL) {
                              alg = GNUTLS_DIG_UNKNOWN;
                              if ((jwt_get_alg(jwt) == JWT_ALG_HS256 && has_openid_config_parameter_value(i_session, "token_endpoint_auth_signing_alg_values_supported", "HS256")) || 
                              (jwt_get_alg(jwt) == JWT_ALG_RS256 && has_openid_config_parameter_value(i_session, "token_endpoint_auth_signing_alg_values_supported", "RS256")) || 
                              (jwt_get_alg(jwt) == JWT_ALG_ES256 && has_openid_config_parameter_value(i_session, "token_endpoint_auth_signing_alg_values_supported", "ES256"))) {
                                alg = GNUTLS_DIG_SHA256;
                              } else if ((jwt_get_alg(jwt) == JWT_ALG_HS384 && has_openid_config_parameter_value(i_session, "token_endpoint_auth_signing_alg_values_supported", "HS384")) || 
                              (jwt_get_alg(jwt) == JWT_ALG_RS384 && has_openid_config_parameter_value(i_session, "token_endpoint_auth_signing_alg_values_supported", "RS384")) || 
                              (jwt_get_alg(jwt) == JWT_ALG_ES384 && has_openid_config_parameter_value(i_session, "token_endpoint_auth_signing_alg_values_supported", "ES384"))) {
                                alg = GNUTLS_DIG_SHA384;
                              } else if ((jwt_get_alg(jwt) == JWT_ALG_HS512 && has_openid_config_parameter_value(i_session, "token_endpoint_auth_signing_alg_values_supported", "HS512")) || 
                              (jwt_get_alg(jwt) == JWT_ALG_RS512 && has_openid_config_parameter_value(i_session, "token_endpoint_auth_signing_alg_values_supported", "RS512")) || 
                              (jwt_get_alg(jwt) == JWT_ALG_ES512 && has_openid_config_parameter_value(i_session, "token_endpoint_auth_signing_alg_values_supported", "ES512"))) {
                                alg = GNUTLS_DIG_SHA384;
                              }
                              if (alg != GNUTLS_DIG_UNKNOWN) {
                                hash_data.data = (unsigned char*)i_session->access_token;
                                hash_data.size = o_strlen(i_session->access_token);
                                if (gnutls_fingerprint(alg, &hash_data, hash, &hash_len) == GNUTLS_E_SUCCESS) {
                                  if (o_base64url_encode(hash, hash_len/2, hash_encoded, &hash_encoded_len)) {
                                    if (o_strcmp((const char *)hash_encoded, json_string_value(json_object_get(i_session->id_token_payload, "at_hash"))) != 0) {
                                      y_log_message(Y_LOG_LEVEL_DEBUG, "i_verify_id_token at - at_hash invalid");
                                      ret = I_ERROR_PARAM;
                                    }
                                  } else {
                                    y_log_message(Y_LOG_LEVEL_ERROR, "i_verify_id_token at - Error o_base64url_encode at_hash");
                                    ret = I_ERROR;
                                  }
                                } else {
                                  y_log_message(Y_LOG_LEVEL_ERROR, "i_verify_id_token at - Error gnutls_fingerprint at_hash");
                                  ret = I_ERROR;
                                }
                              } else {
                                y_log_message(Y_LOG_LEVEL_DEBUG, "i_verify_id_token at - Invalid alg");
                                ret = I_ERROR_PARAM;
                              }
                            } else {
                              y_log_message(Y_LOG_LEVEL_DEBUG, "i_verify_id_token at - missing input");
                              ret = I_ERROR_PARAM;
                            }
                          }
                          if (token_type & I_HAS_CODE) {
                            if (json_object_get(i_session->id_token_payload, "c_hash") != NULL && i_session->code != NULL) {
                              alg = GNUTLS_DIG_UNKNOWN;
                              if (jwt_get_alg(jwt) == JWT_ALG_HS256 || jwt_get_alg(jwt) == JWT_ALG_RS256 || jwt_get_alg(jwt) == JWT_ALG_ES256) {
                                alg = GNUTLS_DIG_SHA256;
                              } else if (jwt_get_alg(jwt) == JWT_ALG_HS384 || jwt_get_alg(jwt) == JWT_ALG_RS384 || jwt_get_alg(jwt) == JWT_ALG_ES384) {
                                alg = GNUTLS_DIG_SHA384;
                              } else if (jwt_get_alg(jwt) == JWT_ALG_HS512 || jwt_get_alg(jwt) == JWT_ALG_RS512 || jwt_get_alg(jwt) == JWT_ALG_ES512) {
                                alg = GNUTLS_DIG_SHA384;
                              }
                              if (alg != GNUTLS_DIG_UNKNOWN) {
                                hash_data.data = (unsigned char*)i_session->code;
                                hash_data.size = o_strlen(i_session->code);
                                if (gnutls_fingerprint(alg, &hash_data, hash, &hash_len) == GNUTLS_E_SUCCESS) {
                                  if (o_base64url_encode(hash, hash_len/2, hash_encoded, &hash_encoded_len)) {
                                    if (o_strcmp((const char *)hash_encoded, json_string_value(json_object_get(i_session->id_token_payload, "c_hash"))) != 0) {
                                      y_log_message(Y_LOG_LEVEL_DEBUG, "i_verify_id_token - c_hash invalid");
                                      ret = I_ERROR_PARAM;
                                    }
                                  } else {
                                    y_log_message(Y_LOG_LEVEL_ERROR, "i_verify_id_token at - Error o_base64url_encode c_hash");
                                    ret = I_ERROR;
                                  }
                                } else {
                                  y_log_message(Y_LOG_LEVEL_ERROR, "i_verify_id_token at - Error gnutls_fingerprint c_hash");
                                  ret = I_ERROR;
                                }
                              } else {
                                y_log_message(Y_LOG_LEVEL_DEBUG, "i_verify_id_token at - unknown alg");
                                ret = I_ERROR_PARAM;
                              }
                            } else {
                              y_log_message(Y_LOG_LEVEL_DEBUG, "i_verify_id_token at - missing input");
                              ret = I_ERROR_PARAM;
                            }
                          }
                        }
                      } else {
                        y_log_message(Y_LOG_LEVEL_ERROR, "i_verify_id_token at - Error json_loads id_token_payload");
                        ret = I_ERROR;
                      }
                    } else {
                      y_log_message(Y_LOG_LEVEL_ERROR, "i_verify_id_token at - Error r_jwk_import_from_json_t");
                      ret = I_ERROR;
                    }
                    json_decref(j_tmp);
                  } else {
                    y_log_message(Y_LOG_LEVEL_ERROR, "i_verify_id_token at - Error json_loads id_token_header");
                    ret = I_ERROR;
                  }
                  o_free(jwt_str);
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "i_verify_id_token at - Error jwt_dump_str");
                  ret = I_ERROR;
                }
                jwt_free(jwt);
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "i_verify_id_token - Error r_jwk_export_to_pem_der (2) at index %zu", i);
              ret = I_ERROR;
            }
            o_free(pem);
            len = 0;
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "i_verify_id_token - Error o_malloc pem at index %zu", i);
            ret = I_ERROR;
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "i_verify_id_token - Error r_jwk_export_to_pem_der (1) at index %zu", i);
          ret = I_ERROR;
        }
        r_free_jwk(jwk);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "i_verify_id_token - Error getting jwk at index %zu", i);
        ret = I_ERROR;
      }
    }
  } else {
    ret = I_ERROR_PARAM;
  }
  
  return ret;
}
