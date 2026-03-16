package io.github.jaffe2718.qwen3asr4j.param;

/**
 * The parameters for transcribing audio.
 *
 * @param maxTokens the maximum number of tokens to generate
 * @param language the language of the audio
 * @param nThreads the number of threads to use
 * @param printProgress whether to print progress
 * @param printTiming whether to print timing
 */
public record TranscribeParams(
    int maxTokens,
    String language,
    int nThreads,
    boolean printProgress,
    boolean printTiming
) {

    /**
     * The default constructor.
     */
    public TranscribeParams() {
        this(1024, "", 4, true, false);
    }
}
