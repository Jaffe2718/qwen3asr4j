package io.github.jaffe2718.qwen3asr4j.result;

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
