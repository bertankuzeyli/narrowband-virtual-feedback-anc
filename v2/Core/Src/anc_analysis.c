/*
 * anc_analysis.c
 */

#include "anc_analysis.h"
#include <stdio.h> // printf için

// --- DEĞİŞKEN TANIMLARI ---
arm_rfft_fast_instance_f32 fft_handler;
float32_t fft_input_buffer[FFT_LENGTH];
float32_t fft_output_buffer[FFT_LENGTH];

uint32_t fft_buffer_index = 0;
volatile uint8_t is_fft_ready = 0;

float32_t target_freq = 0.0f;
volatile float32_t debug_detected_freq = 0.0f;

// --- BAŞLATMA ---
void ANC_Analysis_Init(void)
{
    // FFT Motorunu Başlat (4096 nokta için)
    arm_rfft_fast_init_f32(&fft_handler, FFT_LENGTH);

    // Değişkenleri sıfırla
    fft_buffer_index = 0;
    is_fft_ready = 0;
    target_freq = 0.0f;
}

// --- İSTİHBARAT FONKSİYONU ---
void Find_Dominant_Freq(void)
{
    // 1. FFT Hesapla
    arm_rfft_fast_f32(&fft_handler, fft_input_buffer, fft_output_buffer, 0);

    // 2. Genlik (Magnitude) Hesapla
    // RAM tasarrufu için input buffer'ı eziyoruz (Artık işimiz bitti onla)
    float32_t* magnitudes = fft_input_buffer;

    arm_cmplx_mag_f32(fft_output_buffer, magnitudes, FFT_LENGTH / 2);

    // 3. En Yüksek Tepeyi Bul
    float32_t max_val;
    uint32_t max_idx;

    // DC Offset (0 Hz) ve çok düşük frekansları (ilk 5 bin) atla
    uint32_t start_index = 5;
    arm_max_f32(&magnitudes[start_index], (FFT_LENGTH / 2) - start_index, &max_val, &max_idx);

    // Gerçek indeksi bul
    max_idx += start_index;

    // 4. Frekansa Çevir
    float32_t detected_freq = (float32_t)max_idx * ((float32_t)SAMPLING_RATE / (float32_t)FFT_LENGTH);

    // 5. Hedefi Güncelle (Sessizlik Eşiği Kontrolü)
    if (max_val > SILENCE_THRESH)
    {
        target_freq = detected_freq;
        debug_detected_freq = detected_freq;
    }
    else
    {
        target_freq = 0.0f;
        debug_detected_freq = 0.0f;
    }
}
