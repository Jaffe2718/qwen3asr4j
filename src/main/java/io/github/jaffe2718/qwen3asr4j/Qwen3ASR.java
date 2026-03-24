package io.github.jaffe2718.qwen3asr4j;

import io.github.jaffe2718.qwen3asr4j.param.TranscribeParams;
import io.github.jaffe2718.qwen3asr4j.result.TranscribeResult;
import org.jetbrains.annotations.Nullable;
import org.slf4j.Logger;

import java.io.FileNotFoundException;
import java.util.Map;

/**
 * The Qwen3ASR class.
 */
public class Qwen3ASR extends GGUFModelWrapper {

    /**
     * The constructor.
     * @param modelPath the path to the model file
     * @param logger the logger to use, null to disable logging
     * @throws FileNotFoundException if the model file does not exist or is invalid
     */
    public Qwen3ASR(String modelPath, @Nullable Logger logger) throws FileNotFoundException {
        super(modelPath, logger);
    }

    /**
     * Transcribe the audio samples
     * @param samples the audio samples
     * @param params the transcribe parameters
     * @return the transcribe result
     * @see TranscribeParams
     */
    public TranscribeResult transcribe(float[] samples, TranscribeParams params) {
        return this.transcribe(samples, samples.length, params);
    }

    /**
     * Transcribe the audio file
     * @param audioPath the path to the audio file
     * @param params the transcribe parameters
     * @return the transcribe result
     * @see TranscribeParams
     */
    public TranscribeResult transcribe(String audioPath, TranscribeParams params) throws FileNotFoundException {
        return this.transcribeFile(audioPath, params);
    }

    /**
     * Set the progress callback function
     * (tokens_generated, max_tokens) -> void
     * @param callback the progress callback
     */
    public native void setProgressCallback(ProgressCallback callback);

    /**
     * Check if the model is loaded
     * @return true if the model is loaded, false otherwise
     */
    public native boolean isLoaded();

     /**
     * Get the last error message
     * @return the last error message
     */
    public native String getError() throws NullPointerException;

    /**
     * Get the model config
     * @return the model config, null if not loaded
     * <ol>
     *     <li>vocab_size int</li>
     *     <li>hidden_size int</li>
     *     <li>n_decoder_layers int</li>
     *     <li>n_attention_heads int</li>
     *     <li>n_key_value_heads int</li>
     *     <li>intermediate_size int</li>
     *     <li>head_dim int</li>
     *     <li>rms_norm_eps float</li>
     *     <li>rope_theta float</li>
     *     <li>pad_token_id int</li>
     *     <li>eos_token_id int</li>
     *     <li>audio_start_token_id int</li>
     *     <li>audio_end_token_id int</li>
     *     <li>audio_pad_token_id int</li>
     * </ol>
     */
    @Nullable
    public native Map<String, Number> getConfig();

    /**
     * Load the model and set the context ID
     * @param modelPath the path to the model file
     */
    @Override
    protected native void load(String modelPath) throws FileNotFoundException;

    /**
     * Free the context and release the model
     */
    @Override
    protected native void free();

    /**
     * Transcribe the audio samples
     * @param samples the audio samples
     * @param nSamples the number of samples
     * @param params the transcribe parameters
     * @return the transcribe result
     */
    private native TranscribeResult transcribe(float[] samples, int nSamples, TranscribeParams params) throws NullPointerException;

    /**
     * Transcribe the audio file
     * @param audioPath the path to the audio file
     * @param params the transcribe parameters
     * @return the transcribe result
     */
    private native TranscribeResult transcribeFile(String audioPath, TranscribeParams params) throws FileNotFoundException, NullPointerException;
}