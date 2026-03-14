package io.github.jaffe2718.qwen3asr4j.result;

import org.jetbrains.annotations.NotNull;

public record AlignedWord(
        @NotNull String word,
        float start,
        float end
) {}
