#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

void sqrtSerial(int N,
                float initialGuess,
                float values[],
                float output[])
{

    static const float kThreshold = 0.00001f;

    for (int i=0; i<N; i++) {

        float x = values[i];
        float guess = initialGuess;

        float error = fabs(guess * guess * x - 1.f);

        while (error > kThreshold) {
            guess = (3.f * guess - x * guess * guess * guess) * 0.5f;
            error = fabs(guess * guess * x - 1.f);
        }

        output[i] = x * guess;
    }
}

void sqrt(int N,
    float initialGuess,
    float values[],
    float output[])
{
    static int VECTOR_SIZE = 8;

    __m256 const_05 = _mm256_set1_ps(.5f);
    __m256 const_3 = _mm256_set1_ps(3.f);
    __m256 const_1 = _mm256_set1_ps(1.f);

    __m256 kThreshold = _mm256_set1_ps(0.00001f);

    // 0111 .... 1111 부호만 끄는 bit
    __m256 absmask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7FFFFFFF));


    for (int i = 0; i<N; i+=VECTOR_SIZE){
        
        __m256 x = _mm256_loadu_ps(values+i);
        __m256 guess = _mm256_set1_ps(initialGuess);

        __m256 error = _mm256_and_ps((_mm256_sub_ps(_mm256_mul_ps(_mm256_mul_ps(guess, guess), x), const_1)), absmask);


        //__m256 ltmask = _mm256_castsi256_ps(_mm256_set1_epi32(0xFFFFFFFF));
        __m256 mask = _mm256_cmp_ps(kThreshold, error, _CMP_GT_OQ);
        int bitmask = _mm256_movemask_ps(mask);
        while(bitmask < 0xFF){
            
            guess = _mm256_mul_ps(_mm256_sub_ps(_mm256_mul_ps(const_3, guess), _mm256_mul_ps(x, _mm256_mul_ps(guess, _mm256_mul_ps(guess, guess)))), const_05);

            error = _mm256_and_ps((_mm256_sub_ps(_mm256_mul_ps(_mm256_mul_ps(guess, guess), x), const_1)), absmask);

            mask = _mm256_cmp_ps(kThreshold, error, _CMP_GT_OQ);
            bitmask = _mm256_movemask_ps(mask);
        }


    }

}
