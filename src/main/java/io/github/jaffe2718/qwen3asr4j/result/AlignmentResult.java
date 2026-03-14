package io.github.jaffe2718.qwen3asr4j.result;

public record AlignmentResult(
        AlignedWord[] words,
        boolean success,
        String errorMsg,
        long tMelMs,
        long tEncodeMs,
        long tDecodeMs,
        long tTotalMs
) {}