/*
 * Test suite for libwebauth token manipulation functions.
 *
 * Written by Roland Schemers
 * Updated for current TAP library support by Russ Allbery
 * Copyright 2002, 2003, 2006, 2009, 2012
 *     The Board of Trustees of the Leland Stanford Junior University
 *
 * See LICENSE for licensing terms.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tests/tap/basic.h>
#include <webauth.h>
#include <webauth/basic.h>
#include <webauth/tokens.h>

#define BUFSIZE 4096
#define MAX_ATTRS 128


int
main(void)
{
    struct webauth_context *ctx;
    WEBAUTH_KEY *key;
    WEBAUTH_KEYRING *ring, *ring2;
    char key_material[WA_AES_128];
    WEBAUTH_ATTR_LIST *ain, *aout;
    size_t rlen, plen, len, i;
    int s;
    char *token;
    time_t curr;

    plan(81);

    if (webauth_context_init(&ctx, NULL) != WA_ERR_NONE)
        bail("cannot initialize WebAuth context");

    time(&curr);
    ain = webauth_attr_list_new(32);
    webauth_attr_list_add_str(ain, WA_TK_TOKEN_TYPE, "id", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT_AUTH, "webkdc", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT, "krb5:schemers", 0,
                              WA_F_NONE);
    webauth_attr_list_add_time(ain, WA_TK_EXPIRATION_TIME, curr + 3600,
                               WA_F_NONE);

    s = webauth_random_key(key_material, WA_AES_128);
    is_int(WA_ERR_NONE, s, "Getting random key material succeeds");
    key = webauth_key_create(WA_AES_KEY, key_material, WA_AES_128);
    ok(key != NULL, "Creating a key succeeds");
    ring = webauth_keyring_new(32);
    ok(ring != NULL, "Creating a key ring succeeds");
    time(&curr);
    s = webauth_keyring_add(ring, curr, curr, key);
    is_int(WA_ERR_NONE, s, "Adding the key to the keyring succeeds");
    webauth_key_free(key);

    rlen = webauth_token_encoded_length(ain, &plen);
    s = webauth_token_create(ctx, ain, 0, &token, &len, ring);
    is_int(WA_ERR_NONE, s, "Creating a token succeeds");
    is_int(rlen, len, "...and has the correct length");

    /* Now let's try to decode the token. */
    aout = NULL;
    s = webauth_token_parse(ctx, token, len, 0, ring, &aout);
    is_int(WA_ERR_NONE, s, "Parsing the token succeeds");
    is_int(ain->num_attrs, aout->num_attrs,
           "...and the attribute count is correct");
    for (i = 0; i < ain->num_attrs; i++) {
        is_string(ain->attrs[i].name, aout->attrs[i].name,
                  "...attribute name %lu is correct", (unsigned long) i);
        is_int(ain->attrs[i].length, aout->attrs[i].length,
               "...attribute length %lu is correct", (unsigned long) i);
        ok(memcmp(aout->attrs[i].value, ain->attrs[i].value,
                  ain->attrs[i].length) == 0,
           "...attribute value %lu is correct", (unsigned long) i);
    }
    webauth_attr_list_free(aout);

    /*
     * Now let's encrypt a token in a key not on the ring and make sure it
     * doesn't decrypt
     */
    s = webauth_random_key(key_material, WA_AES_128);
    is_int(WA_ERR_NONE, s, "Getting random key material succeeds");
    key = webauth_key_create(WA_AES_KEY, key_material, WA_AES_128);
    ok(key != NULL, "Creating a key succeeds");
    ring2 = webauth_keyring_new(32);
    ok(ring2 != NULL, "Creating a key ring succeeds");
    time(&curr);
    s = webauth_keyring_add(ring2, curr, curr, key);
    is_int(WA_ERR_NONE, s, "Adding the key to the keyring succeeds");
    webauth_key_free(key);
    rlen = webauth_token_encoded_length(ain, &plen);
    s = webauth_token_create(ctx, ain, 0, &token, &len, ring2);
    is_int(WA_ERR_NONE, s, "Creating a token succeeds");
    is_int(rlen, len, "...and has the correct length");
    aout = NULL;
    s = webauth_token_parse(ctx, token, len, 0, ring, &aout);
    ok(s != WA_ERR_NONE, "Decoding with the wrong key correctly fails");
    webauth_attr_list_free(ain);
    webauth_keyring_free(ring);
    webauth_keyring_free(ring2);

    /* Now let's try the {create,parse}_with_key versions. */
    ain = webauth_attr_list_new(32);
    webauth_attr_list_add_str(ain, WA_TK_TOKEN_TYPE, "id", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT_AUTH, "webkdc", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT, "krb5:schemers", 0,
                              WA_F_NONE);
    webauth_attr_list_add_time(ain, WA_TK_EXPIRATION_TIME, curr + 3600,
                               WA_F_NONE);
    s = webauth_random_key(key_material, WA_AES_128);
    is_int(WA_ERR_NONE, s, "Getting random key material succeeds");
    key = webauth_key_create(WA_AES_KEY, key_material, WA_AES_128);
    ok(key != NULL, "Creating a key succeeds");
    rlen = webauth_token_encoded_length(ain, &plen);
    s = webauth_token_create_with_key(ctx, ain, 0, &token, &len, key);
    is_int(WA_ERR_NONE, s, "Creating a token with a key succeeds");
    is_int(rlen, len, "...and has the correct length");
    aout = NULL;
    s = webauth_token_parse_with_key(ctx, token, len, 0, key, &aout);
    is_int(WA_ERR_NONE, s, "Parsing the token succeeds");
    is_int(ain->num_attrs, aout->num_attrs,
           "...and the attribute count is correct");
    for (i = 0; i < ain->num_attrs; i++) {
        is_string(ain->attrs[i].name, aout->attrs[i].name,
                  "...attribute name %lu is correct", (unsigned long) i);
        is_int(ain->attrs[i].length, aout->attrs[i].length,
               "...attribute length %lu is correct", (unsigned long) i);
        ok(memcmp(aout->attrs[i].value, ain->attrs[i].value,
                  ain->attrs[i].length) == 0,
           "...attribute value %lu is correct", (unsigned long) i);
    }
    webauth_attr_list_free(aout);
    webauth_attr_list_free(ain);
    webauth_key_free(key);

    /* Let's try to parse an expired token. */
    ain = webauth_attr_list_new(32);
    webauth_attr_list_add_str(ain, WA_TK_TOKEN_TYPE, "id", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT_AUTH, "webkdc", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT, "krb5:schemers", 0,
                              WA_F_NONE);
    webauth_attr_list_add_time(ain, WA_TK_EXPIRATION_TIME, curr - 3600,
                               WA_F_NONE);
    s = webauth_random_key(key_material, WA_AES_128);
    is_int(WA_ERR_NONE, s, "Getting random key material succeeds");
    key = webauth_key_create(WA_AES_KEY, key_material, WA_AES_128);
    ok(key != NULL, "Creating a key succeeds");
    rlen = webauth_token_encoded_length(ain, &plen);
    s = webauth_token_create_with_key(ctx, ain, 0, &token, &len, key);
    is_int(WA_ERR_NONE, s, "Creating a token with a key succeeds");
    is_int(rlen, len, "...and has the correct length");
    aout = NULL;
    s = webauth_token_parse_with_key(ctx, token, len, 0, key, &aout);
    is_int(WA_ERR_TOKEN_EXPIRED, s,
           "Parsing an expired token produces the correct error");
    is_int(ain->num_attrs, aout->num_attrs,
           "...and the attribute count is correct");
    for (i = 0; i < ain->num_attrs; i++) {
        is_string(ain->attrs[i].name, aout->attrs[i].name,
                  "...attribute name %lu is correct", (unsigned long) i);
        is_int(ain->attrs[i].length, aout->attrs[i].length,
               "...attribute length %lu is correct", (unsigned long) i);
        ok(memcmp(aout->attrs[i].value, ain->attrs[i].value,
                  ain->attrs[i].length) == 0,
           "...attribute value %lu is correct", (unsigned long) i);
    }
    webauth_attr_list_free(aout);
    webauth_attr_list_free(ain);
    webauth_key_free(key);

    /* let's try to parse a stale token. */
    ain = webauth_attr_list_new(32);
    webauth_attr_list_add_str(ain, WA_TK_TOKEN_TYPE, "id", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT_AUTH, "webkdc", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT, "krb5:schemers", 0,
                              WA_F_NONE);
    webauth_attr_list_add_time(ain, WA_TK_CREATION_TIME, curr - 3600,
                               WA_F_NONE);
    s = webauth_random_key(key_material, WA_AES_128);
    is_int(WA_ERR_NONE, s, "Getting random key material succeeds");
    key = webauth_key_create(WA_AES_KEY, key_material, WA_AES_128);
    ok(key != NULL, "Creating a key succeeds");
    rlen = webauth_token_encoded_length(ain, &plen);
    s = webauth_token_create_with_key(ctx, ain, 0, &token, &len, key);
    is_int(WA_ERR_NONE, s, "Creating a token with a key succeeds");
    is_int(rlen, len, "...and has the correct length");
    aout = NULL;
    s = webauth_token_parse_with_key(ctx, token, len, 300, key, &aout);
    is_int(WA_ERR_TOKEN_STALE, s,
           "Parsing a stale token produces the correct error");
    is_int(ain->num_attrs, aout->num_attrs,
           "...and the attribute count is correct");
    for (i = 0; i < ain->num_attrs; i++) {
        is_string(ain->attrs[i].name, aout->attrs[i].name,
                  "...attribute name %lu is correct", (unsigned long) i);
        is_int(ain->attrs[i].length, aout->attrs[i].length,
               "...attribute length %lu is correct", (unsigned long) i);
        ok(memcmp(aout->attrs[i].value, ain->attrs[i].value,
                  ain->attrs[i].length) == 0,
           "...attribute value %lu is correct", (unsigned long) i);
    }
    webauth_attr_list_free(aout);
    webauth_attr_list_free(ain);
    webauth_key_free(key);

    return 0;
}
