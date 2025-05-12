#include <alsa/asoundlib.h>

int device_capabilities(const char *device_name) {
	snd_pcm_t *pcm_handle;
	int err;

	if ((err = snd_pcm_open(&pcm_handle, device_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "Cannot open audio device %s: %s\n", device_name, snd_strerror(err));
		return err;
	}

	printf("Opened device %s, handle: %p\n", device_name, pcm_handle);


	snd_pcm_hw_params_t *hw_params;
	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr, "Cannot allocate hardware parameter structure: %s\n", snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_any(pcm_handle, hw_params)) < 0) {
		fprintf(stderr, "Cannot initialize hardware parameter structure: %s\n", snd_strerror(err));
		return err;
	}

	// formats
	int format;
	for (format = 0; format <= SND_PCM_FORMAT_LAST; format++) {
		if (snd_pcm_hw_params_test_format(pcm_handle, hw_params, format) == 0) {
			printf("%s\n", snd_pcm_format_name(format));
		}
	}

	// channels
	unsigned int min_channels, max_channels;

	err = snd_pcm_hw_params_get_channels_min(hw_params, &min_channels);
	if (err < 0) {
		fprintf(stderr, "Error getting minimum channels: %s\n", snd_strerror(err));
		return err;
	}

	err = snd_pcm_hw_params_get_channels_max(hw_params, &max_channels);
	if (err < 0) {
		fprintf(stderr, "Error getting maximum channels: %s\n", snd_strerror(err));
		return err;
	}

	printf("Channel range: %u to %u channels\n", min_channels, max_channels);

	unsigned int min_rate, max_rate;
	int dir = 0;

	err = snd_pcm_hw_params_get_rate_min(hw_params, &min_rate, &dir);
	if (err < 0) {
		fprintf(stderr, "Error getting minimum rate: %s\n", snd_strerror(err));
		return err;
	}

	err = snd_pcm_hw_params_get_rate_max(hw_params, &max_rate, &dir);
	if (err < 0) {
		fprintf(stderr, "Error getting maximum rate: %s\n", snd_strerror(err));
		return err;
	}

	printf("Sample rate range: %u Hz to %u Hz\n", min_rate, max_rate);

	// period size
	snd_pcm_uframes_t min_period, max_period;
	snd_pcm_hw_params_get_period_size_min(hw_params, &min_period, &dir);
	snd_pcm_hw_params_get_period_size_max(hw_params, &max_period, &dir);

	printf("Period size range: %lu to %lu frames\n", min_period, max_period);

	// buffer size
	snd_pcm_uframes_t min_buffer, max_buffer;
	snd_pcm_hw_params_get_buffer_size_min(hw_params, &min_buffer);
	snd_pcm_hw_params_get_buffer_size_max(hw_params, &max_buffer);

	printf("BUffer size range: %lu to %lu frames\n", min_buffer, max_buffer);
	
	snd_pcm_hw_params_free(hw_params);

	snd_pcm_close(pcm_handle);
	printf("Closed device %s\n", device_name);

	return 0;
}

int main() {
	device_capabilities("hw:0,0");
	return device_capabilities("default");
}
