package io.github.jaffe2718.qwen3asr4j;

import java.util.function.BiConsumer;

public class Qwen3ASR implements AutoCloseable {

    private int ctxId = -1;    // -1 means not loaded

    public Qwen3ASR(String modelPath) {
        this.load(modelPath);
    }

    @Override
    public void close() {
        if (ctxId != -1) {
            this.free();
            ctxId = -1;
        }
    }

    public TranscribeResult transcribe(float[] samples, TranscribeParams params) {
        return this.transcribe(samples, samples.length, params);
    }

    /**
     * Set the progress callback function
     * (tokens_generated, max_tokens) -> void
     * @param callback the progress callback
     */
    public native void setProgressCallback(BiConsumer<Integer, Integer> callback);

    /**
     * Check if the model is loaded
     * @return true if the model is loaded, false otherwise
     */
    public native boolean isLoaded();

    /**
     * Load the model & set the context ID
     * @param modelPath the path to the model file
     */
    private native void load(String modelPath);

    /**
     * Free the context & release the model
     */
    private native void free();

    /**
     * Transcribe the audio samples
     * @param samples the audio samples
     * @param nSamples the number of samples
     * @param params the transcribe parameters
     * @return the transcribe result
     */
    private native TranscribeResult transcribe(float[] samples, int nSamples, TranscribeParams params);

}