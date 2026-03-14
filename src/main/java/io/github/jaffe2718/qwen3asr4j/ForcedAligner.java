package io.github.jaffe2718.qwen3asr4j;

import io.github.jaffe2718.qwen3asr4j.result.AlignmentResult;
import org.jetbrains.annotations.Nullable;

import java.util.Map;

public class ForcedAligner implements AutoCloseable {

    private int ctxId = -1;    // -1 means not loaded

    public ForcedAligner(String modelPath) {
        this.load(modelPath);
    }

    @Override
    public void close() {
        if (ctxId != -1) {
            this.free();
            ctxId = -1;
        }
    }

    /**
     * Align the audio samples with the text
     * @param samples the audio samples
     * @param text the text to align
     * @param language the language of the text (default: "")
     * @return the alignment result
     * @see AlignmentResult
     */
    public AlignmentResult align(float[] samples, String text, String language) {
        if (ctxId == -1) {
            throw new IllegalStateException("ForcedAligner is not loaded");
        }
        return align(samples, samples.length, text, language);
    }

     /**
     * Align the audio samples with the text
     * @param samples the audio samples
     * @param text the text to align
     * @return the alignment result
     * @see AlignmentResult
     */
    public AlignmentResult align(float[] samples, String text) {
        return align(samples, samples.length, text, "");
    }

    /**
     * Get the last error message
     * @return the last error message
     * @throws IllegalStateException if the ForcedAligner is not loaded
     */
    public native String getError() throws IllegalStateException;    // TODO test

    /**
     * Check if the ForcedAligner is loaded
     * @return true if loaded, false otherwise
     */
    public native boolean isLoaded();    // TODO test

    /**
     * Get the hyperparameters of the model
     * @return the hyperparameters, null if not loaded
     */
    @Nullable
    public native Map<String, Number> getHparams();    // TODO test

    /**
     * Load the model & set the context ID
     * @param modelPath the path to the model file
     */
    private native void load(String modelPath);     // TODO test

    /**
     * Free the context ID
     */
    private native void free();   // TODO test

     /**
     * Align the audio samples with the text
     * @param samples the audio samples
     * @param n_samples the number of samples
     * @param text the text to align
     * @param language the language of the text (default: "")
     * @return the alignment result
     */
    private native AlignmentResult align(float[] samples, int n_samples, String text, String language);   // TODO test

}
