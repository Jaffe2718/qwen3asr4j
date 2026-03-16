package io.github.jaffe2718.qwen3asr4j.result;

/**
 * The result of transcribing audio.
 *
 * @param language the language of the audio
 * @param text the transcribed text
 * @param success whether the transcribing is successful
 * @param errorMsg the error message if the transcribing is unsuccessful
 * @param tLoadMs the time cost of loading the audio in milliseconds
 * @param tMelMs the time cost of computing the mel spectrogram in milliseconds
 * @param tEncodeMs the time cost of encoding the mel spectrogram in milliseconds
 * @param tDecodeMs the time cost of decoding the encoded sequence in milliseconds
 * @param tTotalMs the total time cost of transcribing the audio in milliseconds
 */
public record TranscribeResult(
        String language,
        String text,
        boolean success,
        String errorMsg,
        long tLoadMs,
        long tMelMs,
        long tEncodeMs,
        long tDecodeMs,
        long tTotalMs
) {}
