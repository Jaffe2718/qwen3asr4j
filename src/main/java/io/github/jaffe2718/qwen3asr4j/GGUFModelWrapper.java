package io.github.jaffe2718.qwen3asr4j;

import org.jetbrains.annotations.Nullable;
import org.slf4j.Logger;

import java.io.FileNotFoundException;

@SuppressWarnings("unused")
public abstract class GGUFModelWrapper implements AutoCloseable {

    protected @Nullable final Logger logger;

    /**
     * The context ID of the model, will be modified in the native load method, -1 means not loaded
     * @see #load(String)
     */
    @SuppressWarnings("FieldMayBeFinal")
    private int ctxId = -1;

    protected GGUFModelWrapper(String modelPath, @Nullable Logger logger) throws FileNotFoundException {
        this.load(modelPath);
        this.logger = logger;
    }

    protected abstract void load(String modelPath) throws FileNotFoundException;
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

