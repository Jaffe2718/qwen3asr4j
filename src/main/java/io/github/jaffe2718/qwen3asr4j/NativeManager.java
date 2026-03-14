package io.github.jaffe2718.qwen3asr4j;

import java.io.File;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.file.*;
import java.util.HashMap;
import java.util.Objects;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.slf4j.Logger;

public abstract class NativeManager {

    public static final String NATIVE_LIB_DIR;
    public static final String NATIVE_EXTENSION;
    public static final String[] LOAD_ORDER = new String[] {
            "cudart64_12", "cublasLt64_12", "cublas64_12", "openblas", "ggml-base", "ggml-cpu", "ggml-cuda", "ggml-blas", "ggml-metal", "ggml-vulkan", "ggml", "qwen3asr4j"
    };

    static {
        String osName = System.getProperty("os.name").toLowerCase();
        String arch = System.getProperty("os.arch").toLowerCase();
        String osShort;
        String osArch;
        if (osName.contains("win")) {
            osShort = "win";
            NATIVE_EXTENSION = ".dll";
        } else if (osName.contains("mac")) {
            osShort = "mac";
            NATIVE_EXTENSION = ".dylib";
        } else if (osName.contains("linux")) {
            osShort = "linux";
            NATIVE_EXTENSION = ".so";   // not *.so*
        } else {
            throw new IllegalStateException("Unsupported OS: " + osName);
        }
        if (arch.contains("amd64") || arch.contains("x86_64")) {
            osArch = "x64";
        } else if (arch.contains("aarch64")) {
            osArch = "arm64";
        } else {
            throw new IllegalStateException("Unsupported Arch: " + arch);
        }
        NATIVE_LIB_DIR = osShort + "-" + osArch;
    }

    /**
     * Load the native libraries from the resources.
     *
     * @param logger the logger to use for logging, or null if no logging is desired
     * @throws IOException     if something goes wrong
     * @throws URISyntaxException if something goes wrong
     */
    @SuppressWarnings("DataFlowIssue")
    public static void loadLibrary(@Nullable Logger logger) throws IOException, URISyntaxException {
        // 1. extract all files in NATIVE_LIB_DIR to temporary directory
        Path tempDir = Files.createTempDirectory("qwen3asr4j");
        extractResource(NativeManager.class.getResource("/" + NATIVE_LIB_DIR).toURI(), tempDir);
        // 2. load [lib]qwen3asr4j.[so|dll|dylib]
        loadLibrary(tempDir, logger);
    }

    /**
     * Load the native libraries from the specified directory.
     *
     * @param libDir the directory containing the native libraries
     * @param logger the logger to use for logging, or null if no logging is desired
     */
    public static void loadLibrary(Path libDir, @Nullable Logger logger) {
        for (String libName : LOAD_ORDER) {
            for (File file : Objects.requireNonNull(libDir.toFile().listFiles())) {
                if (file.isFile() && file.getName().endsWith(libName + NATIVE_EXTENSION)) {
                    try {
                        System.load(file.getAbsolutePath());
                        if (logger != null) {
                            logger.info("Loaded library {}", file);
                        }
                    } catch (UnsatisfiedLinkError ue) {
                        if (logger != null) {
                            logger.error("Failed to load native library: {}", libName, ue);
                        }
                    }
                }
            }
        }

    }

    /**
     * Helper method that extracts internal resources to a directory.
     *
     * @param uri     internal resource
     * @param destDir destination directory
     * @throws IOException if something goes wrong
     */
    @SuppressWarnings("resource")
    private static void extractResource(@NotNull URI uri, Path destDir) throws IOException {
        Path internalPath;
        // If we're not inside a JAR, there's nothing to do
        if ("jar".equals(uri.getScheme())) {
            // Extract the path to the jar file and the internal path inside the jar
            String[] parts = uri.toString().split("!");
            URI jarUri = URI.create(parts[0]);

            FileSystem fs;

            try {
                fs = FileSystems.newFileSystem(jarUri, new HashMap<>());
            } catch (FileSystemAlreadyExistsException e) {
                fs = FileSystems.getFileSystem(jarUri);
            }

            // Root of fs
            internalPath = fs.getPath(parts[1]);
        } else {
            internalPath = Paths.get(uri);
        }

        // Walk through the tree and create all necessary directories
        Files.walk(internalPath).forEach(path -> {
            try {
                Path dest = destDir.resolve(internalPath.relativize(path).toString());

                if (Files.isDirectory(path)) {
                    Files.createDirectories(dest);
                } else {
                    Files.copy(path, dest, StandardCopyOption.REPLACE_EXISTING);
                }
            } catch (IOException e) {
                throw new UncheckedIOException(e);
            }
        });
    }
}