package io.github.jaffe2718.qwen3asr4j.result;

import org.jetbrains.annotations.NotNull;

/**
 * The aligned word record.
 * @param word the word
 * @param start the start time in seconds
 * @param end the end time in seconds
 */
public record AlignedWord(
        @NotNull String word,
        float start,
        float end
) {}
