#include <alsa/asoundlib.h>

int main() {
		snd_pcm_t *pcm_handle;
		const char *device_name = "default";
		int err;

		if ((err = snd_pcm_open(&pcm_handle, device_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
				fprintf(stderr, "Cannot open audio device %s: %s\n", device_name, snd_strerror(err));
				return err;
		}

		printf("Opened device %s, handle: %p\n", device_name, pcm_handle);

		snd_pcm_close(pcm_handle);

		printf("Closed device %s\n", device_name);
}
