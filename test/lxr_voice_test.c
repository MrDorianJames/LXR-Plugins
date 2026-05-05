/*
 * lxr_voice_test.c
 *
 * Standalone smoke test: triggers each of the four LXR voices in turn,
 * concatenates 0.5s of each into a single WAV file. Useful for verifying
 * the DSP layer builds correctly before tackling the JUCE plugin layer.
 *
 * Output: build/test/lxr_voices.wav
 *
 * Build:   cmake --build build --target lxr_voice_test
 * Run:     ./build/test/lxr_voices.wav
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "DrumVoice.h"
#include "Snare.h"
#include "CymbalVoice.h"
#include "HiHat.h"
#include "config.h"

extern void initRng(void);

#define SAMPLE_RATE       44100
#define DURATION_SAMPLES  22050   /* 0.5s per voice */
#define NUM_VOICES_TESTED 4

static void write_wav(const char *path, const int16_t *data, int n_samples)
{
    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); return; }
    int   byte_rate  = SAMPLE_RATE * 2;
    int   data_size  = n_samples * 2;
    int   chunk_size = 36 + data_size;
    int   subchunk1  = 16;
    int   sr         = SAMPLE_RATE;
    short fmt = 1, ch = 1, ba = 2, bps = 16;

    fwrite("RIFF", 1, 4, f);   fwrite(&chunk_size, 4, 1, f);
    fwrite("WAVEfmt ", 1, 8, f);
    fwrite(&subchunk1, 4, 1, f); fwrite(&fmt, 2, 1, f); fwrite(&ch, 2, 1, f);
    fwrite(&sr, 4, 1, f);        fwrite(&byte_rate, 4, 1, f);
    fwrite(&ba, 2, 1, f);        fwrite(&bps, 2, 1, f);
    fwrite("data", 1, 4, f);     fwrite(&data_size, 4, 1, f);
    fwrite(data, 2, n_samples, f);
    fclose(f);
}

static void render_drum  (int16_t *out, int n) {
    int blk, j; int16_t buf[OUTPUT_DMA_SIZE];
    Drum_trigger(0, 127, 60);
    for (blk = 0; blk * OUTPUT_DMA_SIZE < n; blk++) {
        calcDrumVoiceAsync(0);
        calcDrumVoiceSyncBlock(0, buf, OUTPUT_DMA_SIZE);
        for (j = 0; j < OUTPUT_DMA_SIZE; j++) {
            int idx = blk * OUTPUT_DMA_SIZE + j;
            if (idx < n) out[idx] = buf[j];
        }
    }
}
static void render_snare (int16_t *out, int n) {
    int blk, j; int16_t buf[OUTPUT_DMA_SIZE];
    Snare_trigger(127, 60);
    for (blk = 0; blk * OUTPUT_DMA_SIZE < n; blk++) {
        Snare_calcAsync();
        Snare_calcSyncBlock(buf, OUTPUT_DMA_SIZE);
        for (j = 0; j < OUTPUT_DMA_SIZE; j++) {
            int idx = blk * OUTPUT_DMA_SIZE + j;
            if (idx < n) out[idx] = buf[j];
        }
    }
}
static void render_cymbal (int16_t *out, int n) {
    int blk, j; int16_t buf[OUTPUT_DMA_SIZE];
    Cymbal_trigger(127, 60);
    for (blk = 0; blk * OUTPUT_DMA_SIZE < n; blk++) {
        Cymbal_calcAsync();
        Cymbal_calcSyncBlock(buf, OUTPUT_DMA_SIZE);
        for (j = 0; j < OUTPUT_DMA_SIZE; j++) {
            int idx = blk * OUTPUT_DMA_SIZE + j;
            if (idx < n) out[idx] = buf[j];
        }
    }
}
static void render_hihat (int16_t *out, int n) {
    int blk, j; int16_t buf[OUTPUT_DMA_SIZE];
    HiHat_trigger(127, 0, 60);  /* closed */
    for (blk = 0; blk * OUTPUT_DMA_SIZE < n; blk++) {
        HiHat_calcAsync();
        HiHat_calcSyncBlock(buf, OUTPUT_DMA_SIZE);
        for (j = 0; j < OUTPUT_DMA_SIZE; j++) {
            int idx = blk * OUTPUT_DMA_SIZE + j;
            if (idx < n) out[idx] = buf[j];
        }
    }
}

int main(void)
{
    int total = DURATION_SAMPLES * NUM_VOICES_TESTED;
    int16_t *out = calloc(total, sizeof(int16_t));
    if (!out) return 1;

    initRng();
    initDrumVoice();
    Snare_init();
    Cymbal_init();
    HiHat_init();

    render_drum   (out + 0 * DURATION_SAMPLES, DURATION_SAMPLES);
    render_snare  (out + 1 * DURATION_SAMPLES, DURATION_SAMPLES);
    render_cymbal (out + 2 * DURATION_SAMPLES, DURATION_SAMPLES);
    render_hihat  (out + 3 * DURATION_SAMPLES, DURATION_SAMPLES);

    write_wav("lxr_voices.wav", out, total);
    free(out);

    printf("Wrote lxr_voices.wav (%d samples, %.2fs)\n",
           total, (float)total / (float)SAMPLE_RATE);
    return 0;
}
