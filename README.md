# Qwen3ASR4J

Qwen3ASR4J is a Java wrapper for Qwen3-ASR models, providing high-performance speech recognition and forced alignment
capabilities in Java applications. The C++ algorithm is based
on [qwen3-asr.cpp](https://github.com/predict-woo/qwen3-asr.cpp) with modifications to support Java integration.

## Features

- **High Performance**: Leverages GGML for efficient inference on CPU, CUDA, and Vulkan
- **Multi-platform Support**: Windows, Linux, and macOS
- **Dual Functionality**:
    - Speech recognition (ASR) - convert audio to text
    - Forced alignment - align text with audio timestamps
- **Model Support**: Qwen3-ASR-0.6B, Qwen3-ASR-1.7B, and Qwen3-ForcedAligner-0.6B
- **Java-Friendly API**: Simple and intuitive interface for Java developers

## Supports

| Platform    | Supported                         |
|-------------|-----------------------------------|
| linux-arm64 | CPU / <u>CUDA</u> / <u>Vulkan</u> |
| linux-x64   | CPU / <u>CUDA</u> / <u>Vulkan</u> |
| mac-arm64   | CPU                               |
| mac-x64     | CPU                               |
| win-x64     | CPU / <u>CUDA</u> / <u>Vulkan</u> |

> **Note**: CUDA and Vulkan support require additional native libraries to be installed.

| Model Name               | Size         | Type           | Description                            |
|--------------------------|--------------|----------------|----------------------------------------|
| Qwen3-ASR-0.6B           | ~1.8GB (f16) | ASR            | Lightweight speech recognition model   |
| Qwen3-ASR-1.7B           | ~4.7GB (f16) | ASR            | More accurate speech recognition model |
| Qwen3-ForcedAligner-0.6B | ~1.8GB (f16) | Forced Aligner | Aligns text with audio timestamps      |

## Installation

### Gradle

Add the following to your `build.gradle`:

```gradle
dependencies {
    implementation 'com.example:qwen3asr4j:<version>'
}
```

## Example Usage

```java
public class Example {
    private static final Logger LOGGER = LoggerFactory.getLogger(Example.class);
    private static float[] SAMPLES;
    
    public static void main(String[] args) throws Exception {
        // git clone https://huggingface.co/Qwen/Qwen3-ASR-1.7B/
        // git clone https://huggingface.co/Qwen/Qwen3-ForcedAligner-0.6B/
        // python scripts/convert_hf_to_gguf.py --input Qwen3-ASR-1.7B --output qwen3-asr-1.7b-f16.gguf --type f16
        // python scripts/convert_hf_to_gguf.py --input Qwen3-ForcedAligner-0.6B --output qwen3-forcedaligner-0.6b-f16.gguf --type f16
        
        Qwen3ASR asr = new Qwen3ASR("qwen3-asr-1.7b-f16.gguf", LOGGER);
        TranscribeResult result = asr.transcribe(SAMPLES, new TranscribeParams());
        LOGGER.info("Transcribed text: {}", result.text());
        LOGGER.info("Language: {}", result.language());
        asr.close();
        
        ForcedAligner aligner = new ForcedAligner("qwen3-forcedaligner-0.6b-f16.gguf", LOGGER);
        AlignmentResult alignResult = aligner.align(SAMPLES, result.text());
        for (AlignedWord word : alignResult.words()) {
            LOGGER.info("([{}], {} --> {})", word.word(), word.start(), word.end());
        }
        aligner.close();
    }
    
    private static float @NotNull [] toFloatArray(byte @NotNull [] data) {
        float[] result = new float[data.length / 2];
        ShortBuffer shortBuffer = ByteBuffer.wrap(data).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer();
        for (int i = 0; i < result.length; i++) {
            result[i] = Float.max(-1f, Float.min(((float) shortBuffer.get()) / (float) Short.MAX_VALUE, 1f));
        }
        return result;
    }
    
    static {
        try (FileInputStream wavInEnUs = new FileInputStream("samples/jfk.wav")) {
            wavInEnUs.skip(44);
            byte[] bytesEnUs = new byte[wavInEnUs.available()];
            wavInEnUs.read(bytesEnUs);
            wavInEnUs.close();
            SAMPLES = toFloatArray(bytesEnUs);
        } catch (IOException e) {
            LOGGER.error("Failed to load sampleEnUs", e);
            SAMPLES = new float[0];
        }
    }
}
```

## Building the Library

### Prerequisites

- CMake 3.21+
- MSVC 2022+ (Windows)
- GCC 11+ (Linux/MacOS)
- Java Development Kit (JDK) 21+
- Gradle 8.14.3

### Build Steps

1. **Setup Environment**
   - Install CMake, MSVC 2022+ (Windows), GCC 11+ (Linux/MacOS), JDK 21+, and Gradle 8.14.3
   - Set up environment variables for JDK and Gradle
   - Set environment variables:
       - `GGML_CUDA=[ON/OFF]` (default: OFF) - Enable CUDA support
       - `GGML_VULKAN=[ON/OFF]` (default: OFF) - Enable Vulkan support

2. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/qwen3asr4j.git
   cd qwen3asr4j
   ```

3. **Build native libraries**
   ```bash
   ./gradlew compileCpp
   ```

## Model Conversion

To use custom Qwen3-ASR models, you can convert them from Hugging Face format to GGUF format using the provided script:

```bash
python scripts/convert_hf_to_gguf.py --input path/to/hf/model --output model.gguf --type f16
```

## Performance Tips

- **Model Selection**: Use Qwen3-ASR-0.6B for faster inference, Qwen3-ASR-1.7B for higher accuracy
- **Hardware Acceleration**: Enable CUDA or Vulkan for significantly faster inference
- **Batch Processing**: Process multiple audio files in parallel for better throughput
- **Audio Preprocessing**: Ensure audio is resampled to 16kHz for optimal performance

## Troubleshooting

- **Model Loading Failures**: Ensure the model file path is correct and the model is in GGUF format
- **Inference Errors**: Check audio format (16kHz, 16-bit PCM recommended)
- **Performance Issues**: Try using a smaller model or enabling hardware acceleration

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgements

- [QwenLM](https://github.com/QwenLM) for the original Qwen3-ASR models
- [qwen3-asr.cpp](https://github.com/predict-woo/qwen3-asr.cpp) for the C++ implementation
- [GGML](https://github.com/ggerganov/ggml) for efficient machine learning inference

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Contact

For questions or issues, please open an issue on the GitHub repository.