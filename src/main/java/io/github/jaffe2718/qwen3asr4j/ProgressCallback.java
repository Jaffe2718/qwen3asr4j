package io.github.jaffe2718.qwen3asr4j;

import java.util.function.BiConsumer;

@FunctionalInterface
public interface ProgressCallback {
    void accept(int tokensGenerated, int maxTokens);
}
