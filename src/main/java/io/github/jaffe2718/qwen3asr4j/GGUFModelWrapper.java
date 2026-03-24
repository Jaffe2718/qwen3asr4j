package io.github.jaffe2718.qwen3asr4j;

import org.jetbrains.annotations.Nullable;
import org.slf4j.Logger;

import java.io.FileNotFoundException;

/**
 * The base class for GGUF model wrappers.
 */
@SuppressWarnings("unused")
public abstract class GGUFModelWrapper implements AutoCloseable {

    /**
     * Set the global logger for logging
     * @param logger the logger to use, null to disable logging
     */
    public static native void setGGMLGlobalLogger(Logger logger);

    /**
     * The logger to use for logging, null to disable logging
     */
    protected @Nullable final Logger logger;

    /**
     * The context ID of the model, will be modified in the native load method, -1 means not loaded
     * @see #load(String)
     */
    @SuppressWarnings("FieldMayBeFinal")
    private int ctxId = -1;

    /**
     * The constructor.
     * @param modelPath the path to the model file
     * @param logger the logger to use, null to disable logging
     * @throws FileNotFoundException if the model file does not exist or is invalid
     */
    protected GGUFModelWrapper(String modelPath, @Nullable Logger logger) throws FileNotFoundException {
        this.load(modelPath);
        this.logger = logger;
    }

    /**
     * Load the model and set the context ID
     * @param modelPath the path to the model file
     * @throws FileNotFoundException if the model file does not exist or is invalid
     */
    protected abstract void load(String modelPath) throws FileNotFoundException;

    /**
     * Free the context and release the model
     */
    protected abstract void free();

    /**
     * Log an info message, called in the native load method
     * @param msg the message to log
     */
    private void info(String msg) {
        if (this.logger != null) {
            this.logger.info(msg);
        }
    }

    @Override
    public void close() {
        this.free();
    }
}

