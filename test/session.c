/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>

#include <jansson.h>
#include <check.h>
#include <yder.h>
#include <iddawc.h>
#include <rhonabwy.h>

#define CLIENT_ID "client"
#define END_SESSION_ENDPOINT "https://as.tld/end_session/"
#define ID_TOKEN "id_tokenXyz1234"
#define ID_TOKEN_SID "sidXyz1234"
#define POST_LOGOUT_REDIRECT_URI "https://client.tld/postLogout/"
#define POST_LOGOUT_REDIRECT_URI_ENC "https%3A%2F%2Fclient.tld%2FpostLogout%2F"
#define ISSUER "https://as.tld"
#define EXPIRES_IN 3600
#define SUB "wRNaPT1UBIw4Cl9eo3yOzoH7vE81Phfu"
#define VALUE "valueToCheck"
#define PKCE_CODE_VERIFIER "PKCECodeVerifier123456789012345678901234567890"

const unsigned char public_key[] = 
"-----BEGIN PUBLIC KEY-----\n"
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAnzyis1ZjfNB0bBgKFMSv\n"
"vkTtwlvBsaJq7S5wA+kzeVOVpVWwkWdVha4s38XM/pa/yr47av7+z3VTmvDRyAHc\n"
"aT92whREFpLv9cj5lTeJSibyr/Mrm/YtjCZVWgaOYIhwrXwKLqPr/11inWsAkfIy\n"
"tvHWTxZYEcXLgAXFuUuaS3uF9gEiNQwzGTU1v0FqkqTBr4B8nW3HCN47XUu0t8Y0\n"
"e+lf4s4OxQawWD79J9/5d3Ry0vbV3Am1FtGJiJvOwRsIfVChDpYStTcHTCMqtvWb\n"
"V6L11BWkpzGXSW4Hv43qa+GSYOD2QU68Mb59oSk2OB+BtOLpJofmbGEGgvmwyCI9\n"
"MwIDAQAB\n"
"-----END PUBLIC KEY-----\n";

const unsigned char private_key[] =
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEogIBAAKCAQEAnzyis1ZjfNB0bBgKFMSvvkTtwlvBsaJq7S5wA+kzeVOVpVWw\n"
"kWdVha4s38XM/pa/yr47av7+z3VTmvDRyAHcaT92whREFpLv9cj5lTeJSibyr/Mr\n"
"m/YtjCZVWgaOYIhwrXwKLqPr/11inWsAkfIytvHWTxZYEcXLgAXFuUuaS3uF9gEi\n"
"NQwzGTU1v0FqkqTBr4B8nW3HCN47XUu0t8Y0e+lf4s4OxQawWD79J9/5d3Ry0vbV\n"
"3Am1FtGJiJvOwRsIfVChDpYStTcHTCMqtvWbV6L11BWkpzGXSW4Hv43qa+GSYOD2\n"
"QU68Mb59oSk2OB+BtOLpJofmbGEGgvmwyCI9MwIDAQABAoIBACiARq2wkltjtcjs\n"
"kFvZ7w1JAORHbEufEO1Eu27zOIlqbgyAcAl7q+/1bip4Z/x1IVES84/yTaM8p0go\n"
"amMhvgry/mS8vNi1BN2SAZEnb/7xSxbflb70bX9RHLJqKnp5GZe2jexw+wyXlwaM\n"
"+bclUCrh9e1ltH7IvUrRrQnFJfh+is1fRon9Co9Li0GwoN0x0byrrngU8Ak3Y6D9\n"
"D8GjQA4Elm94ST3izJv8iCOLSDBmzsPsXfcCUZfmTfZ5DbUDMbMxRnSo3nQeoKGC\n"
"0Lj9FkWcfmLcpGlSXTO+Ww1L7EGq+PT3NtRae1FZPwjddQ1/4V905kyQFLamAA5Y\n"
"lSpE2wkCgYEAy1OPLQcZt4NQnQzPz2SBJqQN2P5u3vXl+zNVKP8w4eBv0vWuJJF+\n"
"hkGNnSxXQrTkvDOIUddSKOzHHgSg4nY6K02ecyT0PPm/UZvtRpWrnBjcEVtHEJNp\n"
"bU9pLD5iZ0J9sbzPU/LxPmuAP2Bs8JmTn6aFRspFrP7W0s1Nmk2jsm0CgYEAyH0X\n"
"+jpoqxj4efZfkUrg5GbSEhf+dZglf0tTOA5bVg8IYwtmNk/pniLG/zI7c+GlTc9B\n"
"BwfMr59EzBq/eFMI7+LgXaVUsM/sS4Ry+yeK6SJx/otIMWtDfqxsLD8CPMCRvecC\n"
"2Pip4uSgrl0MOebl9XKp57GoaUWRWRHqwV4Y6h8CgYAZhI4mh4qZtnhKjY4TKDjx\n"
"QYufXSdLAi9v3FxmvchDwOgn4L+PRVdMwDNms2bsL0m5uPn104EzM6w1vzz1zwKz\n"
"5pTpPI0OjgWN13Tq8+PKvm/4Ga2MjgOgPWQkslulO/oMcXbPwWC3hcRdr9tcQtn9\n"
"Imf9n2spL/6EDFId+Hp/7QKBgAqlWdiXsWckdE1Fn91/NGHsc8syKvjjk1onDcw0\n"
"NvVi5vcba9oGdElJX3e9mxqUKMrw7msJJv1MX8LWyMQC5L6YNYHDfbPF1q5L4i8j\n"
"8mRex97UVokJQRRA452V2vCO6S5ETgpnad36de3MUxHgCOX3qL382Qx9/THVmbma\n"
"3YfRAoGAUxL/Eu5yvMK8SAt/dJK6FedngcM3JEFNplmtLYVLWhkIlNRGDwkg3I5K\n"
"y18Ae9n7dHVueyslrb6weq7dTkYDi3iOYRW8HRkIQh06wEdbxt0shTzAJvvCQfrB\n"
"jg/3747WSsf/zBTcHihTRBdAv6OmdhV4/dD5YBfLAkLrd+mX7iE=\n"
"-----END RSA PRIVATE KEY-----\n";

const char jwk_pubkey_str_2[] = "{\"kty\":\"RSA\",\"n\":\"0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_BJECPebWKRX"\
                                 "jBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6"\
                                 "qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw\""\
                                 ",\"e\":\"AQAB\",\"alg\":\"RSA1_5\",\"kid\":\"2\"}";
const char jwk_privkey_str_2[] = "{\"kty\":\"RSA\",\"n\":\"0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_BJECPebWKR"\
                                  "XjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHz"\
                                  "u6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKg"\
                                  "w\",\"e\":\"AQAB\",\"d\":\"X4cTteJY_gn4FYPsXB8rdXix5vwsg1FLN5E3EaG6RJoVH-HLLKD9M7dx5oo7GURknchnrRweUkC7hT5fJLM0WbFAKNLWY2v"\
                                  "v7B6NqXSzUvxT0_YSfqijwp3RTzlBaCxWp4doFk5N2o8Gy_nHNKroADIkJ46pRUohsXywbReAdYaMwFs9tv8d_cPVY3i07a3t8MN6TNwm0dSawm9v47UiCl3Sk"\
                                  "5ZiG7xojPLu4sbg1U2jx4IBTNBznbJSzFHK66jT8bgkuqsk0GjskDJk19Z4qwjwbsnn4j2WBii3RL-Us2lGVkY8fkFzme1z0HbIkfz0Y6mqnOYtqc0X4jfcKoA"\
                                  "C8Q\",\"p\":\"83i-7IvMGXoMXCskv73TKr8637FiO7Z27zv8oj6pbWUQyLPQBQxtPVnwD20R-60eTDmD2ujnMt5PoqMrm8RfmNhVWDtjjMmCMjOpSXicFHj7"\
                                  "XOuVIYQyqVWlWEh6dN36GVZYk93N8Bc9vY41xy8B9RzzOGVQzXvNEvn7O0nVbfs\",\"q\":\"3dfOR9cuYq-0S-mkFLzgItgMEfFzB2q3hWehMuG0oCuqnb3v"\
                                  "obLyumqjVZQO1dIrdwgTnCdpYzBcOfW5r370AFXjiWft_NGEiovonizhKpo9VVS78TzFgxkIdrecRezsZ-1kYd_s1qDbxtkDEgfAITAG9LUnADun4vIcb6yelx"\
                                  "k\",\"dp\":\"G4sPXkc6Ya9y8oJW9_ILj4xuppu0lzi_H7VTkS8xj5SdX3coE0oimYwxIi2emTAue0UOa5dpgFGyBJ4c8tQ2VF402XRugKDTP8akYhFo5tAA7"\
                                  "7Qe_NmtuYZc3C3m3I24G2GvR5sSDxUyAN2zq8Lfn9EUms6rY3Ob8YeiKkTiBj0\",\"dq\":\"s9lAH9fggBsoFR8Oac2R_E2gw282rT2kGOAhvIllETE1efrA"\
                                  "6huUUvMfBcMpn8lqeW6vzznYY5SSQF7pMdC_agI3nG8Ibp1BUb0JUiraRNqUfLhcQb_d9GF4Dh7e74WbRsobRonujTYN1xCaP6TO61jvWrX-L18txXw494Q_cg"\
                                  "k\",\"qi\":\"GyM_p6JrXySiz1toFgKbWV-JdI3jQ4ypu9rbMWx3rQJBfmt0FoYzgUIZEVFEcOqwemRN81zoDAaa-Bk0KWNGDjJHZDdDmFhW3AN7lI-puxk_m"\
                                  "HZGJ11rxyR8O55XLSe3SPmRfKwZI6yU24ZxvQKFYItdldUKGzO6Ia6zTKhAVRU\",\"alg\":\"RSA1_5\",\"kid\":\"2\"}";
const char end_session_token_pattern[] = "{\"aud\":\"%s\",\"iat\":%lld,\"exp\":%lld,\"iss\":\"%s\",\"jti\":\"abc1234\",\"events\":{\"http://schemas.openid.net/event/backchannel-logout\":{}}}";

START_TEST(test_iddawc_session_build_url)
{
  struct _i_session i_session;
  char * url;

  ck_assert_int_eq(i_init_session(&i_session), I_OK);

  ck_assert_ptr_eq(NULL, i_build_end_session_url(&i_session));
  ck_assert_int_eq(i_set_str_parameter(&i_session, I_OPT_END_SESSION_ENDPOINT, END_SESSION_ENDPOINT), I_OK);
  ck_assert_ptr_eq(NULL, i_build_end_session_url(&i_session));
  ck_assert_int_eq(i_set_str_parameter(&i_session, I_OPT_ID_TOKEN, ID_TOKEN), I_OK);
  ck_assert_ptr_ne(NULL, url = i_build_end_session_url(&i_session));
  ck_assert_str_eq(url, END_SESSION_ENDPOINT "?id_token_hint=" ID_TOKEN);
  o_free(url);

  ck_assert_int_eq(i_set_str_parameter(&i_session, I_OPT_POST_LOGOUT_REDIRECT_URI, POST_LOGOUT_REDIRECT_URI), I_OK);
  ck_assert_ptr_ne(NULL, url = i_build_end_session_url(&i_session));
  ck_assert_str_eq(url, END_SESSION_ENDPOINT "?id_token_hint=" ID_TOKEN "&post_logout_redirect_uri=" POST_LOGOUT_REDIRECT_URI_ENC);
  o_free(url);

  i_clean_session(&i_session);
}
END_TEST

START_TEST(test_iddawc_session_verify_token_ok)
{
  struct _i_session i_session;
  jwt_t * jwt;
  jwk_t * jwk, * jwk_sign;
  char * grants = NULL, * jwt_str;
  time_t now;
  time(&now);
  ck_assert_int_eq(r_jwk_init(&jwk), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk, R_X509_TYPE_PRIVKEY, R_FORMAT_PEM, private_key, o_strlen((const char *)private_key)), RHN_OK);
  ck_assert_int_eq(r_jwk_init(&jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk_sign, R_X509_TYPE_PUBKEY, R_FORMAT_PEM, public_key, o_strlen((const char *)public_key)), RHN_OK);
  ck_assert_ptr_ne(grants = msprintf(end_session_token_pattern, CLIENT_ID, (long long)now, (long long)(now + EXPIRES_IN), ISSUER), NULL);

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_ISSUER, ISSUER,
                                                    I_OPT_CLIENT_ID, CLIENT_ID,
                                                    I_OPT_BACKCHANNEL_LOGOUT_SESSION_REQUIRED, 0,
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(r_jwks_append_jwk(i_session.server_jwks, jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwt_init(&jwt), RHN_OK);
  ck_assert_int_eq(r_jwt_set_sign_alg(jwt, R_JWA_ALG_RS256), RHN_OK);
  ck_assert_int_eq(r_jwt_set_full_claims_json_str(jwt, grants), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "sub", SUB), RHN_OK);
  ck_assert_ptr_ne((jwt_str = r_jwt_serialize_signed(jwt, jwk, 0)), NULL);
  ck_assert_int_eq(I_OK, i_verify_end_session_backchannel_token(&i_session, jwt_str));
  o_free(jwt_str);
  r_jwt_free(jwt);
  i_clean_session(&i_session);

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_ISSUER, ISSUER,
                                                    I_OPT_CLIENT_ID, CLIENT_ID,
                                                    I_OPT_BACKCHANNEL_LOGOUT_SESSION_REQUIRED, 0,
                                                    I_OPT_ID_TOKEN_SID, ID_TOKEN_SID,
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(r_jwks_append_jwk(i_session.server_jwks, jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwt_init(&jwt), RHN_OK);
  ck_assert_int_eq(r_jwt_set_sign_alg(jwt, R_JWA_ALG_RS256), RHN_OK);
  ck_assert_int_eq(r_jwt_set_full_claims_json_str(jwt, grants), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "sid", ID_TOKEN_SID), RHN_OK);
  ck_assert_ptr_ne((jwt_str = r_jwt_serialize_signed(jwt, jwk, 0)), NULL);
  ck_assert_int_eq(I_OK, i_verify_end_session_backchannel_token(&i_session, jwt_str));
  o_free(jwt_str);
  r_jwt_free(jwt);
  i_clean_session(&i_session);

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_ISSUER, ISSUER,
                                                    I_OPT_CLIENT_ID, CLIENT_ID,
                                                    I_OPT_BACKCHANNEL_LOGOUT_SESSION_REQUIRED, 1,
                                                    I_OPT_ID_TOKEN_SID, ID_TOKEN_SID,
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(r_jwks_append_jwk(i_session.server_jwks, jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwt_init(&jwt), RHN_OK);
  ck_assert_int_eq(r_jwt_set_sign_alg(jwt, R_JWA_ALG_RS256), RHN_OK);
  ck_assert_int_eq(r_jwt_set_full_claims_json_str(jwt, grants), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "sid", ID_TOKEN_SID), RHN_OK);
  ck_assert_ptr_ne((jwt_str = r_jwt_serialize_signed(jwt, jwk, 0)), NULL);
  ck_assert_int_eq(I_OK, i_verify_end_session_backchannel_token(&i_session, jwt_str));
  o_free(jwt_str);
  r_jwt_free(jwt);
  i_clean_session(&i_session);

  o_free(grants);
  r_jwk_free(jwk);
  r_jwk_free(jwk_sign);
}
END_TEST

START_TEST(test_iddawc_session_verify_token_invalid_iss)
{
  struct _i_session i_session;
  jwt_t * jwt;
  jwk_t * jwk, * jwk_sign;
  char * grants = NULL, * jwt_str;
  time_t now;
  time(&now);
  ck_assert_int_eq(r_jwk_init(&jwk), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk, R_X509_TYPE_PRIVKEY, R_FORMAT_PEM, private_key, o_strlen((const char *)private_key)), RHN_OK);
  ck_assert_int_eq(r_jwk_init(&jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk_sign, R_X509_TYPE_PUBKEY, R_FORMAT_PEM, public_key, o_strlen((const char *)public_key)), RHN_OK);
  ck_assert_ptr_ne(grants = msprintf(end_session_token_pattern, CLIENT_ID, (long long)now, (long long)(now + EXPIRES_IN), ISSUER), NULL);

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_ISSUER, "error",
                                                    I_OPT_CLIENT_ID, CLIENT_ID,
                                                    I_OPT_BACKCHANNEL_LOGOUT_SESSION_REQUIRED, 1,
                                                    I_OPT_ID_TOKEN_SID, ID_TOKEN_SID,
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(r_jwks_append_jwk(i_session.server_jwks, jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwt_init(&jwt), RHN_OK);
  ck_assert_int_eq(r_jwt_set_sign_alg(jwt, R_JWA_ALG_RS256), RHN_OK);
  ck_assert_int_eq(r_jwt_set_full_claims_json_str(jwt, grants), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "sid", ID_TOKEN_SID), RHN_OK);
  ck_assert_ptr_ne((jwt_str = r_jwt_serialize_signed(jwt, jwk, 0)), NULL);
  ck_assert_int_eq(I_ERROR_PARAM, i_verify_end_session_backchannel_token(&i_session, jwt_str));
  o_free(jwt_str);
  r_jwt_free(jwt);
  i_clean_session(&i_session);

  o_free(grants);
  r_jwk_free(jwk);
  r_jwk_free(jwk_sign);
}
END_TEST

START_TEST(test_iddawc_session_verify_token_invalid_aud)
{
  struct _i_session i_session;
  jwt_t * jwt;
  jwk_t * jwk, * jwk_sign;
  char * grants = NULL, * jwt_str;
  time_t now;
  time(&now);
  ck_assert_int_eq(r_jwk_init(&jwk), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk, R_X509_TYPE_PRIVKEY, R_FORMAT_PEM, private_key, o_strlen((const char *)private_key)), RHN_OK);
  ck_assert_int_eq(r_jwk_init(&jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk_sign, R_X509_TYPE_PUBKEY, R_FORMAT_PEM, public_key, o_strlen((const char *)public_key)), RHN_OK);
  ck_assert_ptr_ne(grants = msprintf(end_session_token_pattern, CLIENT_ID, (long long)now, (long long)(now + EXPIRES_IN), ISSUER), NULL);

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_ISSUER, ISSUER,
                                                    I_OPT_CLIENT_ID, "error",
                                                    I_OPT_BACKCHANNEL_LOGOUT_SESSION_REQUIRED, 1,
                                                    I_OPT_ID_TOKEN_SID, ID_TOKEN_SID,
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(r_jwks_append_jwk(i_session.server_jwks, jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwt_init(&jwt), RHN_OK);
  ck_assert_int_eq(r_jwt_set_sign_alg(jwt, R_JWA_ALG_RS256), RHN_OK);
  ck_assert_int_eq(r_jwt_set_full_claims_json_str(jwt, grants), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "sid", ID_TOKEN_SID), RHN_OK);
  ck_assert_ptr_ne((jwt_str = r_jwt_serialize_signed(jwt, jwk, 0)), NULL);
  ck_assert_int_eq(I_ERROR_PARAM, i_verify_end_session_backchannel_token(&i_session, jwt_str));
  o_free(jwt_str);
  r_jwt_free(jwt);
  i_clean_session(&i_session);

  o_free(grants);
  r_jwk_free(jwk);
  r_jwk_free(jwk_sign);
}
END_TEST

START_TEST(test_iddawc_session_verify_token_invalid_iat)
{
  struct _i_session i_session;
  jwt_t * jwt;
  jwk_t * jwk, * jwk_sign;
  char * grants = NULL, * jwt_str;
  time_t now;
  time(&now);
  ck_assert_int_eq(r_jwk_init(&jwk), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk, R_X509_TYPE_PRIVKEY, R_FORMAT_PEM, private_key, o_strlen((const char *)private_key)), RHN_OK);
  ck_assert_int_eq(r_jwk_init(&jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk_sign, R_X509_TYPE_PUBKEY, R_FORMAT_PEM, public_key, o_strlen((const char *)public_key)), RHN_OK);
  ck_assert_ptr_ne(grants = msprintf(end_session_token_pattern, CLIENT_ID, (long long)(now + (EXPIRES_IN/2)), (long long)(now + EXPIRES_IN), ISSUER), NULL);

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_ISSUER, ISSUER,
                                                    I_OPT_CLIENT_ID, CLIENT_ID,
                                                    I_OPT_BACKCHANNEL_LOGOUT_SESSION_REQUIRED, 1,
                                                    I_OPT_ID_TOKEN_SID, ID_TOKEN_SID,
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(r_jwks_append_jwk(i_session.server_jwks, jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwt_init(&jwt), RHN_OK);
  ck_assert_int_eq(r_jwt_set_sign_alg(jwt, R_JWA_ALG_RS256), RHN_OK);
  ck_assert_int_eq(r_jwt_set_full_claims_json_str(jwt, grants), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "sid", ID_TOKEN_SID), RHN_OK);
  ck_assert_ptr_ne((jwt_str = r_jwt_serialize_signed(jwt, jwk, 0)), NULL);
  ck_assert_int_eq(I_ERROR_PARAM, i_verify_end_session_backchannel_token(&i_session, jwt_str));
  o_free(jwt_str);
  r_jwt_free(jwt);
  i_clean_session(&i_session);

  o_free(grants);
  r_jwk_free(jwk);
  r_jwk_free(jwk_sign);
}
END_TEST

START_TEST(test_iddawc_session_verify_token_invalid_jti)
{
  struct _i_session i_session;
  jwt_t * jwt;
  jwk_t * jwk, * jwk_sign;
  char * grants = NULL, * jwt_str;
  time_t now;
  time(&now);
  ck_assert_int_eq(r_jwk_init(&jwk), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk, R_X509_TYPE_PRIVKEY, R_FORMAT_PEM, private_key, o_strlen((const char *)private_key)), RHN_OK);
  ck_assert_int_eq(r_jwk_init(&jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk_sign, R_X509_TYPE_PUBKEY, R_FORMAT_PEM, public_key, o_strlen((const char *)public_key)), RHN_OK);
  ck_assert_ptr_ne(grants = msprintf(end_session_token_pattern, CLIENT_ID, (long long)now, (long long)(now + EXPIRES_IN), ISSUER), NULL);

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_ISSUER, ISSUER,
                                                    I_OPT_CLIENT_ID, CLIENT_ID,
                                                    I_OPT_BACKCHANNEL_LOGOUT_SESSION_REQUIRED, 1,
                                                    I_OPT_ID_TOKEN_SID, ID_TOKEN_SID,
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(r_jwks_append_jwk(i_session.server_jwks, jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwt_init(&jwt), RHN_OK);
  ck_assert_int_eq(r_jwt_set_sign_alg(jwt, R_JWA_ALG_RS256), RHN_OK);
  ck_assert_int_eq(r_jwt_set_full_claims_json_str(jwt, grants), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "jti", NULL), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "sid", ID_TOKEN_SID), RHN_OK);
  ck_assert_ptr_ne((jwt_str = r_jwt_serialize_signed(jwt, jwk, 0)), NULL);
  ck_assert_int_eq(I_ERROR_PARAM, i_verify_end_session_backchannel_token(&i_session, jwt_str));
  o_free(jwt_str);
  r_jwt_free(jwt);
  i_clean_session(&i_session);

  o_free(grants);
  r_jwk_free(jwk);
  r_jwk_free(jwk_sign);
}
END_TEST

START_TEST(test_iddawc_session_verify_token_invalid_events)
{
  struct _i_session i_session;
  jwt_t * jwt;
  jwk_t * jwk, * jwk_sign;
  char * grants = NULL, * jwt_str;
  time_t now;
  time(&now);
  ck_assert_int_eq(r_jwk_init(&jwk), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk, R_X509_TYPE_PRIVKEY, R_FORMAT_PEM, private_key, o_strlen((const char *)private_key)), RHN_OK);
  ck_assert_int_eq(r_jwk_init(&jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk_sign, R_X509_TYPE_PUBKEY, R_FORMAT_PEM, public_key, o_strlen((const char *)public_key)), RHN_OK);
  ck_assert_ptr_ne(grants = msprintf(end_session_token_pattern, CLIENT_ID, (long long)now, (long long)(now + EXPIRES_IN), ISSUER), NULL);

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_ISSUER, ISSUER,
                                                    I_OPT_CLIENT_ID, CLIENT_ID,
                                                    I_OPT_BACKCHANNEL_LOGOUT_SESSION_REQUIRED, 1,
                                                    I_OPT_ID_TOKEN_SID, ID_TOKEN_SID,
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(r_jwks_append_jwk(i_session.server_jwks, jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwt_init(&jwt), RHN_OK);
  ck_assert_int_eq(r_jwt_set_sign_alg(jwt, R_JWA_ALG_RS256), RHN_OK);
  ck_assert_int_eq(r_jwt_set_full_claims_json_str(jwt, grants), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "events", NULL), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "sid", ID_TOKEN_SID), RHN_OK);
  ck_assert_ptr_ne((jwt_str = r_jwt_serialize_signed(jwt, jwk, 0)), NULL);
  ck_assert_int_eq(I_ERROR_PARAM, i_verify_end_session_backchannel_token(&i_session, jwt_str));
  o_free(jwt_str);
  r_jwt_free(jwt);
  i_clean_session(&i_session);

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_ISSUER, ISSUER,
                                                    I_OPT_CLIENT_ID, CLIENT_ID,
                                                    I_OPT_BACKCHANNEL_LOGOUT_SESSION_REQUIRED, 1,
                                                    I_OPT_ID_TOKEN_SID, ID_TOKEN_SID,
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(r_jwks_append_jwk(i_session.server_jwks, jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwt_init(&jwt), RHN_OK);
  ck_assert_int_eq(r_jwt_set_sign_alg(jwt, R_JWA_ALG_RS256), RHN_OK);
  ck_assert_int_eq(r_jwt_set_full_claims_json_str(jwt, grants), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "events", "error"), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "sid", ID_TOKEN_SID), RHN_OK);
  ck_assert_ptr_ne((jwt_str = r_jwt_serialize_signed(jwt, jwk, 0)), NULL);
  ck_assert_int_eq(I_ERROR_PARAM, i_verify_end_session_backchannel_token(&i_session, jwt_str));
  o_free(jwt_str);
  r_jwt_free(jwt);
  i_clean_session(&i_session);

  o_free(grants);
  r_jwk_free(jwk);
  r_jwk_free(jwk_sign);
}
END_TEST

START_TEST(test_iddawc_session_verify_token_invalid_sid)
{
  struct _i_session i_session;
  jwt_t * jwt;
  jwk_t * jwk, * jwk_sign;
  char * grants = NULL, * jwt_str;
  time_t now;
  time(&now);
  ck_assert_int_eq(r_jwk_init(&jwk), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk, R_X509_TYPE_PRIVKEY, R_FORMAT_PEM, private_key, o_strlen((const char *)private_key)), RHN_OK);
  ck_assert_int_eq(r_jwk_init(&jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk_sign, R_X509_TYPE_PUBKEY, R_FORMAT_PEM, public_key, o_strlen((const char *)public_key)), RHN_OK);
  ck_assert_ptr_ne(grants = msprintf(end_session_token_pattern, CLIENT_ID, (long long)now, (long long)(now + EXPIRES_IN), ISSUER), NULL);

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_ISSUER, ISSUER,
                                                    I_OPT_CLIENT_ID, CLIENT_ID,
                                                    I_OPT_BACKCHANNEL_LOGOUT_SESSION_REQUIRED, 1,
                                                    I_OPT_ID_TOKEN_SID, "error",
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(r_jwks_append_jwk(i_session.server_jwks, jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwt_init(&jwt), RHN_OK);
  ck_assert_int_eq(r_jwt_set_sign_alg(jwt, R_JWA_ALG_RS256), RHN_OK);
  ck_assert_int_eq(r_jwt_set_full_claims_json_str(jwt, grants), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "sid", ID_TOKEN_SID), RHN_OK);
  ck_assert_ptr_ne((jwt_str = r_jwt_serialize_signed(jwt, jwk, 0)), NULL);
  ck_assert_int_eq(I_ERROR_PARAM, i_verify_end_session_backchannel_token(&i_session, jwt_str));
  o_free(jwt_str);
  r_jwt_free(jwt);
  i_clean_session(&i_session);

  o_free(grants);
  r_jwk_free(jwk);
  r_jwk_free(jwk_sign);
}
END_TEST

START_TEST(test_iddawc_session_verify_token_missing_sid_sub)
{
  struct _i_session i_session;
  jwt_t * jwt;
  jwk_t * jwk, * jwk_sign;
  char * grants = NULL, * jwt_str;
  time_t now;
  time(&now);
  ck_assert_int_eq(r_jwk_init(&jwk), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk, R_X509_TYPE_PRIVKEY, R_FORMAT_PEM, private_key, o_strlen((const char *)private_key)), RHN_OK);
  ck_assert_int_eq(r_jwk_init(&jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk_sign, R_X509_TYPE_PUBKEY, R_FORMAT_PEM, public_key, o_strlen((const char *)public_key)), RHN_OK);
  ck_assert_ptr_ne(grants = msprintf(end_session_token_pattern, CLIENT_ID, (long long)now, (long long)(now + EXPIRES_IN), ISSUER), NULL);

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_ISSUER, ISSUER,
                                                    I_OPT_CLIENT_ID, CLIENT_ID,
                                                    I_OPT_BACKCHANNEL_LOGOUT_SESSION_REQUIRED, 0,
                                                    I_OPT_ID_TOKEN_SID, ID_TOKEN_SID,
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(r_jwks_append_jwk(i_session.server_jwks, jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwt_init(&jwt), RHN_OK);
  ck_assert_int_eq(r_jwt_set_sign_alg(jwt, R_JWA_ALG_RS256), RHN_OK);
  ck_assert_int_eq(r_jwt_set_full_claims_json_str(jwt, grants), RHN_OK);
  ck_assert_ptr_ne((jwt_str = r_jwt_serialize_signed(jwt, jwk, 0)), NULL);
  ck_assert_int_eq(I_ERROR_PARAM, i_verify_end_session_backchannel_token(&i_session, jwt_str));
  o_free(jwt_str);
  r_jwt_free(jwt);
  i_clean_session(&i_session);

  o_free(grants);
  r_jwk_free(jwk);
  r_jwk_free(jwk_sign);
}
END_TEST

START_TEST(test_iddawc_session_verify_token_invalid_key)
{
  struct _i_session i_session;
  jwt_t * jwt;
  jwk_t * jwk, * jwk_sign;
  char * grants = NULL, * jwt_str;
  time_t now;
  time(&now);
  ck_assert_int_eq(r_jwk_init(&jwk), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_json_str(jwk, jwk_privkey_str_2), RHN_OK);
  ck_assert_int_eq(r_jwk_init(&jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwk_import_from_pem_der(jwk_sign, R_X509_TYPE_PUBKEY, R_FORMAT_PEM, public_key, o_strlen((const char *)public_key)), RHN_OK);
  ck_assert_ptr_ne(grants = msprintf(end_session_token_pattern, CLIENT_ID, (long long)now, (long long)(now + EXPIRES_IN), ISSUER), NULL);

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_ISSUER, ISSUER,
                                                    I_OPT_CLIENT_ID, CLIENT_ID,
                                                    I_OPT_BACKCHANNEL_LOGOUT_SESSION_REQUIRED, 1,
                                                    I_OPT_ID_TOKEN_SID, ID_TOKEN_SID,
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(r_jwks_append_jwk(i_session.server_jwks, jwk_sign), RHN_OK);
  ck_assert_int_eq(r_jwt_init(&jwt), RHN_OK);
  ck_assert_int_eq(r_jwt_set_sign_alg(jwt, R_JWA_ALG_RS256), RHN_OK);
  ck_assert_int_eq(r_jwt_set_full_claims_json_str(jwt, grants), RHN_OK);
  ck_assert_int_eq(r_jwt_set_claim_str_value(jwt, "sid", ID_TOKEN_SID), RHN_OK);
  ck_assert_ptr_ne((jwt_str = r_jwt_serialize_signed(jwt, jwk, 0)), NULL);
  ck_assert_int_eq(I_ERROR_PARAM, i_verify_end_session_backchannel_token(&i_session, jwt_str));
  o_free(jwt_str);
  r_jwt_free(jwt);
  i_clean_session(&i_session);

  o_free(grants);
  r_jwk_free(jwk);
  r_jwk_free(jwk_sign);
}
END_TEST

START_TEST(test_iddawc_session_close)
{
  struct _i_session i_session;

  ck_assert_int_eq(i_init_session(&i_session), I_OK);
  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_CODE, VALUE,
                                                    I_OPT_REFRESH_TOKEN, VALUE,
                                                    I_OPT_ACCESS_TOKEN, VALUE,
                                                    I_OPT_ID_TOKEN, VALUE,
                                                    I_OPT_NONCE, VALUE,
                                                    I_OPT_USERINFO, VALUE,
                                                    I_OPT_TOKEN_JTI, VALUE,
                                                    I_OPT_DEVICE_AUTH_CODE, VALUE,
                                                    I_OPT_DEVICE_AUTH_USER_CODE, VALUE,
                                                    I_OPT_DEVICE_AUTH_VERIFICATION_URI, VALUE,
                                                    I_OPT_DEVICE_AUTH_VERIFICATION_URI_COMPLETE, VALUE,
                                                    I_OPT_PKCE_CODE_VERIFIER, PKCE_CODE_VERIFIER,
                                                    I_OPT_CIBA_USER_CODE, VALUE,
                                                    I_OPT_CIBA_AUTH_REQ_ID, VALUE,
                                                    I_OPT_ID_TOKEN_SID, ID_TOKEN_SID,
                                                    I_OPT_NONE), I_OK);
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_CODE));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_REFRESH_TOKEN));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_ACCESS_TOKEN));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_ID_TOKEN));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_NONCE));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_USERINFO));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_TOKEN_JTI));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_CODE));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_USER_CODE));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_VERIFICATION_URI));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_VERIFICATION_URI_COMPLETE));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_PKCE_CODE_VERIFIER));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_CIBA_USER_CODE));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_CIBA_AUTH_REQ_ID));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_ID_TOKEN_SID));

  ck_assert_int_eq(i_close_session(&i_session, "error"), I_ERROR_PARAM);
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_CODE));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_REFRESH_TOKEN));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_ACCESS_TOKEN));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_ID_TOKEN));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_NONCE));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_USERINFO));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_TOKEN_JTI));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_CODE));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_USER_CODE));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_VERIFICATION_URI));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_VERIFICATION_URI_COMPLETE));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_PKCE_CODE_VERIFIER));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_CIBA_USER_CODE));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_CIBA_AUTH_REQ_ID));
  ck_assert_ptr_ne(NULL, i_get_str_parameter(&i_session, I_OPT_ID_TOKEN_SID));

  ck_assert_int_eq(i_close_session(&i_session, NULL), I_OK);
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_CODE));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_REFRESH_TOKEN));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_ACCESS_TOKEN));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_ID_TOKEN));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_NONCE));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_USERINFO));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_TOKEN_JTI));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_CODE));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_USER_CODE));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_VERIFICATION_URI));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_VERIFICATION_URI_COMPLETE));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_PKCE_CODE_VERIFIER));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_CIBA_USER_CODE));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_CIBA_AUTH_REQ_ID));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_ID_TOKEN_SID));

  ck_assert_int_eq(i_set_parameter_list(&i_session, I_OPT_CODE, VALUE,
                                                    I_OPT_REFRESH_TOKEN, VALUE,
                                                    I_OPT_ACCESS_TOKEN, VALUE,
                                                    I_OPT_ID_TOKEN, VALUE,
                                                    I_OPT_NONCE, VALUE,
                                                    I_OPT_USERINFO, VALUE,
                                                    I_OPT_TOKEN_JTI, VALUE,
                                                    I_OPT_DEVICE_AUTH_CODE, VALUE,
                                                    I_OPT_DEVICE_AUTH_USER_CODE, VALUE,
                                                    I_OPT_DEVICE_AUTH_VERIFICATION_URI, VALUE,
                                                    I_OPT_DEVICE_AUTH_VERIFICATION_URI_COMPLETE, VALUE,
                                                    I_OPT_PKCE_CODE_VERIFIER, PKCE_CODE_VERIFIER,
                                                    I_OPT_CIBA_USER_CODE, VALUE,
                                                    I_OPT_CIBA_AUTH_REQ_ID, VALUE,
                                                    I_OPT_ID_TOKEN_SID, ID_TOKEN_SID,
                                                    I_OPT_NONE), I_OK);
  ck_assert_int_eq(i_close_session(&i_session, ID_TOKEN_SID), I_OK);
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_CODE));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_REFRESH_TOKEN));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_ACCESS_TOKEN));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_ID_TOKEN));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_NONCE));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_USERINFO));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_TOKEN_JTI));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_CODE));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_USER_CODE));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_VERIFICATION_URI));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_DEVICE_AUTH_VERIFICATION_URI_COMPLETE));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_PKCE_CODE_VERIFIER));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_CIBA_USER_CODE));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_CIBA_AUTH_REQ_ID));
  ck_assert_ptr_eq(NULL, i_get_str_parameter(&i_session, I_OPT_ID_TOKEN_SID));

  i_clean_session(&i_session);
}
END_TEST

static Suite *iddawc_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Iddawc session requests tests");
  tc_core = tcase_create("test_iddawc_session");
  tcase_add_test(tc_core, test_iddawc_session_build_url);
  tcase_add_test(tc_core, test_iddawc_session_verify_token_ok);
  tcase_add_test(tc_core, test_iddawc_session_verify_token_invalid_iss);
  tcase_add_test(tc_core, test_iddawc_session_verify_token_invalid_aud);
  tcase_add_test(tc_core, test_iddawc_session_verify_token_invalid_iat);
  tcase_add_test(tc_core, test_iddawc_session_verify_token_invalid_jti);
  tcase_add_test(tc_core, test_iddawc_session_verify_token_invalid_events);
  tcase_add_test(tc_core, test_iddawc_session_verify_token_invalid_sid);
  tcase_add_test(tc_core, test_iddawc_session_verify_token_missing_sid_sub);
  tcase_add_test(tc_core, test_iddawc_session_verify_token_invalid_key);
  tcase_add_test(tc_core, test_iddawc_session_close);
  tcase_set_timeout(tc_core, 30);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(int argc, char *argv[])
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  //y_init_logs("Iddawc", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Iddawc Session tests");
  i_global_init();
  s = iddawc_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  
  i_global_close();
  //y_close_logs();
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
