package io.github.jaffe2718.qwen3asr4j;

/**
 * The progress callback interface.
 */
@FunctionalInterface
@SuppressWarnings("unused")
public interface ProgressCallback {

    /**
     * The progress callback function.
     * @param tokensGenerated the number of tokens generated
     * @param maxTokens the maximum number of tokens to generate
     */
    void accept(int tokensGenerated, int maxTokens);
}
