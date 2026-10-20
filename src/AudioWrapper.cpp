#include "AudioWrapper.hpp"
#include <obs.h>
#include <util/bmem.h>

#define BLOG(level, msg, ...) blog(level, "[obs-streamelements] " msg, ##__VA_ARGS__)

struct audio_wrapper_data {
    obs_source_t *source;
    void *field_8;
    void *target;
    void *field_18;
};

static const char *audio_wrapper_get_name(void *unused)
{
    UNUSED_PARAMETER(unused);
    return "transition_audio_wrapper";
}

static void *audio_wrapper_create(obs_data_t *settings, obs_source_t *source)
{
    UNUSED_PARAMETER(settings);
    struct audio_wrapper_data *data = (struct audio_wrapper_data *)bmalloc(sizeof(struct audio_wrapper_data));
    if (!data) return nullptr;
    
    data->source = source;
    data->field_8 = nullptr;
    data->target = nullptr;
    data->field_18 = nullptr;
    return data;
}

static void audio_wrapper_destroy(void *data)
{
    if (data) {
        bfree(data);
    }
}

struct obs_source_info audio_wrapper_source = {
    .id = "streamelements_transition_audio_wrapper_source",
    .type = OBS_SOURCE_TYPE_INPUT,
    .output_flags = OBS_SOURCE_AUDIO | OBS_SOURCE_DO_NOT_DUPLICATE,
    .get_name = audio_wrapper_get_name,
    .create = audio_wrapper_create,
    .destroy = audio_wrapper_destroy,
};

void register_audio_wrapper_source(void)
{
    obs_register_source(&audio_wrapper_source);
    BLOG(LOG_INFO, "Registered 'streamelements_transition_audio_wrapper_source' source filter");
}
