package io.github.jaffe2718.qwen3asr4j;

@FunctionalInterface
@SuppressWarnings("unused")
public interface ProgressCallback {
    void accept(int tokensGenerated, int maxTokens);
}
