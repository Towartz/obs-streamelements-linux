#pragma once

#include <obs-module.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct obs_source_info audio_wrapper_source;
void register_audio_wrapper_source(void);

#ifdef __cplusplus
}
#endif
