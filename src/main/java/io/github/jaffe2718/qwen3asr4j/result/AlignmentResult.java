package io.github.jaffe2718.qwen3asr4j.result;

/**
 * The alignment result record.
 * @param words the aligned words
 * @param success whether the alignment is successful
 * @param errorMsg the error message if the alignment fails
 * @param tMelMs the time cost of mel spectrogram generation in milliseconds
 * @param tEncodeMs the time cost of encoding in milliseconds
 * @param tDecodeMs the time cost of decoding in milliseconds
 * @param tTotalMs the total time cost in milliseconds
 */
public record AlignmentResult(
        AlignedWord[] words,
        boolean success,
        String errorMsg,
        long tMelMs,
        long tEncodeMs,
        long tDecodeMs,
        long tTotalMs
) {}