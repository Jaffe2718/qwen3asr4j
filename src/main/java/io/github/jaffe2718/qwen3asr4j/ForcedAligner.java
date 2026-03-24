package io.github.jaffe2718.qwen3asr4j;

import io.github.jaffe2718.qwen3asr4j.result.AlignmentResult;
import org.jetbrains.annotations.Nullable;
import org.slf4j.Logger;

import java.io.FileNotFoundException;
import java.util.Map;

/**
 * The forced aligner class.
 */
public class ForcedAligner extends GGUFModelWrapper {

    /**
     * The constructor.
     * @param modelPath the path to the model file
     * @param logger the logger to use, null to disable logging
     * @throws FileNotFoundException if the model file is not found
     */
    public ForcedAligner(String modelPath, @Nullable Logger logger) throws FileNotFoundException {
        super(modelPath, logger);
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
        return align(samples, text, "");
    }

    /**
     * Align the audio file with the text
     * @param audioPath the path to the audio file
     * @param text the text to align
     * @param language the language of the text (default: "")
     * @return the alignment result
     * @see AlignmentResult
     * @throws FileNotFoundException if the audio file does not exist
     * @throws NullPointerException if the ForcedAligner is not loaded
     */
    public AlignmentResult align(String audioPath, String text, String language) throws FileNotFoundException, NullPointerException {
        return alignFile(audioPath, text, language);
    }

    /**
     * Align the audio file with the text
     * @param audioPath the path to the audio file
     * @param text the text to align
     * @return the alignment result
     * @see AlignmentResult
     * @throws FileNotFoundException if the audio file does not exist
     * @throws NullPointerException if the ForcedAligner is not loaded
     */
    public AlignmentResult align(String audioPath, String text) throws FileNotFoundException, NullPointerException {
        return align(audioPath, text, "");
    }

    /**
     * Get the last error message
     * @return the last error message
     * @throws NullPointerException if the ForcedAligner is not loaded
     */
    public native String getError() throws NullPointerException;

    /**
     * Check if the ForcedAligner is loaded
     * @return true if loaded, false otherwise
     */
    public native boolean isLoaded();

    /**
     * Get the hyperparameters of the model
     * @return the hyperparameters, null if not loaded
     */
    @Nullable
    public native Map<String, Number> getHparams();

    /**
     * Load the model and set the context ID
     * @param modelPath the path to the model file
     * @throws FileNotFoundException if the model file does not exist or is invalid
     */
    @Override
    protected native void load(String modelPath) throws FileNotFoundException;

    /**
     * Free the context ID
     */
    @Override
    protected native void free();

    /**
     * Align the audio samples with the text
     * @param samples the audio samples
     * @param n_samples the number of samples
     * @param text the text to align
     * @param language the language of the text (default: "")
     * @return the alignment result
     */
    private native AlignmentResult align(float[] samples, int n_samples, String text, String language) throws NullPointerException;

    /**
     * Align the audio file with the text
     * @param audioPath the path to the audio file
     * @param text the text to align
     * @param language the language of the text (default: "")
     * @return the alignment result
     */
    private native AlignmentResult alignFile(String audioPath, String text, String language) throws FileNotFoundException, NullPointerException;
}
