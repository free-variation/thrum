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

	printf("Buffer size range: %lu to %lu frames\n", min_buffer, max_buffer);
	
	snd_pcm_hw_params_free(hw_params);

	snd_pcm_close(pcm_handle);
	printf("Closed device %s\n", device_name);

	return 0;
}

void list_device_types() {
	const char *device_types[] = {"pcm", "ctl", "rawmidi", "hwdep", "seq", "timer"};
	int num_types = 6;

	for (int i = 0; i < num_types; i++) {
		void **hints;
		int count = 0;

		int err = snd_device_name_hint(-1, device_types[i], &hints);

		if (err >= 0) {
			printf("Defice type %s:\n", device_types[i]);

			void **hint = hints;
			while (*hint != NULL) {
				char *name = snd_device_name_get_hint(*hint, "NAME");
				if (name) {
					printf("\tDevice: %s\n", name);

					char *desc = snd_device_name_get_hint(*hint, "DESC");
					if (desc) {
						printf("\tDescription: %s\n", desc);
						free(desc);
					}

					if (strcmp(device_types[i], "pcm") == 0) {
						char *ioid = snd_device_name_get_hint(*hint, "IOID");
						if (ioid) {
							printf("\tI/O: %s\n", ioid);
							free(ioid);
						} else {
							printf("	I/O: Playback and Capture\n");
						}
					}

					free(name);
				}

				hint++;
				count++;
			}

			printf("\tTotal: %d devices\n\n", count);

			snd_device_name_free_hint(hints);
		} else {
			printf("Device type %s: Error %s\n\n", device_types[i], snd_strerror(err));
		}
	}
}

int initialize_device(const char*device_name, snd_pcm_t **pcm_handle) {
	int err;
	snd_pcm_hw_params_t *hw_params;

	if ((err = snd_pcm_open(pcm_handle, device_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "Cannot open audio device %s: %s\n", device_name, snd_strerror(err));
		goto error;
	}

	printf("Initializing device %s, handle: %p\n", device_name, *pcm_handle);

	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr, "Cannot allocate hardware parameter structure: %s\n", snd_strerror(err));
		goto error;
	}

	if ((err = snd_pcm_hw_params_any(*pcm_handle, hw_params)) < 0) {
	 	fprintf(stderr, "Cannot initialize hardware parameter structure: %s\n", snd_strerror(err));
	 	goto error;
	}

	err = snd_pcm_hw_params_set_channels(*pcm_handle, hw_params, 1);
	if (err < 0) {
		fprintf(stderr, "Cannot set channel count to mono: %s\n", snd_strerror(err));
		goto error;
	}

	err = snd_pcm_hw_params_set_access(*pcm_handle, hw_params, SND_PCM_ACCESS_RW_NONINTERLEAVED);
	if (err < 0) {
		fprintf(stderr, "Cannot set non-interleaved access: %s\n", snd_strerror(err));
		goto error;
	}

	const unsigned int sampling_rate = 48000;
	unsigned int rate = sampling_rate;
	int dir = 0;

	err = snd_pcm_hw_params_set_rate_near(*pcm_handle, hw_params, &rate, &dir);
	if (err < 0) {
		fprintf(stderr, "Cannot set sample rate access: %s\n", snd_strerror(err));
		goto error;
	}

	if (rate != sampling_rate) {
		printf("\tNote: Actual rate set to %u Hz (requested %u)\n", rate, sampling_rate);

	}

	err = snd_pcm_hw_params_set_format(*pcm_handle, hw_params, SND_PCM_FORMAT_FLOAT_LE);
	if (err < 0) {
		fprintf(stderr, "Cannot set floating-point format: %s\n", snd_strerror(err));
		printf("\tTrying 32-bit integer ...\n");

		err = snd_pcm_hw_params_set_format(*pcm_handle, hw_params, SND_PCM_FORMAT_S32_LE);
		if (err < 0) {
			fprintf(stderr, "Cannot set 32-bit integer format: %s\n", snd_strerror(err));
			goto error;
		}
	}

	snd_pcm_uframes_t period_size = 256;
	
	err = snd_pcm_hw_params_set_period_size_near(*pcm_handle, hw_params, &period_size, &dir);
	if (err < 0) {
		fprintf(stderr, "Cannot set period size: %s\n", snd_strerror(err));
		goto error;
	}

	printf("Period size set to %lu frames\n", period_size);

	unsigned int periods = 3;
	err = snd_pcm_hw_params_set_periods_near(*pcm_handle, hw_params, &periods, &dir);
	if (err < 0) {
		fprintf(stderr, "Cannot set number of periods: %s\n", snd_strerror(err));
		goto error;
	}

	// set all the parameters
	err = snd_pcm_hw_params(*pcm_handle, hw_params);
	if (err < 0) {
		fprintf(stderr, "Cannot set parameters: %s\n", snd_strerror(err));
		goto error;
	}

	snd_pcm_hw_params_free(hw_params);
	hw_params = NULL;

	err = snd_pcm_prepare(*pcm_handle);
	if (err < 0) {
		fprintf(stderr, "Cannot prepare PCM device: %s\n", snd_strerror(err));
		goto error;
	}

	return 0;

error:
	if (hw_params) snd_pcm_hw_params_free(hw_params);
	if (*pcm_handle) snd_pcm_close(*pcm_handle);
	return err;
}

int main(int argc, char *argv[]) {
	snd_pcm_t *pcm_handle;
	const char *use_device = "default";
	int err;

	list_device_types();

	if (argc > 1) {
		use_device = argv[1];
	}

	device_capabilities(use_device);
	
	err = initialize_device(use_device, &pcm_handle);

	if (!err) {
		snd_pcm_close(pcm_handle);
		printf("Closed device %s\n", use_device);
	}
}
