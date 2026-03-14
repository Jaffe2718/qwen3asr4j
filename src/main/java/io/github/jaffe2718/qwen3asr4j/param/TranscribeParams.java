package io.github.jaffe2718.qwen3asr4j.param;

public record TranscribeParams(
    int maxTokens,
    String language,
    int nThreads,
    boolean printProgress,
    boolean printTiming
) {
    public TranscribeParams() {
        this(1024, "", 4, true, false);
    }
}
