package io.github.jaffe2718.qwen3asr4j;

import org.jetbrains.annotations.NotNull;
import org.junit.jupiter.api.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.URISyntaxException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.ShortBuffer;

public class TestQwen3ASR {

    public static final Logger LOGGER = LoggerFactory.getLogger(TestQwen3ASR.class);

    private static float @NotNull [] toFloatArray(byte @NotNull [] data) {
        float[] result = new float[data.length / 2];
        ShortBuffer shortBuffer = ByteBuffer.wrap(data).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer();
        for (int i = 0; i < result.length; i++) {
            result[i] = Float.max(-1f, Float.min(((float) shortBuffer.get()) / (float) Short.MAX_VALUE, 1f));
        }
        return result;
    }

    @Test
    public void testBuiltinLib() throws IOException, URISyntaxException {
        NativeManager.loadLibrary(LOGGER);
        Qwen3ASR asr = new Qwen3ASR("qwen3-asr-0.6b-f16.gguf");
        LOGGER.info("isLoaded = {}", asr.isLoaded());

        FileInputStream wavInEnUs = new FileInputStream("samples/jfk.wav");
        wavInEnUs.skip(44);
        byte[] bytesEnUs = new byte[wavInEnUs.available()];
        wavInEnUs.read(bytesEnUs);
        wavInEnUs.close();
        LOGGER.info("transcribing {} samples in English from samples/jfk.wav", bytesEnUs.length / 2);
        TranscribeResult resultEnUs = asr.transcribe(toFloatArray(bytesEnUs), new TranscribeParams());
        LOGGER.info("resultEnUs.success() = {}", resultEnUs.success());
        LOGGER.info("resultEnUs.language() = {}", resultEnUs.language());
        LOGGER.info("resultEnUs.text() = {}", resultEnUs.text());
        LOGGER.info("resultEnUs.errorMsg() = {}", resultEnUs.errorMsg());
        LOGGER.info("resultEnUs.tLoadMs() = {}", resultEnUs.tLoadMs());
        LOGGER.info("resultEnUs.tMelMs() = {}", resultEnUs.tMelMs());
        LOGGER.info("resultEnUs.tEncodeMs() = {}", resultEnUs.tEncodeMs());
        LOGGER.info("resultEnUs.tDecodeMs() = {}", resultEnUs.tDecodeMs());
        LOGGER.info("resultEnUs.tTotalMs() = {}", resultEnUs.tTotalMs());
    }


    @Test
    public void testSampleZhCn() throws IOException, URISyntaxException {
        NativeManager.loadLibrary(LOGGER);
        Qwen3ASR asr = new Qwen3ASR("qwen3-asr-0.6b-f16.gguf");
        asr.setProgressCallback((tokensGenerated, maxTokens) -> LOGGER.info("tokensGenerated = {}, maxTokens = {}", tokensGenerated, maxTokens));
        LOGGER.info("isLoaded = {}", asr.isLoaded());

        FileInputStream wavInZhCn = new FileInputStream("samples/IC0936W0337.wav");
        wavInZhCn.skip(44);
        byte[] bytesZhCn = new byte[wavInZhCn.available()];
        wavInZhCn.read(bytesZhCn);
        wavInZhCn.close();
        LOGGER.info("transcribing {} samples in Chinese from samples/IC0936W0337.wav", bytesZhCn.length / 2);
        TranscribeResult resultZhCn = asr.transcribe(toFloatArray(bytesZhCn), new TranscribeParams());
        LOGGER.info("resultZhCn.success() = {}", resultZhCn.success());
        LOGGER.info("resultZhCn.language() = {}", resultZhCn.language());
        LOGGER.info("resultZhCn.text() = {}", resultZhCn.text());
        LOGGER.info("resultZhCn.errorMsg() = {}", resultZhCn.errorMsg());
        LOGGER.info("resultZhCn.tLoadMs() = {}", resultZhCn.tLoadMs());
        LOGGER.info("resultZhCn.tMelMs() = {}", resultZhCn.tMelMs());
        LOGGER.info("resultZhCn.tEncodeMs() = {}", resultZhCn.tEncodeMs());
        LOGGER.info("resultZhCn.tDecodeMs() = {}", resultZhCn.tDecodeMs());
        LOGGER.info("resultZhCn.tTotalMs() = {}", resultZhCn.tTotalMs());
    }
}
