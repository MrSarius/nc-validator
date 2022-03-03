#ifndef _SESSION_H_
#define _SESSION_H_

#include <moep/system.h>
#include <moep/types.h>
#include <moep/modules/moep80211.h>

#include <moepgf/moepgf.h>

#include <jsm.h>
#include "params.h"

struct session;
typedef struct session * session_t;

session_t
session_register(const struct params_session *params, const struct params_jsm *jsm, const u8 *sid);

session_t session_find(const u8 *sid);

int session_sid(u8 *sid, const u8 *hwaddr1, const u8 *hwaddr2);

void session_decoder_add(session_t s, moep_frame_t f);

int session_encoder_add(session_t s, moep_frame_t f);

int tx_decoded_frame(struct session *s);
int tx_encoded_frame(struct session *s, generation_t g);
int tx_ack_frame(struct session *s, generation_t g);

void session_commit_state(struct session *s, const struct generation_state *state);

void session_log_state();

void session_cleanup();

double session_redundancy(session_t s);

int session_remaining_space(const session_t s);

int session_min_remaining_space();

double session_ul_quality(session_t s);

#endif
