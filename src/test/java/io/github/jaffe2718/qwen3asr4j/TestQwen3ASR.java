package io.github.jaffe2718.qwen3asr4j;

import org.jetbrains.annotations.NotNull;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIf;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.URISyntaxException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.ShortBuffer;
import java.nio.file.Path;


/**
 * Test class for Qwen3ASR.
 * <pre>
 * env:
 *  - GGML_CUDA    (default: OFF)
 *  - GGML_VULKAN  (default: OFF)
 * </pre>
 * <code>gradlew test</code>
 */
public class TestQwen3ASR {

    private static final Logger LOGGER = LoggerFactory.getLogger(TestQwen3ASR.class);
    private static final String GGML_CUDA = System.getenv("GGML_CUDA") != null ? System.getenv("GGML_CUDA") : "OFF";
    private static final String GGML_VULKAN = System.getenv("GGML_VULKAN") != null ? System.getenv("GGML_VULKAN") : "OFF";

    private static float[] sampleEnUs;
    private static float[] sampleZhCn;


    public static boolean isCpuOnly() {
        return GGML_CUDA.equals("OFF") && GGML_VULKAN.equals("OFF");
    }
    public static boolean isVulkanEnabled() {
        return GGML_CUDA.equals("OFF") && GGML_VULKAN.equals("ON");
    }

    private static float @NotNull [] toFloatArray(byte @NotNull [] data) {
        float[] result = new float[data.length / 2];
        ShortBuffer shortBuffer = ByteBuffer.wrap(data).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer();
        for (int i = 0; i < result.length; i++) {
            result[i] = Float.max(-1f, Float.min(((float) shortBuffer.get()) / (float) Short.MAX_VALUE, 1f));
        }
        return result;
    }

    private static void testCallback(int tokensGenerated, int maxTokens) {
        LOGGER.info("tokensGenerated = {}, maxTokens = {}", tokensGenerated, maxTokens);
    }

    private static void printTranscribeResult(@NotNull TranscribeResult result) {
        LOGGER.info("result.success() = {}", result.success());
        LOGGER.info("result.language() = {}", result.language());
        LOGGER.info("result.text() = {}", result.text());
        LOGGER.info("result.errorMsg() = {}", result.errorMsg());
        LOGGER.info("result.tLoadMs() = {}", result.tLoadMs());
        LOGGER.info("result.tMelMs() = {}", result.tMelMs());
        LOGGER.info("result.tEncodeMs() = {}", result.tEncodeMs());
        LOGGER.info("result.tDecodeMs() = {}", result.tDecodeMs());
        LOGGER.info("result.tTotalMs() = {}", result.tTotalMs());
    }

    @BeforeAll
    public static void prepare() {
        LOGGER.info("GGML_CUDA = {}", GGML_CUDA);
        LOGGER.info("GGML_VULKAN = {}", GGML_VULKAN);
        try (FileInputStream wavInEnUs = new FileInputStream("samples/jfk.wav")) {
            wavInEnUs.skip(44);
            byte[] bytesEnUs = new byte[wavInEnUs.available()];
            wavInEnUs.read(bytesEnUs);
            wavInEnUs.close();
            sampleEnUs = toFloatArray(bytesEnUs);
        } catch (IOException e) {
            LOGGER.error("Failed to load sampleEnUs", e);
        }
        try (FileInputStream wavInZhCn = new FileInputStream("samples/IC0936W0337.wav")) {
            wavInZhCn.skip(44);
            byte[] bytesZhCn = new byte[wavInZhCn.available()];
            wavInZhCn.read(bytesZhCn);
            sampleZhCn = toFloatArray(bytesZhCn);
        } catch (IOException e) {
            LOGGER.error("Failed to load sampleZhCn", e);
        }
    }

    @Test
    @EnabledIf("isCpuOnly")
    public void testBuiltinLib() throws IOException, URISyntaxException {
        NativeManager.loadLibrary(LOGGER);
        Qwen3ASR asr = new Qwen3ASR("qwen3-asr-0.6b-f16.gguf");
        LOGGER.info("isLoaded = {}", asr.isLoaded());
        asr.setProgressCallback(TestQwen3ASR::testCallback);
        LOGGER.info("transcribing {} samples in English from samples/jfk.wav", sampleEnUs.length);
        TranscribeResult resultEnUs = asr.transcribe(sampleEnUs, new TranscribeParams());
        printTranscribeResult(resultEnUs);
    }

    @Test
    @EnabledIf("isCpuOnly")
    public void testSampleZhCn() throws IOException, URISyntaxException {
        NativeManager.loadLibrary(LOGGER);
        Qwen3ASR asr = new Qwen3ASR("qwen3-asr-0.6b-f16.gguf");
        asr.setProgressCallback(TestQwen3ASR::testCallback);
        LOGGER.info("isLoaded = {}", asr.isLoaded());
        LOGGER.info("transcribing {} samples in Chinese from samples/IC0936W0337.wav", sampleZhCn.length);
        TranscribeResult resultZhCn = asr.transcribe(sampleZhCn, new TranscribeParams());
        printTranscribeResult(resultZhCn);
    }

    @Test
    @EnabledIf("isVulkanEnabled")
    public void testVulkan() throws IOException, URISyntaxException {
        NativeManager.loadLibrary(Path.of(NativeManager.NATIVE_LIB_DIR + "-vulkan"), LOGGER);
        Qwen3ASR asr = new Qwen3ASR("qwen3-asr-0.6b-f16.gguf");
        LOGGER.info("isLoaded = {}", asr.isLoaded());
        asr.setProgressCallback(TestQwen3ASR::testCallback);
        LOGGER.info("transcribing {} samples in English from samples/jfk.wav", sampleEnUs.length);
        TranscribeResult resultEnUs = asr.transcribe(sampleEnUs, new TranscribeParams());
        printTranscribeResult(resultEnUs);
    }

}
